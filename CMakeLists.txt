cmake_minimum_required(VERSION 3.5)
project(leaderboardsystem VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(leaderboardsystem
        src/commons.c
        src/hashmap.c
        include/commons.h
        include/hashmap.h
        include/hashmap_base.h
        main.c
)

target_include_directories(leaderboardsystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboardsystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)