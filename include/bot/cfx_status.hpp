#pragma once

#include <dpp/dpp.h>

#include <cstddef>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace bot {

struct CfxIncident {
    std::string title;
    std::string status;
    std::string description;
    std::string pub_date;
    std::string link;
};

class CfxStatus {
public:
    static CfxStatus& instance() {
        static CfxStatus mgr;
        return mgr;
    }

    void add_channel(uint64_t channel_id, dpp::snowflake message_id) {
        std::lock_guard lock(mutex_);
        channels_[channel_id] = message_id;
    }

    void remove_channel(uint64_t channel_id) {
        std::lock_guard lock(mutex_);
        channels_.erase(channel_id);
    }

    std::unordered_map<uint64_t, dpp::snowflake> get_channels() {
        std::lock_guard lock(mutex_);
        return channels_;
    }

    void set_last_hash(std::size_t hash) {
        std::lock_guard lock(mutex_);
        last_hash_ = hash;
    }

    std::size_t get_last_hash() {
        std::lock_guard lock(mutex_);
        return last_hash_;
    }

    void start_polling(dpp::cluster& bot);

    static std::vector<CfxIncident> parse_rss(const std::string& body);
    static dpp::embed build_embed(const std::vector<CfxIncident>& incidents);

private:
    CfxStatus() = default;
    std::mutex mutex_;
    std::unordered_map<uint64_t, dpp::snowflake> channels_;
    std::size_t last_hash_{0};
};

} // namespace bot
