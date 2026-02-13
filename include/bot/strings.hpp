#pragma once

namespace bot::strings {

namespace log {
    constexpr auto token_missing    = "DISCORD_BOT_TOKEN n'est pas definie.\n";
    constexpr auto starting         = "Demarrage du bot...\n";
    constexpr auto online           = "Bot en ligne : ";
    constexpr auto loaded           = "{} commande(s) chargee(s).\n";
    constexpr auto registered       = "{} commande(s) enregistree(s) sur Discord.\n";
}

namespace cmd {
    namespace ping {
        constexpr auto description  = "Repond avec pong !";
        constexpr auto reply        = "Pong !";
    }

    namespace info {
        constexpr auto description  = "Affiche les informations du bot";
        constexpr auto title        = "Informations du Bot";
        constexpr auto library      = "Librairie";
        constexpr auto cpp_standard = "Standard C++";
        constexpr auto uptime       = "Temps de fonctionnement";
    }

    namespace kick {
        constexpr auto description      = "Expulser un membre du serveur";
        constexpr auto opt_user         = "Le membre a expulser";
        constexpr auto opt_reason       = "Raison de l'expulsion";
        constexpr auto no_permission    = "Vous n'avez pas la permission d'expulser des membres.";
        constexpr auto no_reason        = "Aucune raison fournie";
        constexpr auto success          = "Expulse <@{}>.";
        constexpr auto failure          = "Impossible d'expulser <@{}> : {}";
    }

    namespace ban {
        constexpr auto description      = "Bannir un membre du serveur";
        constexpr auto opt_user         = "Le membre a bannir";
        constexpr auto opt_reason       = "Raison du bannissement";
        constexpr auto no_permission    = "Vous n'avez pas la permission de bannir des membres.";
        constexpr auto no_reason        = "Aucune raison fournie";
        constexpr auto success          = "Banni <@{}>.";
        constexpr auto failure          = "Impossible de bannir <@{}> : {}";
    }

    namespace sticky {
        constexpr auto description      = "Epingler un message en bas du salon";
        constexpr auto opt_message      = "Le contenu du message a epingler";
        constexpr auto no_permission    = "Vous n'avez pas la permission de gerer les messages.";
        constexpr auto success          = "Message epingle dans ce salon.";
    }

    namespace unsticky {
        constexpr auto description      = "Retirer le message epingle du salon";
        constexpr auto no_permission    = "Vous n'avez pas la permission de gerer les messages.";
        constexpr auto success          = "Message epingle retire.";
        constexpr auto not_found        = "Aucun message epingle dans ce salon.";
    }
}

} // namespace bot::strings
