#include "oj/redis_cache.h"

#include "oj/log.h"

#ifdef OJ_WITH_REDIS
#include <hiredis/hiredis.h>
#endif

namespace oj {

RedisCache& RedisCache::instance() {
    static RedisCache c;
    return c;
}

RedisCache::~RedisCache() {
    disconnect();
}

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
    return false;
#else
    disconnect();
    redisContext* c = redisConnect(host.c_str(), static_cast<int>(port));
    if (!c || c->err) {
        const char* err = c ? c->errstr : "alloc failed";
        OJ_LOG_ERROR(std::string("Redis connect failed: ") + err);
        if (c) {
            redisFree(c);
        }
        return false;
    }
    if (!password.empty()) {
        redisReply* reply = static_cast<redisReply*>(
            redisCommand(c, "AUTH %s", password.c_str()));
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            OJ_LOG_ERROR("Redis AUTH failed");
            if (reply) {
                freeReplyObject(reply);
            }
            redisFree(c);
            return false;
        }
        freeReplyObject(reply);
    }
    if (db != 0) {
        redisReply* reply = static_cast<redisReply*>(
            redisCommand(c, "SELECT %d", db));
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            OJ_LOG_ERROR("Redis SELECT failed");
            if (reply) {
                freeReplyObject(reply);
            }
            redisFree(c);
            return false;
        }
        freeReplyObject(reply);
    }
    ctx_ = c;
    connected_ = true;
    OJ_LOG_INFO("Redis: connected to " + host + ":" + std::to_string(port));
    return true;
#endif
}

void RedisCache::disconnect() {
#ifdef OJ_WITH_REDIS
    if (ctx_) {
        redisFree(static_cast<redisContext*>(ctx_));
        ctx_ = nullptr;
    }
#endif
    connected_ = false;
}

bool RedisCache::set(const std::string& key, const std::string& value, int ttl_seconds) {
#ifndef OJ_WITH_REDIS
    (void)key;
    (void)value;
    (void)ttl_seconds;
    return false;
#else
    if (!ctx_) {
        return false;
    }
    redisContext* c = static_cast<redisContext*>(ctx_);
    redisReply* reply = nullptr;
    if (ttl_seconds > 0) {
        reply = static_cast<redisReply*>(redisCommand(
            c, "SET %s %s EX %d", key.c_str(), value.c_str(), ttl_seconds));
    } else {
        reply = static_cast<redisReply*>(
            redisCommand(c, "SET %s %s", key.c_str(), value.c_str()));
    }
    if (!reply) {
        OJ_LOG_ERROR("Redis SET failed (null reply)");
        return false;
    }
    bool ok = (reply->type != REDIS_REPLY_ERROR);
    freeReplyObject(reply);
    return ok;
#endif
}

std::optional<std::string> RedisCache::get(const std::string& key) {
#ifndef OJ_WITH_REDIS
    (void)key;
    return std::nullopt;
#else
    if (!ctx_) {
        return std::nullopt;
    }
    redisContext* c = static_cast<redisContext*>(ctx_);
    redisReply* reply =
        static_cast<redisReply*>(redisCommand(c, "GET %s", key.c_str()));
    if (!reply) {
        return std::nullopt;
    }
    std::optional<std::string> out;
    if (reply->type == REDIS_REPLY_STRING && reply->str) {
        out = std::string(reply->str, reply->len);
    }
    freeReplyObject(reply);
    return out;
#endif
}

bool RedisCache::del(const std::string& key) {
#ifndef OJ_WITH_REDIS
    (void)key;
    return false;
#else
    if (!ctx_) {
        return false;
    }
    redisContext* c = static_cast<redisContext*>(ctx_);
    redisReply* reply =
        static_cast<redisReply*>(redisCommand(c, "DEL %s", key.c_str()));
    if (!reply) {
        return false;
    }
    bool ok = reply->type != REDIS_REPLY_ERROR;
    freeReplyObject(reply);
    return ok;
#endif
}

}  // namespace oj
