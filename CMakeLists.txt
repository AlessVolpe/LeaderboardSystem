cmake_minimum_required(VERSION 3.28)
project(leaderboardsystem C)

set(CMAKE_C_STANDARD 11)

add_executable(leaderboardsystem main.c
        Player.h
        leaderboard.h
        error_handling.h
        player_pool.h
)
