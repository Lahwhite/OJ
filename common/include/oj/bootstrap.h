#pragma once

#include "oj/config.h"

namespace oj {

// 启动时统一初始化日志、数据库连接池、Redis 这些基础设施
void initInfrastructure(const AppConfig& config);

// 进程退出前做资源回收

}  // namespace oj
