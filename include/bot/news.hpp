#pragma once

#include <dpp/dpp.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bot {

struct NewsArticle {
    std::string title;
    std::string link;
    std::string guid;
    std::string pub_date;
    std::string source;
    std::string description;
};

struct NewsCategory {
    std::string name;
    std::string display_name;
    std::vector<std::string> feeds;
    uint32_t color;
};

class NewsFeed {
public:
    static NewsFeed& instance() {
        static NewsFeed feed;
        return feed;
    }

    static const std::vector<NewsCategory>& get_categories();

    void subscribe(uint64_t channel_id, const std::string& category) {
        std::lock_guard lock(mutex_);
        subscriptions_[channel_id].push_back(category);
    }

    bool unsubscribe(uint64_t channel_id, const std::string& category) {
        std::lock_guard lock(mutex_);
        auto it = subscriptions_.find(channel_id);
        if (it == subscriptions_.end()) return false;

        auto& cats = it->second;
        auto cat_it = std::find(cats.begin(), cats.end(), category);
        if (cat_it == cats.end()) return false;

        cats.erase(cat_it);
        if (cats.empty()) subscriptions_.erase(it);
        return true;
    }

    std::vector<std::string> get_subscriptions(uint64_t channel_id) {
        std::lock_guard lock(mutex_);
        auto it = subscriptions_.find(channel_id);
        if (it == subscriptions_.end()) return {};
        return it->second;
    }

    bool is_seen(const std::string& guid) {
        std::lock_guard lock(mutex_);
        return seen_guids_.count(guid) > 0;
    }

    void mark_seen(const std::string& guid) {
        std::lock_guard lock(mutex_);
        if (seen_guids_.size() > 500) seen_guids_.clear();
        seen_guids_.insert(guid);
    }

    void start_polling(dpp::cluster& bot);

    static std::vector<NewsArticle> parse_google_rss(const std::string& body);
    static std::vector<NewsArticle> parse_atom(const std::string& body);
    static dpp::embed build_article_embed(const NewsArticle& article, const NewsCategory& category);

    std::unordered_map<uint64_t, std::vector<std::string>> get_all_subscriptions() {
        std::lock_guard lock(mutex_);
        return subscriptions_;
    }

private:
    NewsFeed() = default;
    std::mutex mutex_;
    std::unordered_map<uint64_t, std::vector<std::string>> subscriptions_;
    std::unordered_set<std::string> seen_guids_;
};

} // namespace bot
