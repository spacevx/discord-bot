#include <dpp/dpp.h>

#include "bot/command_router.hpp"
#include "bot/strings.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

static std::string get_bot_token() {
    const char* token = std::getenv("DISCORD_BOT_TOKEN");
    if (!token) {
        return {};
    }
    return token;
}

int main() {
    const std::string token = get_bot_token();
    if (token.empty()) {
        std::cerr << bot::strings::log::token_missing;
        return 1;
    }

    dpp::cluster bot(token, dpp::i_guilds);

    bot::CommandRouter commands;
    commands.load_all();

    bot.on_ready([&bot, &commands](const dpp::ready_t&) {
        std::cout << bot::strings::log::online << bot.me.username << " ("
                  << bot.me.id << ")\n";

        if (dpp::run_once<struct register_commands>()) {
            commands.register_commands(bot);
        }
    });

    bot.on_slashcommand([&commands](const dpp::slashcommand_t& event) {
        commands.dispatch(event);
    });

    bot.on_log([](const dpp::log_t& event) {
        if (event.severity >= dpp::ll_info) {
            std::cout << "[DPP] " << event.message << "\n";
        }
    });

    std::cout << bot::strings::log::starting;
    bot.start(dpp::st_wait);

    return 0;
}
