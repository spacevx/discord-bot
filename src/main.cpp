#include <dpp/dpp.h>

#include "bot/cfx_status.hpp"
#include "bot/command_router.hpp"
#include "bot/presence.hpp"
#include "bot/sticky.hpp"
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

    dpp::cluster bot(token, dpp::i_guilds | dpp::i_guild_messages | dpp::i_message_content);

    bot::CommandRouter commands;
    commands.load_all();

    bot.on_ready([&bot, &commands](const dpp::ready_t&) {
        std::cout << bot::strings::log::online << bot.me.username << " ("
                  << bot.me.id << ")\n";

        if (dpp::run_once<struct register_commands>()) {
            commands.register_commands(bot);
            bot::CfxStatus::instance().start_polling(bot);
            bot::start_presence_animation(bot);
        }
    });

    bot.on_slashcommand([&commands](const dpp::slashcommand_t& event) {
        commands.dispatch(event);
    });

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        if (event.msg.author.id == bot.me.id) return;

        auto& mgr = bot::Stickies::instance();
        auto data = mgr.get(event.msg.channel_id);
        if (!data) return;

        if (data->last_message_id) {
            bot.message_delete(data->last_message_id, event.msg.channel_id);
        }

        auto channel_id = event.msg.channel_id;
        bot.message_create(dpp::message(channel_id, data->content),
            [channel_id](const dpp::confirmation_callback_t& cb) {
                if (!cb.is_error()) {
                    auto msg = cb.get<dpp::message>();
                    bot::Stickies::instance().update_message_id(channel_id, msg.id);
                }
            });
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
