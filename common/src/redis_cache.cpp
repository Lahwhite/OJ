// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/redis_cache.h"

#include "oj/log.h"

#ifdef OJ_WITH_REDIS
#include <hiredis/hiredis.h>
#endif

namespace oj {

RedisCache& RedisCache::instance() {
    static RedisCache c;
    // 返回当前阶段的处理结果或默认兜底值
    return c;
}

RedisCache::~RedisCache() {
    disconnect();
}

// 布尔返回值通常用于表示是否执行成功
bool RedisCache::connect(const std::string& host,
                           unsigned port,
                           const std::string& password,
                           int db) {
#ifndef OJ_WITH_REDIS
    (void)host;
    (void)port;
    (void)password;
    (void)db;
    OJ_LOG_WARN("Redis: built without OJ_WITH_REDIS; cache disabled");
    // 返回当前阶段的处理结果或默认兜底值
    return false;
#else
    // 这里每次重新 connect 前先断开旧连接，省得状态不干净
    disconnect();
    redisContext* c = redisConnect(host.c_str(), static_cast<int>(port));
    // 条件判断：根据运行时状态决定后续流程
    if (!c || c->err) {
        const char* err = c ? c->errstr : "alloc failed";
        OJ_LOG_ERROR(std::string("Redis connect failed: ") + err);
        // 条件判断：根据运行时状态决定后续流程
        if (c) {
            redisFree(c);
        }
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    // 条件判断：根据运行时状态决定后续流程
    if (!password.empty()) {
        redisReply* reply = static_cast<redisReply*>(
            redisCommand(c, "AUTH %s", password.c_str()));
        // 条件判断：根据运行时状态决定后续流程
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            OJ_LOG_ERROR("Redis AUTH failed");
            // 条件判断：根据运行时状态决定后续流程
            if (reply) {
                freeReplyObject(reply);
            }
            redisFree(c);
            // 返回当前阶段的处理结果或默认兜底值
            return false;
        }
        freeReplyObject(reply);
    }
    // 条件判断：根据运行时状态决定后续流程
    if (db != 0) {
        redisReply* reply = static_cast<redisReply*>(
            redisCommand(c, "SELECT %d", db));
        // 条件判断：根据运行时状态决定后续流程
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            OJ_LOG_ERROR("Redis SELECT failed");
            // 条件判断：根据运行时状态决定后续流程
            if (reply) {
                freeReplyObject(reply);
            }
            redisFree(c);
            // 返回当前阶段的处理结果或默认兜底值
            return false;
        }
        freeReplyObject(reply);
    }
    ctx_ = c;
    connected_ = true;
    OJ_LOG_INFO("Redis: connected to " + host + ":" + std::to_string(port));
    // 返回当前阶段的处理结果或默认兜底值
    return true;
#endif
}

// 过程型函数：主要通过副作用完成状态更新
void RedisCache::disconnect() {
#ifdef OJ_WITH_REDIS
    // 条件判断：根据运行时状态决定后续流程
    if (ctx_) {
        redisFree(static_cast<redisContext*>(ctx_));
        ctx_ = nullptr;
    }
#endif
    connected_ = false;
}

// 布尔返回值通常用于表示是否执行成功
bool RedisCache::set(const std::string& key, const std::string& value, int ttl_seconds) {
#ifndef OJ_WITH_REDIS
    (void)key;
    (void)value;
    (void)ttl_seconds;
    // 返回当前阶段的处理结果或默认兜底值
    return false;
#else
    // 条件判断：根据运行时状态决定后续流程
    if (!ctx_) {
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    redisContext* c = static_cast<redisContext*>(ctx_);
    redisReply* reply = nullptr;
    // 条件判断：根据运行时状态决定后续流程
    if (ttl_seconds > 0) {
        reply = static_cast<redisReply*>(redisCommand(
            c, "SET %s %s EX %d", key.c_str(), value.c_str(), ttl_seconds));
    } else {
        reply = static_cast<redisReply*>(
            redisCommand(c, "SET %s %s", key.c_str(), value.c_str()));
    }
    // 条件判断：根据运行时状态决定后续流程
    if (!reply) {
        OJ_LOG_ERROR("Redis SET failed (null reply)");
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    // 布尔返回值通常用于表示是否执行成功
    bool ok = (reply->type != REDIS_REPLY_ERROR);
    freeReplyObject(reply);
    // 返回当前阶段的处理结果或默认兜底值
    return ok;
#endif
}

std::optional<std::string> RedisCache::get(const std::string& key) {
#ifndef OJ_WITH_REDIS
    (void)key;
    // 返回当前阶段的处理结果或默认兜底值
    return std::nullopt;
#else
    // 条件判断：根据运行时状态决定后续流程
    if (!ctx_) {
        // 返回当前阶段的处理结果或默认兜底值
        return std::nullopt;
    }
    redisContext* c = static_cast<redisContext*>(ctx_);
    redisReply* reply =
        static_cast<redisReply*>(redisCommand(c, "GET %s", key.c_str()));
    // 条件判断：根据运行时状态决定后续流程
    if (!reply) {
        // 返回当前阶段的处理结果或默认兜底值
        return std::nullopt;
    }
    std::optional<std::string> out;
    // 条件判断：根据运行时状态决定后续流程
    if (reply->type == REDIS_REPLY_STRING && reply->str) {
        out = std::string(reply->str, reply->len);
    }
    freeReplyObject(reply);
    // 返回当前阶段的处理结果或默认兜底值
    return out;
#endif
}

// 布尔返回值通常用于表示是否执行成功
bool RedisCache::del(const std::string& key) {
#ifndef OJ_WITH_REDIS
    (void)key;
    // 返回当前阶段的处理结果或默认兜底值
    return false;
#else
    // 条件判断：根据运行时状态决定后续流程
    if (!ctx_) {
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    redisContext* c = static_cast<redisContext*>(ctx_);
    redisReply* reply =
        static_cast<redisReply*>(redisCommand(c, "DEL %s", key.c_str()));
    // 条件判断：根据运行时状态决定后续流程
    if (!reply) {
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    // 布尔返回值通常用于表示是否执行成功
    bool ok = reply->type != REDIS_REPLY_ERROR;
    freeReplyObject(reply);
    // 返回当前阶段的处理结果或默认兜底值
    return ok;
#endif
}

}  // namespace oj
