#include "storage/in_memory_cache.hpp"

#include <algorithm>

namespace oj {

std::optional<std::vector<LeaderboardRow>> InMemoryLeaderboardCache::get_global_rows(std::int32_t limit, std::int32_t offset) {
    if (!global_cache_.has_value()) {
        return std::nullopt;
    }
    if (std::chrono::steady_clock::now() >= global_cache_->expired_at) {
        global_cache_.reset();
        return std::nullopt;
    }
    return paginate(global_cache_->rows, limit, offset);
}

void InMemoryLeaderboardCache::set_global_rows(const std::vector<LeaderboardRow>& rows, int ttl_seconds) {
    CacheEntry<LeaderboardRow> entry;
    entry.rows = rows;
    entry.expired_at = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    global_cache_ = std::move(entry);
}

void InMemoryLeaderboardCache::invalidate_global_rows() {
    global_cache_.reset();
}

std::optional<std::vector<ContestLeaderboardRow>> InMemoryLeaderboardCache::get_contest_rows(
    std::int64_t contest_id,
    std::int32_t limit,
    std::int32_t offset) {
    auto it = contest_cache_.find(contest_id);
    if (it == contest_cache_.end()) {
        return std::nullopt;
    }
    if (std::chrono::steady_clock::now() >= it->second.expired_at) {
        contest_cache_.erase(it);
        return std::nullopt;
    }
    return paginate(it->second.rows, limit, offset);
}

void InMemoryLeaderboardCache::set_contest_rows(
    std::int64_t contest_id,
    const std::vector<ContestLeaderboardRow>& rows,
    int ttl_seconds) {
    CacheEntry<ContestLeaderboardRow> entry;
    entry.rows = rows;
    entry.expired_at = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    contest_cache_[contest_id] = std::move(entry);
}

void InMemoryLeaderboardCache::invalidate_contest_rows(std::int64_t contest_id) {
    contest_cache_.erase(contest_id);
}

template <typename T>
std::vector<T> InMemoryLeaderboardCache::paginate(const std::vector<T>& rows, std::int32_t limit, std::int32_t offset) {
    if (offset < 0) {
        offset = 0;
    }
    if (limit <= 0) {
        limit = static_cast<std::int32_t>(rows.size());
    }

    const auto begin_idx = static_cast<std::size_t>(std::min<std::int32_t>(offset, static_cast<std::int32_t>(rows.size())));
    const auto end_idx = static_cast<std::size_t>(
        std::min<std::int32_t>(offset + limit, static_cast<std::int32_t>(rows.size())));
    return std::vector<T>(rows.begin() + static_cast<std::ptrdiff_t>(begin_idx), rows.begin() + static_cast<std::ptrdiff_t>(end_idx));
}

template std::vector<LeaderboardRow> InMemoryLeaderboardCache::paginate(
    const std::vector<LeaderboardRow>&,
    std::int32_t,
    std::int32_t);
template std::vector<ContestLeaderboardRow> InMemoryLeaderboardCache::paginate(
    const std::vector<ContestLeaderboardRow>&,
    std::int32_t,
    std::int32_t);

}  // namespace oj
