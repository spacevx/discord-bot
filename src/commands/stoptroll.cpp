#include "bot/command.hpp"
#include "bot/strings.hpp"
#include "bot/trollmic.hpp"

namespace {

namespace s = bot::strings::cmd::stoptroll;

class StopTrollCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "stoptroll"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("stoptroll", s::description, bot_id)
            .set_default_permissions(dpp::p_mute_members);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_mute_members)) {
            event.reply(dpp::message(s::no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto& troll = bot::TrollMic::instance();
        auto session = troll.get(event.command.guild_id);
        if (!session) {
            event.reply(dpp::message(s::not_active).set_flags(dpp::m_ephemeral));
            return;
        }

        if (session->is_muted) {
            dpp::guild_member gm;
            gm.guild_id = event.command.guild_id;
            gm.user_id = session->target_id;
            gm.set_mute(false);
            event.from->creator->guild_edit_member(gm);
        }

        troll.remove(event.command.guild_id);
        event.from->disconnect_voice(event.command.guild_id);

        event.reply(dpp::message(s::success).set_flags(dpp::m_ephemeral));
    }
};

REGISTER_COMMAND(StopTrollCommand);

} // anonymous namespace
