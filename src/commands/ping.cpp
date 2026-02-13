#include "bot/command.hpp"
#include "bot/strings.hpp"

namespace {

class PingCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "ping"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("ping", bot::strings::cmd::ping::description, bot_id);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        event.reply(bot::strings::cmd::ping::reply);
    }
};

REGISTER_COMMAND(PingCommand);

} // anonymous namespace
