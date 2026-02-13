#pragma once

#include <dpp/dpp.h>

#include <array>
#include <cstddef>
#include <string>

namespace bot {

inline void start_presence_animation(dpp::cluster& bot) {
    static constexpr std::array frames = {
        "\xe2\x96\x8c",
        "F\xe2\x96\x8c",
        "Fr\xe2\x96\x8c",
        "Fre\xe2\x96\x8c",
        "Free\xe2\x96\x8c",
        "Free \xe2\x96\x8c",
        "Free P\xe2\x96\x8c",
        "Free Pa\xe2\x96\x8c",
        "Free Pal\xe2\x96\x8c",
        "Free Pale\xe2\x96\x8c",
        "Free Pales\xe2\x96\x8c",
        "Free Palest\xe2\x96\x8c",
        "Free Palesti\xe2\x96\x8c",
        "Free Palestin\xe2\x96\x8c",
        "Free Palestine\xe2\x96\x8c",
        "Free Palestine",
        "Free Palestine",
        "Free Palestine",
        "Free Palestine",
        "Free Palestine\xe2\x96\x8c",
        "Free Palestin\xe2\x96\x8c",
        "Free Palesti\xe2\x96\x8c",
        "Free Palest\xe2\x96\x8c",
        "Free Pales\xe2\x96\x8c",
        "Free Pale\xe2\x96\x8c",
        "Free Pal\xe2\x96\x8c",
        "Free Pa\xe2\x96\x8c",
        "Free P\xe2\x96\x8c",
        "Free \xe2\x96\x8c",
        "Free\xe2\x96\x8c",
        "Fre\xe2\x96\x8c",
        "Fr\xe2\x96\x8c",
        "F\xe2\x96\x8c",
        "\xe2\x96\x8c",
        "\xe2\x96\x8c",
        "\xe2\x96\x8c",
    };

    static std::size_t index = 0;

    bot.start_timer([&bot](dpp::timer) {
        dpp::activity act(dpp::at_custom, "status", frames[index], "");
        bot.set_presence(dpp::presence(dpp::ps_online, act));
        index = (index + 1) % frames.size();
    }, 2);
}

} // namespace bot
