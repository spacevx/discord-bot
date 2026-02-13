#include "bot/command.hpp"
#include "bot/sticky.hpp"
#include "bot/strings.hpp"

namespace {

namespace s = bot::strings::cmd::sticky;

class StickyCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "sticky"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("sticky", s::description, bot_id)
            .set_default_permissions(dpp::p_manage_messages)
            .add_option(dpp::command_option(dpp::co_string, "message", s::opt_message, true));
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_manage_messages)) {
            event.reply(dpp::message(s::no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto content = std::get<std::string>(event.get_parameter("message"));
        auto channel_id = event.command.channel_id;
        auto& mgr = bot::Stickies::instance();

        auto old = mgr.get(channel_id);
        if (old && old->last_message_id) {
            event.from->creator->message_delete(old->last_message_id, channel_id);
        }

        mgr.set(channel_id, content);

        event.from->creator->message_create(dpp::message(channel_id, content),
            [channel_id](const dpp::confirmation_callback_t& cb) {
                if (!cb.is_error()) {
                    auto msg = cb.get<dpp::message>();
                    bot::Stickies::instance().update_message_id(channel_id, msg.id);
                }
            });

        event.reply(dpp::message(s::success).set_flags(dpp::m_ephemeral));
    }
};

REGISTER_COMMAND(StickyCommand);

} // anonymous namespace
