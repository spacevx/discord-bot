#include "bot/command.hpp"
#include "bot/strings.hpp"

#include <string>

namespace {

namespace s = bot::strings::cmd::info;

class InfoCommand final : public bot::Command {
public:
    std::string_view name() const noexcept override { return "info"; }

    dpp::slashcommand definition(dpp::snowflake bot_id) const override {
        return dpp::slashcommand("info", s::description, bot_id);
    }

    void execute(const dpp::slashcommand_t& event) const override {
        auto* bot = event.from->creator;

        dpp::embed embed = dpp::embed()
            .set_title(s::title)
            .set_color(dpp::colors::blurple)
            .add_field(s::library, "D++ (DPP)")
            .add_field(s::cpp_standard, "C++20")
            .add_field(s::uptime,
                        std::to_string(bot->uptime().to_secs()) + "s");

        event.reply(dpp::message(event.command.channel_id, embed));
    }
};

REGISTER_COMMAND(InfoCommand);

} // anonymous namespace
