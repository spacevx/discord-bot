#include "bot/command_router.hpp"
#include "bot/strings.hpp"

#include <format>
#include <iostream>
#include <string>

namespace bot {

void CommandRouter::load_all() {
    for (auto& factory : get_command_registry()) {
        auto cmd = factory();
        std::string key{cmd->name()};
        commands_.emplace(std::move(key), std::move(cmd));
    }
    std::cout << std::format(strings::log::loaded, commands_.size());
}

void CommandRouter::register_commands(dpp::cluster& bot) const {
    std::vector<dpp::slashcommand> defs;
    defs.reserve(commands_.size());

    for (const auto& [name, cmd] : commands_) {
        defs.push_back(cmd->definition(bot.me.id));
    }

    bot.global_bulk_command_create(defs);
    std::cout << std::format(strings::log::registered, defs.size());
}

void CommandRouter::dispatch(const dpp::slashcommand_t& event) const {
    const std::string& cmd_name = event.command.get_command_name();
    auto it = commands_.find(cmd_name);
    if (it != commands_.end()) {
        it->second->execute(event);
    }
}

std::size_t CommandRouter::size() const noexcept {
    return commands_.size();
}

} // namespace bot
