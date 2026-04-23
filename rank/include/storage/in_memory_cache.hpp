#pragma once

#include "leaderboard/repository.hpp"

#include <chrono>
#include <unordered_map>

namespace oj {

class InMemoryLeaderboardCache : public ILeaderboardCache {
public:
    std::optional<std::vector<LeaderboardRow>> get_global_rows(std::int32_t limit, std::int32_t offset) override;
    void set_global_rows(const std::vector<LeaderboardRow>& rows, int ttl_seconds) override;
    void invalidate_global_rows() override;

    std::optional<std::vector<ContestLeaderboardRow>> get_contest_rows(
        std::int64_t contest_id,
        std::int32_t limit,
        std::int32_t offset) override;
    void set_contest_rows(std::int64_t contest_id, const std::vector<ContestLeaderboardRow>& rows, int ttl_seconds) override;
    void invalidate_contest_rows(std::int64_t contest_id) override;

private:
    template <typename T>
    struct CacheEntry {
        std::vector<T> rows;
        std::chrono::steady_clock::time_point expired_at;
    };

    template <typename T>
    static std::vector<T> paginate(const std::vector<T>& rows, std::int32_t limit, std::int32_t offset);

    std::optional<CacheEntry<LeaderboardRow>> global_cache_;
    std::unordered_map<std::int64_t, CacheEntry<ContestLeaderboardRow>> contest_cache_;
};

}  // namespace oj
