cmake_minimum_required(VERSION 3.16)
project(leaderboard_external_consumer LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Leaderboard 3.0 CONFIG REQUIRED)
add_executable(leaderboard_external_consumer main.c)
target_link_libraries(leaderboard_external_consumer PRIVATE Leaderboard::leaderboard)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(leaderboard_external_consumer PRIVATE -Wall -Wextra -Werror -pedantic-errors)
endif()

enable_testing()
add_test(NAME leaderboard_external_consumer COMMAND leaderboard_external_consumer)

# The installed-consumer CTest must link the same sanitizer runtime when the
# source library was instrumented; ordinary package consumers do not use this.
option(LEADERBOARD_EXTERNAL_SANITIZERS "Match an instrumented test library" OFF)
if(LEADERBOARD_EXTERNAL_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(leaderboard_external_consumer PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(leaderboard_external_consumer PRIVATE -fsanitize=address,undefined)
endif()
