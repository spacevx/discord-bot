#include "bot/cfx_status.hpp"
#include "bot/command.hpp"
#include "bot/strings.hpp"

namespace {

namespace s = bot::strings::cmd::cfxstatus;

class SetupCfxStatusCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "setupcfxstatus"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("setupcfxstatus", s::description, bot_id)
            .set_default_permissions(dpp::p_manage_messages);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        if (!event.command.member.permissions.has(dpp::p_manage_messages)) {
            event.reply(dpp::message(s::no_permission).set_flags(dpp::m_ephemeral));
            return;
        }

        auto* bot = event.from->creator;
        auto channel_id = event.command.channel_id;

        event.reply(dpp::message(s::success).set_flags(dpp::m_ephemeral));

        bot->request("https://status.cfx.re/history.rss", dpp::m_get,
            [bot, channel_id](const dpp::http_request_completion_t& response) {
                auto incidents = bot::CfxStatus::parse_rss(
                    response.status == 200 ? response.body : "");
                auto embed = bot::CfxStatus::build_embed(incidents);

                bot->message_create(dpp::message(channel_id, embed),
                    [channel_id](const dpp::confirmation_callback_t& cb) {
                        if (!cb.is_error()) {
                            auto msg = cb.get<dpp::message>();
                            bot::CfxStatus::instance().add_channel(channel_id, msg.id);
                        }
                    });
            });
    }
};

REGISTER_COMMAND(SetupCfxStatusCommand);

} // anonymous namespace
