#pragma once

#include <dpp/dpp.h>

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace bot {

struct StickyData {
    std::string content;
    dpp::snowflake last_message_id{};
};

class Stickies {
public:
    static Stickies& instance() {
        static Stickies mgr;
        return mgr;
    }

    void set(uint64_t channel_id, std::string content) {
        std::lock_guard lock(mutex_);
        stickies_[channel_id] = StickyData{std::move(content), {}};
    }

    void remove(uint64_t channel_id) {
        std::lock_guard lock(mutex_);
        stickies_.erase(channel_id);
    }

    std::optional<StickyData> get(uint64_t channel_id) {
        std::lock_guard lock(mutex_);
        auto it = stickies_.find(channel_id);
        if (it == stickies_.end()) return std::nullopt;
        return it->second;
    }

    bool has(uint64_t channel_id) {
        std::lock_guard lock(mutex_);
        return stickies_.contains(channel_id);
    }

    void update_message_id(uint64_t channel_id, dpp::snowflake msg_id) {
        std::lock_guard lock(mutex_);
        auto it = stickies_.find(channel_id);
        if (it != stickies_.end())
            it->second.last_message_id = msg_id;
    }

private:
    Stickies() = default;
    std::mutex mutex_;
    std::unordered_map<uint64_t, StickyData> stickies_;
};

} // namespace bot
