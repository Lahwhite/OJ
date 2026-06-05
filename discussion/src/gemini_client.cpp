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

// 讨论内容会拼成提示词发送给 Gemini，过长时先截断，避免请求体过大或费用不可控。
constexpr size_t kMaxPromptContentLength = 12000;

std::string getenvString(const char* key) {
#if defined(_WIN32)
    // Windows 下用 _dupenv_s，避免 getenv 返回的指针生命周期和安全告警问题。
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, key) != 0 || !value) {
        return {};
    }
    std::string result(value);
    std::free(value);
    return result;
#else
    // 非 Windows 目前只用于编译兜底，仍保留同名环境变量读取逻辑。
    const char* value = std::getenv(key);
    return value ? std::string(value) : std::string();
#endif
}

std::string firstNonEmptyEnv(const char* first, const char* second) {
    // 支持模块专用变量优先，未配置时再兼容通用 GEMINI_API_KEY。
    std::string value = getenvString(first);
    if (!value.empty()) {
        return value;
    }
    return getenvString(second);
}

std::string truncateText(const std::string& text, size_t max_len) {
    // 截断发生在 UTF-8 字节层面，末尾提示让模型知道上下文并不完整。
    if (text.size() <= max_len) {
        return text;
    }
    return text.substr(0, max_len) + "\n...(content truncated)";
}

std::string buildGeminiRequestBody(const std::string& prompt) {
    // Gemini REST API 的 generateContent 需要 contents -> parts -> text 结构。
    json part;
    part["text"] = prompt;

    json content;
    content["role"] = "user";
    content["parts"] = json::array({part});

    json body;
    body["contents"] = json::array({content});
    // 讨论摘要只需要短文本，关闭 thinking 可以降低延迟并减少无关输出。
    body["generationConfig"] = {
        {"maxOutputTokens", 768},
        {"thinkingConfig", {{"thinkingBudget", 0}}}
    };
    return body.dump();
}

#if defined(_WIN32)

std::wstring widenUtf8(const std::string& text) {
    // WinINet 的宽字符接口需要 UTF-16 路径和主机名，业务字符串统一从 UTF-8 转换。
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
    // 预留给需要读取宽字符响应的场景，目前主要用于保持编码转换能力完整。
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

// WinINet 句柄是裸资源，封装后异常路径也能自动 Close。
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
    // WinINet 只给错误码，这里拼上操作阶段方便日志定位失败位置。
    return prefix + ", WinINet error=" + std::to_string(GetLastError());
}

void setInternetTimeout(HINTERNET handle, DWORD timeout_ms) {
    // 连接、发送和接收使用同一超时，避免外部 API 卡住工作线程。
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
    // 当前 runtime_package 面向 Windows，使用系统 WinINet 免去额外 curl 依赖。
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

    // host 默认是 generativelanguage.googleapis.com，也允许通过环境变量切换代理域名。
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

    // Gemini generateContent 只需要 HTTPS POST，禁止缓存可避免调试时拿到旧响应。
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

    // API key 放在请求头中，不拼进 URL，日志里也不会无意打印完整密钥。
    const std::wstring headers = L"Content-Type: application/json; charset=utf-8\r\n"
                                 L"x-goog-api-key: " + widenUtf8(api_key) + L"\r\n";
    if (!HttpSendRequestW(request,
                          headers.c_str(),
                          static_cast<DWORD>(headers.size()),
                          const_cast<char*>(body.data()),
                          static_cast<DWORD>(body.size()))) {
        throw std::runtime_error(lastWinInetError("failed to send Gemini API request"));
    }

    // 先读取 HTTP 状态，再决定是否把响应体解析成错误信息。
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
        // 分块读取完整响应，摘要虽然短，但错误体也可能超过单个缓冲区。
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
            // Gemini 错误响应通常包含 error.message，提取后返回给前端更可诊断。
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
    // 允许部署时把 Gemini 请求转发到兼容网关，默认仍使用官方主机。
    const std::string value = getenvString("OJ_GEMINI_HOST");
    return value.empty() ? "generativelanguage.googleapis.com" : value;
}

std::string geminiApiVersion() {
    // API 版本作为环境变量暴露，便于后续从 v1beta 切换到稳定版本。
    const std::string value = getenvString("OJ_GEMINI_API_VERSION");
    return value.empty() ? "v1beta" : value;
}

unsigned geminiTimeoutMs() {
    // 默认 30 秒，最小 1 秒，防止误配置成 0 导致请求立即失败。
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
    // 模块专用密钥优先，便于和其他可能使用 Gemini 的模块隔离配置。
    return firstNonEmptyEnv("OJ_GEMINI_API_KEY", "GEMINI_API_KEY");
}

}  // namespace

bool GeminiClient::configured() {
    // handler 用它判断是否提前返回 503，不在这里验证密钥是否真的可用。
    return !geminiApiKey().empty();
}

std::string GeminiClient::activeModel() {
    // 默认模型保持较低延迟，也允许部署环境指定其他 Gemini 模型。
    const std::string model = getenvString("OJ_GEMINI_MODEL");
    return model.empty() ? "gemini-2.5-flash" : model;
}

std::string GeminiClient::buildSummaryPrompt(const DiscussionTopic& topic,
                                             const std::vector<DiscussionComment>& comments) {
    // 提示词要求使用中文输出，并限制为短项目符号，便于直接展示在侧栏。
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
        // 没有评论也允许总结，模型会只基于主题正文给出摘要。
        prompt << "No comments yet.\n";
    } else {
        size_t index = 1;
        for (const auto& comment : comments) {
            // 把回复关系写进提示词，帮助模型理解某条评论是在回应谁。
            prompt << index++ << ". "
                   << (comment.nickname.empty() ? comment.username : comment.nickname);
            if (comment.parent_comment_id > 0) {
                prompt << " replied to comment #" << comment.parent_comment_id;
            }
            prompt << ": " << comment.content << "\n";
        }
    }

    // 最终提示词在这里统一裁剪，调用方不需要关心讨论内容长度。
    return truncateText(prompt.str(), kMaxPromptContentLength);
}

std::string GeminiClient::extractTextFromResponse(const std::string& response_body) {
    // Gemini 可能返回多个 candidate/part，这里把所有文本片段按换行合并。
    const json parsed = json::parse(response_body);
    std::ostringstream text;

    if (!parsed.contains("candidates") || !parsed["candidates"].is_array()) {
        // 响应结构异常通常说明 API 版本或模型返回格式变化。
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
        // 没有可展示文本时按失败处理，前端会展示错误而不是空白摘要。
        throw std::runtime_error("Gemini API response does not contain text");
    }
    return result;
}

DiscussionSummaryResult GeminiClient::summarizeDiscussion(
    const DiscussionTopic& topic,
    const std::vector<DiscussionComment>& comments) const {
    const std::string api_key = geminiApiKey();
    if (api_key.empty()) {
        // 再次检查密钥，防止 handler 之外的调用绕过 configured()。
        throw std::runtime_error("Gemini API key is not configured");
    }

    // 构造请求体和模型名都在发送前完成，便于错误日志定位配置问题。
    const std::string model = activeModel();
    const std::string prompt = buildSummaryPrompt(topic, comments);
    const std::string body = buildGeminiRequestBody(prompt);

#if defined(_WIN32)
    // REST 路径格式：/{version}/models/{model}:generateContent。
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
    // Linux/macOS 暂未实现 HTTP 客户端，避免静默假成功。
    (void)body;
    (void)model;
    throw std::runtime_error("Gemini HTTP client is currently implemented for Windows runtime_package");
#endif
}
