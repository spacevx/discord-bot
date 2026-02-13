#include "bot/command.hpp"
#include "bot/news.hpp"
#include "bot/strings.hpp"

namespace {

namespace s = bot::strings::cmd::news;

class RemoveNewsCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "removenews"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        dpp::command_option cat_option(dpp::co_string, "categorie", s::remove_opt_category, true);

        for (const auto& cat : bot::NewsFeed::get_categories()) {
            cat_option.add_choice(dpp::command_option_choice(cat.display_name, cat.name));
        }

        return dpp::slashcommand("removenews", s::remove_description, bot_id)
            .set_default_permissions(dpp::p_manage_messages)
            .add_option(cat_option);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_manage_messages)) {
            event.reply(dpp::message(s::remove_no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto category = std::get<std::string>(event.get_parameter("categorie"));

        const bot::NewsCategory* cat = nullptr;
        for (const auto& c : bot::NewsFeed::get_categories()) {
            if (c.name == category) { cat = &c; break; }
        }

        if (!cat) return;

        bool removed = bot::NewsFeed::instance().unsubscribe(event.command.channel_id, category);

        if (removed) {
            event.reply(dpp::message(fmt::format(s::remove_success, cat->display_name)).set_flags(dpp::m_ephemeral));
        } else {
            event.reply(dpp::message(fmt::format(s::remove_not_found, cat->display_name)).set_flags(dpp::m_ephemeral));
        }
    }
};

REGISTER_COMMAND(RemoveNewsCommand);

} // anonymous namespace
