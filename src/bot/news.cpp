#include "bot/news.hpp"
#include "bot/strings.hpp"

namespace bot {

namespace {

constexpr std::size_t MAX_ARTICLES_PER_POLL = 3;
constexpr std::size_t MAX_ARTICLES_PARSE = 10;

std::string strip_cdata(const std::string& s) {
    constexpr std::string_view prefix = "<![CDATA[";
    constexpr std::string_view suffix = "]]>";
    if (s.size() >= prefix.size() + suffix.size()
        && s.compare(0, prefix.size(), prefix) == 0
        && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return s.substr(prefix.size(), s.size() - prefix.size() - suffix.size());
    }
    return s;
}

std::string extract_tag(const std::string& xml, std::string_view tag, std::size_t start, std::size_t end) {
    std::string open = std::string("<") + std::string(tag) + ">";
    std::string close = std::string("</") + std::string(tag) + ">";

    auto tag_start = xml.find(open, start);
    if (tag_start == std::string::npos || tag_start >= end) return {};

    tag_start += open.size();
    auto tag_end = xml.find(close, tag_start);
    if (tag_end == std::string::npos || tag_end > end) return {};

    return strip_cdata(xml.substr(tag_start, tag_end - tag_start));
}

std::string extract_source_tag(const std::string& xml, std::size_t start, std::size_t end) {
    auto src_start = xml.find("<source", start);
    if (src_start == std::string::npos || src_start >= end) return {};

    auto gt = xml.find(">", src_start);
    if (gt == std::string::npos || gt >= end) return {};

    gt += 1;
    auto src_end = xml.find("</source>", gt);
    if (src_end == std::string::npos || src_end > end) return {};

    return xml.substr(gt, src_end - gt);
}

std::string decode_entities(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '&') {
            if (s.compare(i, 4, "&lt;") == 0) { out += '<'; i += 3; }
            else if (s.compare(i, 4, "&gt;") == 0) { out += '>'; i += 3; }
            else if (s.compare(i, 5, "&amp;") == 0) { out += '&'; i += 4; }
            else if (s.compare(i, 6, "&apos;") == 0) { out += '\''; i += 5; }
            else if (s.compare(i, 6, "&quot;") == 0) { out += '"'; i += 5; }
            else if (s.compare(i, 6, "&nbsp;") == 0) { out += ' '; i += 5; }
            else out += s[i];
        } else {
            out += s[i];
        }
    }

    return out;
}

std::string strip_html(const std::string& html) {
    std::string result;
    result.reserve(html.size());
    bool in_tag = false;

    for (char c : html) {
        if (c == '<') { in_tag = true; continue; }
        if (c == '>') { in_tag = false; continue; }
        if (!in_tag) result += c;
    }

    return result;
}

std::string strip_source_suffix(const std::string& title, const std::string& source) {
    if (source.empty()) return title;

    std::string suffix = " - " + source;
    if (title.size() > suffix.size() && title.compare(title.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return title.substr(0, title.size() - suffix.size());
    }

    return title;
}

const NewsCategory* find_category(const std::string& name) {
    for (const auto& cat : NewsFeed::get_categories()) {
        if (cat.name == name) return &cat;
    }
    return nullptr;
}

void fetch_and_post_category(dpp::cluster& bot, uint64_t channel_id, const std::string& cat_name,
                             std::size_t max_articles, bool mark_all_seen) {
    const auto* category = find_category(cat_name);
    if (!category) return;

    auto feeds = category->feeds;

    auto all_articles = std::make_shared<std::vector<NewsArticle>>();
    auto articles_mutex = std::make_shared<std::mutex>();
    auto remaining = std::make_shared<std::atomic<std::size_t>>(feeds.size());

    for (const auto& url : feeds) {
        bot.request(url, dpp::m_get,
            [&bot, channel_id, cat_name, max_articles, mark_all_seen,
             all_articles, articles_mutex, remaining]
            (const dpp::http_request_completion_t& response) {
                if (response.status == 200) {
                    bool body_is_atom = response.body.find("<feed") != std::string::npos;
                    auto parsed = body_is_atom ? NewsFeed::parse_atom(response.body)
                                               : NewsFeed::parse_google_rss(response.body);
                    std::lock_guard lock(*articles_mutex);
                    for (auto& a : parsed) all_articles->push_back(std::move(a));
                }

                if (remaining->fetch_sub(1) == 1) {
                    std::lock_guard lock(*articles_mutex);
                    auto* cat = find_category(cat_name);
                    if (!cat) return;

                    auto& feed = NewsFeed::instance();
                    std::size_t count = 0;
                    for (const auto& article : *all_articles) {
                        if (count >= max_articles) break;

                        if (mark_all_seen || !feed.is_seen(article.guid)) {
                            feed.mark_seen(article.guid);
                            auto embed = NewsFeed::build_article_embed(article, *cat);
                            bot.message_create(dpp::message(dpp::snowflake(channel_id), embed));
                            ++count;
                        }
                    }
                }
            });
    }
}

} // anonymous namespace

const std::vector<NewsCategory>& NewsFeed::get_categories() {
    namespace s = strings::cmd::news;

    static const std::vector<NewsCategory> categories = {
        {"palestine", s::cat_palestine, {
            "https://news.google.com/rss/search?q=palestine&hl=fr&gl=FR&when=1d",
            "https://news.google.com/rss/search?q=gaza+guerre&hl=fr&gl=FR&when=1d",
            "https://news.google.com/rss/search?q=israel+palestine&hl=fr&gl=FR&when=1d",
        }, 0x2ECC71},
        {"usa", s::cat_usa, {
            "https://news.google.com/rss/search?q=united+states+news&hl=en&gl=US&when=1d",
            "https://news.google.com/rss/search?q=USA+politics&hl=en&gl=US&when=1d",
        }, 0x3498DB},
        {"trump", s::cat_trump, {
            "https://news.google.com/rss/search?q=donald+trump&hl=fr&gl=FR&when=1d",
            "https://news.google.com/rss/search?q=trump+president&hl=en&gl=US&when=1d",
        }, 0xE67E22},
        {"musk", s::cat_musk, {
            "https://news.google.com/rss/search?q=elon+musk&hl=fr&gl=FR&when=1d",
            "https://news.google.com/rss/search?q=elon+musk+tesla+spacex&hl=en&gl=US&when=1d",
        }, 0x9B59B6},
        {"iran", s::cat_iran, {
            "https://news.google.com/rss/search?q=iran&hl=fr&gl=FR&when=1d",
            "https://news.google.com/rss/search?q=iran+nuclear+sanctions&hl=en&gl=US&when=1d",
        }, 0xE74C3C},
        {"epstein", s::cat_epstein, {
            "https://news.google.com/rss/search?q=jeffrey+epstein&hl=fr&gl=FR&when=1d",
            "https://news.google.com/rss/search?q=epstein+files&hl=en&gl=US&when=1d",
        }, 0x2C3E50},
        {"monde", s::cat_monde, {
            "https://news.google.com/rss/topics/CAAqJggKIiBDQkFTRWdvSUwyMHZNRGx1YlY4U0FtWnlHZ0pHVWlnQVAB?hl=fr&gl=FR&when=1d",
        }, 0xF1C40F},
        {"freegames", s::cat_freegames, {
            "https://www.reddit.com/r/FreeGameFindings/.rss",
        }, 0x1ABC9C},
        {"cybersec", s::cat_cybersec, {
            "https://www.darkreading.com/rss.xml",
            "https://www.reddit.com/r/darknet/.rss",
        }, 0x8E44AD},
    };

    return categories;
}

std::vector<NewsArticle> NewsFeed::parse_google_rss(const std::string& body) {
    std::vector<NewsArticle> articles;

    std::size_t pos = 0;
    while (articles.size() < MAX_ARTICLES_PARSE) {
        auto item_start = body.find("<item>", pos);
        if (item_start == std::string::npos) break;

        auto item_end = body.find("</item>", item_start);
        if (item_end == std::string::npos) break;

        item_end += 7;

        NewsArticle article;
        article.title = decode_entities(extract_tag(body, "title", item_start, item_end));
        article.link = extract_tag(body, "link", item_start, item_end);
        article.guid = extract_tag(body, "guid", item_start, item_end);
        article.pub_date = extract_tag(body, "pubDate", item_start, item_end);
        article.source = decode_entities(extract_source_tag(body, item_start, item_end));
        if (article.source.empty())
            article.source = decode_entities(extract_tag(body, "dc:creator", item_start, item_end));

        auto raw_desc = decode_entities(extract_tag(body, "description", item_start, item_end));
        article.description = strip_html(raw_desc);

        if (article.guid.empty()) article.guid = article.link;

        article.title = strip_source_suffix(article.title, article.source);

        if (!article.title.empty()) {
            articles.push_back(std::move(article));
        }

        pos = item_end;
    }

    return articles;
}

dpp::embed NewsFeed::build_article_embed(const NewsArticle& article, const NewsCategory& category) {
    namespace s = strings::cmd::news;

    dpp::embed embed;
    embed.set_title(article.title.substr(0, 256));
    embed.set_url(article.link);
    embed.set_color(category.color);

    if (!article.description.empty()) {
        embed.set_description(article.description.substr(0, 300));
    }

    std::string footer_text = article.source.empty() ? category.display_name : article.source;
    embed.set_footer(dpp::embed_footer().set_text(footer_text));

    if (!article.pub_date.empty()) {
        embed.set_timestamp(time(nullptr));
    }

    return embed;
}

std::vector<NewsArticle> NewsFeed::parse_atom(const std::string& body) {
    std::vector<NewsArticle> articles;

    std::size_t pos = 0;
    while (articles.size() < MAX_ARTICLES_PARSE) {
        auto entry_start = body.find("<entry>", pos);
        if (entry_start == std::string::npos) break;

        auto entry_end = body.find("</entry>", entry_start);
        if (entry_end == std::string::npos) break;

        entry_end += 8;

        NewsArticle article;
        article.title = decode_entities(extract_tag(body, "title", entry_start, entry_end));
        article.guid = extract_tag(body, "id", entry_start, entry_end);
        article.pub_date = extract_tag(body, "updated", entry_start, entry_end);

        auto name_tag = extract_tag(body, "name", entry_start, entry_end);
        if (!name_tag.empty()) article.source = decode_entities(name_tag);

        auto link_start = body.find("<link", entry_start);
        if (link_start != std::string::npos && link_start < entry_end) {
            auto href_pos = body.find("href=\"", link_start);
            if (href_pos != std::string::npos && href_pos < entry_end) {
                href_pos += 6;
                auto href_end = body.find("\"", href_pos);
                if (href_end != std::string::npos && href_end < entry_end) {
                    article.link = decode_entities(body.substr(href_pos, href_end - href_pos));
                }
            }
        }

        if (article.guid.empty()) article.guid = article.link;

        if (!article.title.empty()) {
            articles.push_back(std::move(article));
        }

        pos = entry_end;
    }

    return articles;
}

void NewsFeed::start_polling(dpp::cluster& bot) {
    bot.start_timer([&bot](dpp::timer) {
        auto subs = NewsFeed::instance().get_all_subscriptions();

        for (const auto& [channel_id, categories] : subs) {
            for (const auto& cat_name : categories) {
                fetch_and_post_category(bot, channel_id, cat_name, MAX_ARTICLES_PER_POLL, false);
            }
        }
    }, 300);
}

} // namespace bot
