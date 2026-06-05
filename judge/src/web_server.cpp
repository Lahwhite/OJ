#include "web_server.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#endif

#ifndef _WIN32
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#endif

namespace {

#ifdef _WIN32
// RAII 初始化 Winsock，供端口探测与 HTTP 服务使用
struct WinsockInit {
    WinsockInit() {
        WSADATA wsa{};
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }
    ~WinsockInit() {
        WSACleanup();
    }
};

bool parseArg(int argc, char* argv[], const std::string& key, std::string& out) {
    const std::string prefix = "--" + key + "=";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.rfind(prefix, 0) == 0) {
            out = a.substr(prefix.size());
            return true;
        }
    }
    return false;
}

std::string toLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

// URL 百分号解码，将 + 转为空格
std::string urlDecodePath(const std::string& raw) {
    std::string out;
    out.reserve(raw.size());
    for (std::size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '%' && i + 2 < raw.size()) {
            const auto hex = raw.substr(i + 1, 2);
            char* end = nullptr;
            const long code = std::strtol(hex.c_str(), &end, 16);
            if (end == hex.c_str() + 2) {
                out.push_back(static_cast<char>(code));
                i += 2;
                continue;
            }
        }
        if (raw[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(raw[i]);
        }
    }
    return out;
}

// 解析 GET 请求路径：去查询串、URL 解码、规范化尾部斜杠
std::string normalizeRequestPath(const std::string& request_target) {
    std::string path = request_target;
    const auto q = path.find('?');
    if (q != std::string::npos) {
        path = path.substr(0, q);
    }
    path = urlDecodePath(path);
    if (path.empty()) {
        path = "/";
    }
    if (path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }
    return path.empty() ? "/" : path;
}

std::string readTextFileOrEmpty(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// 构造并发送简易 HTTP/1.1 响应（含 CORS 头）
void sendHttpResponse(SOCKET client,
                      int status_code,
                      const std::string& status_text,
                      const std::string& content_type,
                      const std::string& body) {
    std::ostringstream header;
    header << "HTTP/1.1 " << status_code << ' ' << status_text << "\r\n"
           << "Content-Type: " << content_type << "\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n"
           << "Cache-Control: no-store\r\n"
           << "Access-Control-Allow-Origin: *\r\n"
           << "\r\n";
    const std::string header_text = header.str();
    send(client, header_text.data(), static_cast<int>(header_text.size()), 0);
    if (!body.empty()) {
        send(client, body.data(), static_cast<int>(body.size()), 0);
    }
}

// 处理单个 HTTP 连接：仅支持 GET，路由到 JSON 或内嵌静态资源
void handleClient(SOCKET client, const std::string& result_file) {
    char buffer[4096];
    const int received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        closesocket(client);
        return;
    }
    buffer[received] = '\0';

    std::istringstream request(buffer);
    std::string method;
    std::string target;
    request >> method >> target;
    if (method != "GET") {
        sendHttpResponse(client, 405, "Method Not Allowed", "text/plain; charset=utf-8", "Method Not Allowed");
        closesocket(client);
        return;
    }

    const std::string path = normalizeRequestPath(target);
    // API 路由：返回最新评测结果 JSON
    if (path == "/latest_result.json") {
        const std::string body = readTextFileOrEmpty(result_file);
        if (body.empty()) {
            sendHttpResponse(client, 404, "Not Found", "text/plain; charset=utf-8", "Result not found");
        } else {
            sendHttpResponse(client, 200, "OK", "application/json; charset=utf-8", body);
        }
        closesocket(client);
        return;
    }

    // 静态资源路由：从内嵌二进制数据返回 HTML/CSS/JS
    const EmbeddedAsset* asset = findEmbeddedAsset(path);
    if (!asset) {
        sendHttpResponse(client, 404, "Not Found", "text/plain; charset=utf-8", "Not Found");
        closesocket(client);
        return;
    }

    const std::string body(reinterpret_cast<const char*>(asset->data), asset->size);
    sendHttpResponse(client, 200, "OK", asset->content_type, body);
    closesocket(client);
}

// 创建 TCP 监听套接字并绑定到指定 host:port
SOCKET createListeningSocket(const std::string& host, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    BOOL reuse = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    if (InetPtonA(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
        listen(sock, 8) != 0) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}
#endif // _WIN32

#ifndef _WIN32
// 尝试 bind 指定端口，成功返回套接字 fd
int posixTryBind(const std::string& host, int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    int reuse = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return -1;
    }
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

bool posixIsListening(const std::string& host, int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return false;
    }
    const bool connected = (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
    ::close(fd);
    return connected;
}
#endif // !_WIN32

} // anonymous namespace

// 在端口范围内逐个尝试 bind，返回第一个可用端口
int findAvailablePort(const std::string& host, int start_port, int end_port) {
    if (start_port > end_port) {
        std::swap(start_port, end_port);
    }
#ifdef _WIN32
    WinsockInit init;
    for (int port = start_port; port <= end_port; ++port) {
        SOCKET sock = createListeningSocket(host, port);
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            return port;
        }
    }
#else
    for (int port = start_port; port <= end_port; ++port) {
        int fd = posixTryBind(host, port);
        if (fd >= 0) {
            ::close(fd);
            return port;
        }
    }
#endif
    return -1;
}

// 检测指定 host:port 是否已有服务在监听
bool isPortListening(const std::string& host, int port) {
#ifdef _WIN32
    WinsockInit init;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return false;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    if (InetPtonA(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        closesocket(sock);
        return false;
    }
    const bool running = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
    closesocket(sock);
    return running;
#else
    return posixIsListening(host, port);
#endif
}

// 以分离进程方式启动 Web 服务，评测主进程可立即退出
bool spawnDetachedWebServer(const std::string& executable_path,
                            const std::string& host,
                            int port,
                            int duration_sec,
                            const std::string& result_file) {
#ifdef _WIN32
    // Windows：复用同一可执行文件，传入 --serve_web 子模式参数
    std::string cmd = "\"" + executable_path + "\""
                      + " --serve_web=" + std::to_string(port)
                      + " --serve_host=" + host
                      + " --serve_duration=" + std::to_string(duration_sec)
                      + " --result_file=\"" + result_file + "\"";

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    std::vector<char> cmd_buffer(cmd.begin(), cmd.end());
    cmd_buffer.push_back('\0');

    const BOOL ok = CreateProcessA(
        nullptr,
        cmd_buffer.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW | DETACHED_PROCESS,
        nullptr,
        nullptr,
        &si,
        &pi);
    if (!ok) {
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;

#else
    // Linux：将结果与内嵌前端资源写入 serve 目录，用 python http.server 托管
    std::filesystem::path result_path(result_file);
    std::filesystem::path serve_dir = result_path.parent_path();
    std::filesystem::path latest_path = serve_dir / "latest_result.json";
    std::error_code ec;
    // 复制为 latest_result.json，供前端固定路径拉取
    std::filesystem::copy_file(result_path, latest_path,
                               std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        std::cerr << "[web] Warning: could not copy to latest_result.json: " << ec.message() << '\n';
    }

    // 将内嵌 HTML/CSS/JS 解压到 serve 目录
    for (const EmbeddedAsset* it = embeddedAssetsBegin(); it != embeddedAssetsEnd(); ++it) {
        std::string rel(it->path);
        if (!rel.empty() && rel[0] == '/') {
            rel = rel.substr(1);
        }
        if (rel.empty()) {
            continue;
        }
        std::filesystem::path dest = serve_dir / rel;
        std::filesystem::create_directories(dest.parent_path(), ec);
        std::ofstream out(dest, std::ios::binary | std::ios::trunc);
        if (out.is_open()) {
            out.write(reinterpret_cast<const char*>(it->data),
                      static_cast<std::streamsize>(it->size));
        }
    }

    // fork 子进程运行 python http.server
    pid_t pid = ::fork();
    if (pid < 0) {
        return false;
    }
    if (pid == 0) {
        ::setsid();  // 脱离终端，成为守护进程

        int dev_null = ::open("/dev/null", O_WRONLY);
        if (dev_null >= 0) {
            ::dup2(dev_null, STDOUT_FILENO);
            ::dup2(dev_null, STDERR_FILENO);
            ::close(dev_null);
        }

        ::chdir(serve_dir.c_str());

        const std::string port_str = std::to_string(port);
        ::execl("/usr/bin/python3", "python3",
                "-m", "http.server",
                port_str.c_str(),
                "--bind", host.c_str(),
                nullptr);

        const std::string sh_cmd = "python3 -m http.server " + port_str + " --bind " + host;
        ::execl("/bin/sh", "sh", "-c", sh_cmd.c_str(), nullptr);
        ::_exit(127);
    }

    ::signal(SIGCHLD, SIG_IGN);
    return true;

#endif
}

// 内嵌 Web 服务主循环：在限定时间内 accept 并处理请求
int runWebServeMode(int argc, char* argv[]) {
#ifdef _WIN32
    std::string port_text;
    std::string host = "127.0.0.1";
    std::string duration_text = "300";
    std::string result_file;

    auto findArg = [&](const std::string& key, std::string& out) -> bool {
        const std::string prefix = "--" + key + "=";
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a.rfind(prefix, 0) == 0) {
                out = a.substr(prefix.size());
                return true;
            }
        }
        return false;
    };

    if (!findArg("serve_web", port_text) || !findArg("result_file", result_file)) {
        std::cerr << "Invalid serve mode arguments.\n";
        return 1;
    }
    findArg("serve_host", host);
    findArg("serve_duration", duration_text);

    const int port = std::stoi(port_text);
    const int duration_sec = std::max(30, std::stoi(duration_text));

    WinsockInit init;
    SOCKET server = createListeningSocket(host, port);
    if (server == INVALID_SOCKET) {
        std::cerr << "Failed to bind web server on " << host << ':' << port << '\n';
        return 1;
    }

    // 服务存活时间到达后自动退出
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(duration_sec);
    while (std::chrono::steady_clock::now() < deadline) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(server, &read_set);

        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        const int ready = select(0, &read_set, nullptr, nullptr, &timeout);
        if (ready <= 0) {
            continue;
        }

        // 接受连接并同步处理（简易单线程 HTTP 服务）
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) {
            continue;
        }
        handleClient(client, result_file);
    }

    closesocket(server);
    return 0;
#else
    (void)argc;
    (void)argv;
    return 1;
#endif
}

// 跨平台打开默认浏览器访问结果页
void openBrowserUrl(const std::string& url) {
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    const std::string cmd = "open \"" + url + "\" &";
    std::system(cmd.c_str());
#else
    const std::string cmd = "xdg-open \"" + url + "\" &";
    std::system(cmd.c_str());
#endif
}

// 构造内嵌服务的结果页 URL，t 参数用于缓存穿透
std::string buildResultPageUrl(const std::string& host, int port, long long timestamp_ms) {
    return "http://" + host + ':' + std::to_string(port) + "/?latest=1&t=" + std::to_string(timestamp_ms);
}

// 为外部静态站点 URL 追加 latest 与缓存穿透参数
std::string buildExternalResultUrl(const std::string& base_url, long long timestamp_ms) {
    std::string url = base_url;
    if (url.empty()) {
        return url;
    }
    // 确保 URL 包含 latest=1，触发前端自动加载逻辑
    if (url.find('?') == std::string::npos) {
        url += "?latest=1";
    } else if (url.find("latest=") == std::string::npos) {
        url += "&latest=1";
    }
    url += "&t=" + std::to_string(timestamp_ms);
    return url;
}
