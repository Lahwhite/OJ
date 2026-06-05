#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <cstddef>
#include <string>

// 内嵌到可执行文件中的前端静态资源描述
struct EmbeddedAsset {
    const char* path;
    const char* content_type;
    const unsigned char* data;
    std::size_t size;
};

const EmbeddedAsset* findEmbeddedAsset(const std::string& url_path);
const EmbeddedAsset* embeddedAssetsBegin();
const EmbeddedAsset* embeddedAssetsEnd();

// 在端口范围内查找可绑定的空闲端口
int findAvailablePort(const std::string& host, int start_port, int end_port);
bool isPortListening(const std::string& host, int port);

// 后台启动 Web 服务进程，用于展示评测结果
bool spawnDetachedWebServer(const std::string& executable_path,
                            const std::string& host,
                            int port,
                            int duration_sec,
                            const std::string& result_file);

// 以 --serve_web 参数启动的内嵌 HTTP 服务入口
int runWebServeMode(int argc, char* argv[]);

// 调用系统默认浏览器打开结果页
void openBrowserUrl(const std::string& url);

std::string buildResultPageUrl(const std::string& host,
                               int port,
                               long long timestamp_ms);

std::string buildExternalResultUrl(const std::string& base_url, long long timestamp_ms);

#endif
