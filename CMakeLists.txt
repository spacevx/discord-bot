cmake_minimum_required(VERSION 3.22)
project(discord-bot
    VERSION 0.1.0
    DESCRIPTION "Discord bot using D++ (DPP)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(FetchContent)

FetchContent_Declare(
    dpp
    GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
    GIT_TAG        v10.1.4
    GIT_SHALLOW    TRUE
)

set(DPP_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_VOICE_SUPPORT OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(DPP_USE_PCH ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(dpp)

# OBJECT library ensures self-registering command TUs are not discarded by the linker
add_library(bot_commands OBJECT
    src/commands/ping.cpp
    src/commands/info.cpp
    src/commands/kick.cpp
    src/commands/ban.cpp
    src/commands/sticky.cpp
    src/commands/unsticky.cpp
)

target_include_directories(bot_commands PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(bot_commands PRIVATE dpp)

add_executable(discord-bot
    src/main.cpp
    src/bot/command_router.cpp
)

target_include_directories(discord-bot PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(discord-bot PRIVATE dpp bot_commands)

if(MSVC)
    target_compile_options(discord-bot PRIVATE /W4 /permissive-)
    target_compile_options(bot_commands PRIVATE /W4 /permissive-)
else()
    target_compile_options(discord-bot PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(bot_commands PRIVATE -Wall -Wextra -Wpedantic)
endif()
