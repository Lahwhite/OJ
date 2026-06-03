#include "gemini_client.h"

#include "oj/log.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wininet.h>
#endif

using json = nlohmann::json;

namespace {

constexpr size_t kMaxPromptContentLength = 12000;

std::string getenvString(const char* key) {
#if defined(_WIN32)
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, key) != 0 || !value) {
        return {};
    }
    std::string result(value);
    std::free(value);
    return result;
#else
    const char* value = std::getenv(key);
    return value ? std::string(value) : std::string();
#endif
}

std::string firstNonEmptyEnv(const char* first, const char* second) {
    std::string value = getenvString(first);
    if (!value.empty()) {
        return value;
    }
    return getenvString(second);
}

std::string truncateText(const std::string& text, size_t max_len) {
    if (text.size() <= max_len) {
        return text;
    }
    return text.substr(0, max_len) + "\n...(content truncated)";
}

std::string buildGeminiRequestBody(const std::string& prompt) {
    json part;
    part["text"] = prompt;

    json content;
    content["role"] = "user";
    content["parts"] = json::array({part});

    json body;
    body["contents"] = json::array({content});
    body["generationConfig"] = {
        {"maxOutputTokens", 768},
        {"thinkingConfig", {{"thinkingBudget", 0}}}
    };
    return body.dump();
}

#if defined(_WIN32)

std::wstring widenUtf8(const std::string& text) {
    if (text.empty()) {
        return {};
    }
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (length <= 0) {
        throw std::runtime_error("failed to convert request text to UTF-16");
    }
    std::wstring result(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), length);
    return result;
}

std::string narrowUtf8(const std::wstring& text) {
    if (text.empty()) {
        return {};
    }
    const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (length <= 0) {
        throw std::runtime_error("failed to convert response text to UTF-8");
    }
    std::string result(static_cast<size_t>(length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), length, nullptr, nullptr);
    return result;
}

class InternetHandle {
public:
    explicit InternetHandle(HINTERNET handle = nullptr) : handle_(handle) {}

    ~InternetHandle() {
        if (handle_) {
            InternetCloseHandle(handle_);
        }
    }

    InternetHandle(const InternetHandle&) = delete;
    InternetHandle& operator=(const InternetHandle&) = delete;

    HINTERNET get() const {
        return handle_;
    }

    operator HINTERNET() const {
        return handle_;
    }

private:
    HINTERNET handle_;
};

std::string lastWinInetError(const std::string& prefix) {
    return prefix + ", WinINet error=" + std::to_string(GetLastError());
}

void setInternetTimeout(HINTERNET handle, DWORD timeout_ms) {
    InternetSetOption(handle, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
    InternetSetOption(handle, INTERNET_OPTION_SEND_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
    InternetSetOption(handle, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
}

std::string postJsonWithWinInet(const std::wstring& host,
                                INTERNET_PORT port,
                                const std::wstring& path,
                                const std::string& api_key,
                                const std::string& body,
                                unsigned timeout_ms) {
    InternetHandle session(InternetOpenW(L"OJ Discussion Gemini Client/1.0",
                                         INTERNET_OPEN_TYPE_PRECONFIG,
                                         nullptr,
                                         nullptr,
                                         0));
    if (!session.get()) {
        throw std::runtime_error(lastWinInetError("failed to open internet session"));
    }

    const DWORD timeout = timeout_ms;
    setInternetTimeout(session.get(), timeout);

    InternetHandle connection(InternetConnectW(session,
                                              host.c_str(),
                                              port,
                                              nullptr,
                                              nullptr,
                                              INTERNET_SERVICE_HTTP,
                                              0,
                                              0));
    if (!connection.get()) {
        throw std::runtime_error(lastWinInetError("failed to connect to Gemini API host"));
    }

    setInternetTimeout(connection.get(), timeout);

    InternetHandle request(HttpOpenRequestW(connection,
                                           L"POST",
                                           path.c_str(),
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           INTERNET_FLAG_SECURE |
                                               INTERNET_FLAG_RELOAD |
                                               INTERNET_FLAG_NO_CACHE_WRITE,
                                           0));
    if (!request.get()) {
        throw std::runtime_error(lastWinInetError("failed to open Gemini API request"));
    }

    setInternetTimeout(request.get(), timeout);

    const std::wstring headers = L"Content-Type: application/json; charset=utf-8\r\n"
                                 L"x-goog-api-key: " + widenUtf8(api_key) + L"\r\n";
    if (!HttpSendRequestW(request,
                          headers.c_str(),
                          static_cast<DWORD>(headers.size()),
                          const_cast<char*>(body.data()),
                          static_cast<DWORD>(body.size()))) {
        throw std::runtime_error(lastWinInetError("failed to send Gemini API request"));
    }

    DWORD status = 0;
    DWORD status_size = sizeof(status);
    if (!HttpQueryInfoW(request,
                        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                        &status,
                        &status_size,
                        nullptr)) {
        throw std::runtime_error(lastWinInetError("failed to read Gemini API status"));
    }

    std::string response_body;
    for (;;) {
        char buffer[4096];
        DWORD downloaded = 0;
        if (!InternetReadFile(request, buffer, sizeof(buffer), &downloaded)) {
            throw std::runtime_error(lastWinInetError("failed to read Gemini API response"));
        }
        if (downloaded == 0) {
            break;
        }
        response_body.append(buffer, downloaded);
    }

    if (status < 200 || status >= 300) {
        std::string message = "Gemini API returned HTTP " + std::to_string(status);
        try {
            const json parsed = json::parse(response_body);
            if (parsed.contains("error") && parsed["error"].contains("message")) {
                message += ": " + parsed["error"]["message"].get<std::string>();
            }
        } catch (...) {
        }
        throw std::runtime_error(message);
    }

    return response_body;
}

#endif

std::string geminiHost() {
    const std::string value = getenvString("OJ_GEMINI_HOST");
    return value.empty() ? "generativelanguage.googleapis.com" : value;
}

std::string geminiApiVersion() {
    const std::string value = getenvString("OJ_GEMINI_API_VERSION");
    return value.empty() ? "v1beta" : value;
}

unsigned geminiTimeoutMs() {
    const std::string value = getenvString("OJ_GEMINI_TIMEOUT_MS");
    if (value.empty()) {
        return 30000;
    }
    try {
        const unsigned parsed = static_cast<unsigned>(std::stoul(value));
        return std::max(1000u, parsed);
    } catch (...) {
        return 30000;
    }
}

std::string geminiApiKey() {
    return firstNonEmptyEnv("OJ_GEMINI_API_KEY", "GEMINI_API_KEY");
}

}  // namespace

bool GeminiClient::configured() {
    return !geminiApiKey().empty();
}

std::string GeminiClient::activeModel() {
    const std::string model = getenvString("OJ_GEMINI_MODEL");
    return model.empty() ? "gemini-2.5-flash" : model;
}

std::string GeminiClient::buildSummaryPrompt(const DiscussionTopic& topic,
                                             const std::vector<DiscussionComment>& comments) {
    std::ostringstream prompt;
    prompt << "You are an online judge discussion assistant. Summarize the following topic and comments in "
              "Simplified Chinese.\n"
           << "Requirements:\n"
           << "1. Use 3 to 5 concise bullet points.\n"
           << "2. Cover the key problem, proposed ideas, conclusions, and unresolved questions.\n"
           << "3. Do not invent information that does not appear in the discussion.\n"
           << "4. If comments include algorithm advice or debugging clues, mention them clearly.\n"
           << "5. Keep the result suitable for direct display in the discussion page.\n\n";

    prompt << "Problem ID: " << topic.problem_id << "\n";
    prompt << "Topic title: " << topic.title << "\n";
    prompt << "Topic author: " << (topic.nickname.empty() ? topic.username : topic.nickname) << "\n";
    prompt << "Topic content:\n" << topic.content << "\n\n";
    prompt << "Comments:\n";

    if (comments.empty()) {
        prompt << "No comments yet.\n";
    } else {
        size_t index = 1;
        for (const auto& comment : comments) {
            prompt << index++ << ". "
                   << (comment.nickname.empty() ? comment.username : comment.nickname);
            if (comment.parent_comment_id > 0) {
                prompt << " replied to comment #" << comment.parent_comment_id;
            }
            prompt << ": " << comment.content << "\n";
        }
    }

    return truncateText(prompt.str(), kMaxPromptContentLength);
}

std::string GeminiClient::extractTextFromResponse(const std::string& response_body) {
    const json parsed = json::parse(response_body);
    std::ostringstream text;

    if (!parsed.contains("candidates") || !parsed["candidates"].is_array()) {
        throw std::runtime_error("Gemini API response does not contain candidates");
    }

    for (const auto& candidate : parsed["candidates"]) {
        if (!candidate.contains("content") || !candidate["content"].contains("parts")) {
            continue;
        }
        for (const auto& part : candidate["content"]["parts"]) {
            if (part.contains("text") && part["text"].is_string()) {
                if (text.tellp() > 0) {
                    text << "\n";
                }
                text << part["text"].get<std::string>();
            }
        }
    }

    std::string result = text.str();
    if (result.empty()) {
        throw std::runtime_error("Gemini API response does not contain text");
    }
    return result;
}

DiscussionSummaryResult GeminiClient::summarizeDiscussion(
    const DiscussionTopic& topic,
    const std::vector<DiscussionComment>& comments) const {
    const std::string api_key = geminiApiKey();
    if (api_key.empty()) {
        throw std::runtime_error("Gemini API key is not configured");
    }

    const std::string model = activeModel();
    const std::string prompt = buildSummaryPrompt(topic, comments);
    const std::string body = buildGeminiRequestBody(prompt);

#if defined(_WIN32)
    const std::string path = "/" + geminiApiVersion() + "/models/" + model + ":generateContent";
    const std::string response = postJsonWithWinInet(
        widenUtf8(geminiHost()),
        INTERNET_DEFAULT_HTTPS_PORT,
        widenUtf8(path),
        api_key,
        body,
        geminiTimeoutMs());
    return {extractTextFromResponse(response), model};
#else
    (void)body;
    (void)model;
    throw std::runtime_error("Gemini HTTP client is currently implemented for Windows runtime_package");
#endif
}
