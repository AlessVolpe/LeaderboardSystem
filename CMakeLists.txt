cmake_minimum_required(VERSION 3.5)
project(leaderboardsystem VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

add_executable(leaderboardsystem
        src/hashmap.c
        include/hashmap.h
        include/hashmap_base.h
        main.c
        src/bostree.c
        include/bostree.h
        include/mtwister.h
        src/mtwister.c
        include/player.h
        src/player.c
        src/commons.c
)

target_include_directories(leaderboardsystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboardsystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)