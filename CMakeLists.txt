cmake_minimum_required(VERSION 3.5)
project(leaderboardsystem VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address [-g] [-O]")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak [-g] [-O]")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined [-g] [-Ofast]")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory [-fPIE] [-pie] [-pie] [-fno-omit-frame-pointer] [-g] [-Ofast] ")

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