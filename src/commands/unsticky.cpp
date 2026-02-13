#include "bot/command.hpp"
#include "bot/sticky.hpp"
#include "bot/strings.hpp"

namespace {

namespace s = bot::strings::cmd::unsticky;

class UnstickyCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "unsticky"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("unsticky", s::description, bot_id)
            .set_default_permissions(dpp::p_manage_messages);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_manage_messages)) {
            event.reply(dpp::message(s::no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto channel_id = event.command.channel_id;
        auto& mgr = bot::Stickies::instance();
        auto data = mgr.get(channel_id);

        if (!data) {
            event.reply(dpp::message(s::not_found).set_flags(dpp::m_ephemeral));
            return;
        }

        if (data->last_message_id) {
            event.from->creator->message_delete(data->last_message_id, channel_id);
        }

        mgr.remove(channel_id);
        event.reply(dpp::message(s::success).set_flags(dpp::m_ephemeral));
    }
};

REGISTER_COMMAND(UnstickyCommand);

} // anonymous namespace
