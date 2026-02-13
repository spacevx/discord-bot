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

    namespace cfxstatus {
        constexpr auto description      = "Configurer le suivi du statut CFX.re";
        constexpr auto no_permission    = "Vous n'avez pas la permission de configurer le suivi.";
        constexpr auto success          = "Suivi du statut CFX.re configure dans ce salon.";
        constexpr auto embed_title      = "Statut CFX.re";
        constexpr auto embed_footer     = "Derniere mise a jour";
        constexpr auto no_incidents     = ":white_check_mark: Aucun incident recent.";
        constexpr auto status_up        = ":white_check_mark:";
        constexpr auto status_down      = ":x:";
    }

    namespace news {
        constexpr auto setup_description    = "Configurer un flux d'actualites dans ce salon";
        constexpr auto setup_opt_category   = "La categorie d'actualites a suivre";
        constexpr auto setup_no_permission  = "Vous n'avez pas la permission de configurer les actualites.";
        constexpr auto setup_success        = "Flux d'actualites '{}' configure dans ce salon.";
        constexpr auto remove_description   = "Retirer un flux d'actualites de ce salon";
        constexpr auto remove_opt_category  = "La categorie d'actualites a retirer";
        constexpr auto remove_no_permission = "Vous n'avez pas la permission de retirer les actualites.";
        constexpr auto remove_success       = "Flux d'actualites '{}' retire de ce salon.";
        constexpr auto remove_not_found     = "Ce salon n'est pas abonne au flux '{}'.";
        constexpr auto source               = "Source";

        constexpr auto cat_palestine    = "Palestine / Conflit";
        constexpr auto cat_usa          = "USA";
        constexpr auto cat_trump        = "Trump";
        constexpr auto cat_musk         = "Elon Musk";
        constexpr auto cat_iran         = "Iran";
        constexpr auto cat_epstein      = "Epstein";
        constexpr auto cat_monde        = "Monde";
        constexpr auto cat_freegames    = "Jeux Gratuits";
        constexpr auto cat_cybersec     = "Cybersecurite / Darknet";
    }
}

} // namespace bot::strings
