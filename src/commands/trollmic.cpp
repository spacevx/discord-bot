#include "bot/command.hpp"
#include "bot/strings.hpp"
#include "bot/trollmic.hpp"

#include <format>

namespace {

namespace s = bot::strings::cmd::trollmic;

class TrollMicCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "trollmic"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("trollmic", s::description, bot_id)
            .add_option(dpp::command_option(dpp::co_user, "cible", s::opt_target, true))
            .set_default_permissions(dpp::p_mute_members);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_mute_members)) {
            event.reply(dpp::message(s::no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto& troll = bot::TrollMic::instance();
        if (troll.has(event.command.guild_id)) {
            event.reply(dpp::message(s::already_active).set_flags(dpp::m_ephemeral));
            return;
        }

        auto* g = dpp::find_guild(event.command.guild_id);
        if (!g) return;

        auto vsi = g->voice_members.find(event.command.get_issuing_user().id);
        if (vsi == g->voice_members.end()) {
            event.reply(dpp::message(s::not_in_voice).set_flags(dpp::m_ephemeral));
            return;
        }

        auto target = std::get<dpp::snowflake>(event.get_parameter("cible"));
        auto vc_id = vsi->second.channel_id;

        troll.set(event.command.guild_id, target, vc_id);
        event.from->connect_voice(event.command.guild_id, vc_id, false, false);

        event.reply(dpp::message(std::format(s::success, std::to_string(target))).set_flags(dpp::m_ephemeral));
    }
};

REGISTER_COMMAND(TrollMicCommand);

} // anonymous namespace
