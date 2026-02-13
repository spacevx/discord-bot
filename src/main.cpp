#include <dpp/dpp.h>

#include "bot/cfx_status.hpp"
#include "bot/command_router.hpp"
#include "bot/news.hpp"
#include "bot/presence.hpp"
#include "bot/sticky.hpp"
#include "bot/strings.hpp"
#include "bot/trollmic.hpp"
#include "bot/voice_trigger.hpp"

#include <chrono>
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

    dpp::cluster bot(token, dpp::i_guilds | dpp::i_guild_messages | dpp::i_message_content | dpp::i_guild_voice_states);

    bot::CommandRouter commands;
    commands.load_all();

    const char* whisper_model = std::getenv("WHISPER_MODEL_PATH");
    if (whisper_model) {
        auto& vt = bot::VoiceTrigger::instance();
        if (vt.init(whisper_model, [&bot](uint64_t guild_id) {
            std::cout << bot::strings::cmd::voicetrigger::triggered;

            auto& troll = bot::TrollMic::instance();
            auto session = troll.get(guild_id);
            if (!session) return;

            if (session->is_muted) {
                dpp::guild_member gm;
                gm.guild_id = guild_id;
                gm.user_id = session->target_id;
                gm.set_mute(false);
                bot.guild_edit_member(gm);
            }

            troll.remove(guild_id);
            if (auto* shard = bot.get_shard(0)) shard->disconnect_voice(guild_id);
            bot::VoiceTrigger::instance().clear_buffer(guild_id);
        })) {
            std::cout << bot::strings::cmd::voicetrigger::model_loaded;
        }
    } else {
        std::cerr << bot::strings::cmd::voicetrigger::model_missing;
    }

    bot.on_ready([&bot, &commands](const dpp::ready_t&) {
        std::cout << bot::strings::log::online << bot.me.username << " ("
                  << bot.me.id << ")\n";

        if (dpp::run_once<struct register_commands>()) {
            commands.register_commands(bot);
            bot::CfxStatus::instance().start_polling(bot);
            bot::NewsFeed::instance().start_polling(bot);
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

    bot.on_voice_receive([&bot](const dpp::voice_receive_t& event) {
        auto guild_id = event.voice_client->server_id;
        auto& troll = bot::TrollMic::instance();
        auto session = troll.get(guild_id);
        if (!session) return;

        if (!event.audio_data.empty()) {
            bot::VoiceTrigger::instance().feed_audio(
                guild_id, event.audio_data.data(), event.audio_data.size());
        }

        if (event.user_id != session->target_id) return;

        troll.touch_voice(guild_id);

        if (!session->is_muted) {
            troll.set_muted(guild_id, true);
            dpp::guild_member gm;
            gm.guild_id = guild_id;
            gm.user_id = session->target_id;
            gm.set_mute(true);
            bot.guild_edit_member(gm);
        }
    });

    bot.start_timer([&bot](dpp::timer) {
        auto& troll = bot::TrollMic::instance();
        auto now = std::chrono::steady_clock::now();

        troll.for_each([&](uint64_t guild_id, bot::TrollSession& session) {
            if (!session.is_muted) return;
            if (session.last_voice_packet.time_since_epoch().count() == 0) return;

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - session.last_voice_packet).count();

            if (elapsed > 500) {
                session.is_muted = false;
                dpp::guild_member gm;
                gm.guild_id = guild_id;
                gm.user_id = session.target_id;
                gm.set_mute(false);
                bot.guild_edit_member(gm);
            }
        });
    }, 1);

    std::cout << bot::strings::log::starting;
    bot.start(dpp::st_wait);

    return 0;
}
