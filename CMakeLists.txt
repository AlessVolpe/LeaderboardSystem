cmake_minimum_required(VERSION 3.28)
project(leaderboardsystem C)

set(CMAKE_C_STANDARD 11)

add_executable(leaderboardsystem main.c
        headers/player.h
        headers/leaderboard.h
        headers/error_handling.h
        headers/player_pool.h
        headers/linked_list.h
        headers/overflow.h
        headers/id_generation.h
        id_generation.c
)
