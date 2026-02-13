#include "bot/command.hpp"
#include "bot/news.hpp"
#include "bot/strings.hpp"

namespace {

namespace s = bot::strings::cmd::news;

class SetupNewsCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "setupnews"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        dpp::command_option cat_option(dpp::co_string, "categorie", s::setup_opt_category, true);

        for (const auto& cat : bot::NewsFeed::get_categories()) {
            cat_option.add_choice(dpp::command_option_choice(cat.display_name, cat.name));
        }

        return dpp::slashcommand("setupnews", s::setup_description, bot_id)
            .set_default_permissions(dpp::p_manage_messages)
            .add_option(cat_option);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_manage_messages)) {
            event.reply(dpp::message(s::setup_no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto category = std::get<std::string>(event.get_parameter("categorie"));
        auto channel_id = event.command.channel_id;
        auto* bot = event.from->creator;

        const bot::NewsCategory* cat = nullptr;
        for (const auto& c : bot::NewsFeed::get_categories()) {
            if (c.name == category) { cat = &c; break; }
        }

        if (!cat) return;

        bot::NewsFeed::instance().subscribe(channel_id, category);
        event.reply(dpp::message(fmt::format(s::setup_success, cat->display_name)).set_flags(dpp::m_ephemeral));

        std::size_t feed_count = cat->feeds.size();
        auto all_articles = std::make_shared<std::vector<bot::NewsArticle>>();
        auto articles_mutex = std::make_shared<std::mutex>();
        auto remaining = std::make_shared<std::atomic<std::size_t>>(feed_count);

        for (const auto& url : cat->feeds) {
            bot->request(url, dpp::m_get,
                [bot, channel_id, cat, all_articles, articles_mutex, remaining]
                (const dpp::http_request_completion_t& response) {
                    if (response.status == 200) {
                        bool body_is_atom = response.body.find("<feed") != std::string::npos;
                        auto parsed = body_is_atom ? bot::NewsFeed::parse_atom(response.body)
                                                    : bot::NewsFeed::parse_google_rss(response.body);
                        std::lock_guard lock(*articles_mutex);
                        for (auto& a : parsed) all_articles->push_back(std::move(a));
                    }

                    if (remaining->fetch_sub(1) == 1) {
                        std::lock_guard lock(*articles_mutex);
                        auto& feed = bot::NewsFeed::instance();
                        std::size_t count = 0;

                        for (const auto& article : *all_articles) {
                            if (count >= 3) break;
                            feed.mark_seen(article.guid);
                            auto embed = bot::NewsFeed::build_article_embed(article, *cat);
                            bot->message_create(dpp::message(dpp::snowflake(channel_id), embed));
                            ++count;
                        }
                    }
                });
        }
    }
};

REGISTER_COMMAND(SetupNewsCommand);

} // anonymous namespace
