#include "bot/command.hpp"
#include "bot/strings.hpp"

#include <format>

namespace {

namespace s = bot::strings::cmd::ban;

class BanCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "ban"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("ban", s::description, bot_id)
            .add_option(dpp::command_option(dpp::co_user, "user", s::opt_user, true))
            .add_option(dpp::command_option(dpp::co_string, "reason", s::opt_reason, false));
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_ban_members)) {
            event.reply(dpp::message(s::no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto target = std::get<dpp::snowflake>(event.get_parameter("user"));
        std::string reason = s::no_reason;
        auto reason_param = event.get_parameter("reason");
        if (std::holds_alternative<std::string>(reason_param))
            reason = std::get<std::string>(reason_param);

        event.from->creator->set_audit_reason(reason)
            .guild_ban_add(event.command.guild_id, target, 0, [event, target](const dpp::confirmation_callback_t& cb) {
                if (cb.is_error()) {
                    event.reply(dpp::message(std::format(s::failure, std::to_string(target), cb.get_error().message)).set_flags(dpp::m_ephemeral));
                    return;
                }
                event.reply(dpp::message(std::format(s::success, std::to_string(target))).set_flags(dpp::m_ephemeral));
            });
    }
};

REGISTER_COMMAND(BanCommand);

} // anonymous namespace
