#pragma once

#include "command.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace bot {

class CommandRouter {
public:
    void load_all();
    void register_commands(dpp::cluster& bot) const;
    void dispatch(const dpp::slashcommand_t& event) const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};

} // namespace bot
