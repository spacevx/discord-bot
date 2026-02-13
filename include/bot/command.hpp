#pragma once

#include <dpp/dpp.h>

#include <memory>
#include <vector>

namespace bot {

class Command {
public:
    virtual ~Command() = default;
    virtual dpp::slashcommand definition(dpp::snowflake bot_id) const = 0;
    virtual void execute(const dpp::slashcommand_t& event) const = 0;
    virtual std::string_view name() const noexcept = 0;
};

using CommandFactory = std::unique_ptr<Command>(*)();

inline std::vector<CommandFactory>& get_command_registry() {
    static std::vector<CommandFactory> registry;
    return registry;
}

namespace detail {

struct CommandRegistrar {
    explicit CommandRegistrar(CommandFactory factory) {
        get_command_registry().push_back(factory);
    }
};

} // namespace detail
} // namespace bot

#define REGISTER_COMMAND(CommandClass)                                         \
    static ::bot::detail::CommandRegistrar                                     \
        registrar_##CommandClass(                                              \
            []() -> std::unique_ptr<::bot::Command> {                         \
                return std::make_unique<CommandClass>();                       \
            }                                                                 \
        )
