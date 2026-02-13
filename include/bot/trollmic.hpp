#pragma once

#include <dpp/dpp.h>

#include <chrono>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace bot {

struct TrollSession {
    dpp::snowflake target_id;
    dpp::snowflake channel_id;
    bool is_muted{false};
    std::chrono::steady_clock::time_point last_voice_packet{};
};

class TrollMic {
public:
    static TrollMic& instance() {
        static TrollMic mgr;
        return mgr;
    }

    void set(uint64_t guild_id, dpp::snowflake target_id, dpp::snowflake channel_id) {
        std::lock_guard lock(mutex_);
        sessions_[guild_id] = TrollSession{target_id, channel_id, false, {}};
    }

    void remove(uint64_t guild_id) {
        std::lock_guard lock(mutex_);
        sessions_.erase(guild_id);
    }

    std::optional<TrollSession> get(uint64_t guild_id) {
        std::lock_guard lock(mutex_);
        auto it = sessions_.find(guild_id);
        if (it == sessions_.end()) return std::nullopt;
        return it->second;
    }

    bool has(uint64_t guild_id) {
        std::lock_guard lock(mutex_);
        return sessions_.contains(guild_id);
    }

    void set_muted(uint64_t guild_id, bool muted) {
        std::lock_guard lock(mutex_);
        auto it = sessions_.find(guild_id);
        if (it != sessions_.end())
            it->second.is_muted = muted;
    }

    void touch_voice(uint64_t guild_id) {
        std::lock_guard lock(mutex_);
        auto it = sessions_.find(guild_id);
        if (it != sessions_.end())
            it->second.last_voice_packet = std::chrono::steady_clock::now();
    }

    template <typename Fn>
    void for_each(Fn&& fn) {
        std::lock_guard lock(mutex_);
        for (auto& [guild_id, session] : sessions_)
            fn(guild_id, session);
    }

private:
    TrollMic() = default;
    std::mutex mutex_;
    std::unordered_map<uint64_t, TrollSession> sessions_;
};

} // namespace bot
