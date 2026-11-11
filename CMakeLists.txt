cmake_minimum_required(VERSION 3.16)
project(leaderboardsystem VERSION 3.0.0 LANGUAGES C)

include(CTest)

option(LEADERBOARD_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(LEADERBOARD_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# The current type-safe hashmap API intentionally uses GNU C extensions such as typeof and
# statement expressions. Removing that dependency belongs to a later refactor, not 3.0.1.
set(CMAKE_C_EXTENSIONS ON)

function(leaderboard_apply_dev_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(LEADERBOARD_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
        if(LEADERBOARD_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()

        if(LEADERBOARD_ENABLE_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        endif()
    endif()
endfunction()

add_library(leaderboard
    src/hashmap.c
    src/bostree.c
    src/mtwister.c
    src/player.c
    src/commons.c
)
add_library(Leaderboard::leaderboard ALIAS leaderboard)

set_target_properties(leaderboard PROPERTIES
    OUTPUT_NAME leaderboard
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(leaderboard
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
leaderboard_apply_dev_options(leaderboard)

add_executable(leaderboard_cli
    examples/leaderboard_cli.c
        examples/leaderboard_cli.c
)
target_link_libraries(leaderboard_cli PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(leaderboard_cli)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
