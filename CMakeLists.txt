cmake_minimum_required(VERSION 3.16)
project(leaderboardsystem VERSION 3.0.10.5 LANGUAGES C)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(LEADERBOARD_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(LEADERBOARD_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(LEADERBOARD_BUILD_FUZZERS "Build Clang/libFuzzer harnesses (requires BUILD_TESTING)" OFF)
option(LEADERBOARD_BUILD_BENCHMARKS "Build deterministic performance benchmarks" OFF)

# Check compile AND link, because some GCC distributions accept sanitizer
# flags but do not ship the corresponding runtime libraries (notably UCRT64).
if (LEADERBOARD_ENABLE_SANITIZERS)
    if (NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "LEADERBOARD_ENABLE_SANITIZERS requires GCC or Clang")
    endif ()
    include(CheckCSourceCompiles)
    include(CMakePushCheckState)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address,undefined")
    check_c_source_compiles("int main(void) { return 0; }"
        LEADERBOARD_ASAN_UBSAN_LINK_SUPPORTED)
    cmake_pop_check_state()
    if (NOT LEADERBOARD_ASAN_UBSAN_LINK_SUPPORTED)
        message(FATAL_ERROR
            "The selected toolchain cannot compile and link ASan/UBSan. "
            "Use -DLEADERBOARD_ENABLE_SANITIZERS=OFF or a supported "
            "Linux/WSL GCC or Clang toolchain.")
    endif ()
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Native modules and the new named typed hashmap API use ISO C11.
# Deprecated HASHMAP(...) macro clients still require GNU C extensions.
set(CMAKE_C_EXTENSIONS OFF)

function(leaderboard_apply_dev_options target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if (LEADERBOARD_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif ()
    else ()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
        if (LEADERBOARD_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif ()

        if (LEADERBOARD_ENABLE_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        endif ()
    endif ()
endfunction()

# Compile modules independently while preserving the original single static
# archive and its Leaderboard::leaderboard link target for existing consumers.
add_library(leaderboard_containers_obj OBJECT
        src/hashmap.c
        src/bostree.c
        src/alloc.c
)
add_library(leaderboard_ranking_obj OBJECT
        src/ranking.c
)
add_library(leaderboard_legacy_obj OBJECT
        src/mtwister.c
        src/player.c
        src/commons.c
)

foreach (component IN ITEMS containers ranking legacy)
    target_include_directories(leaderboard_${component}_obj PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include)
    leaderboard_apply_dev_options(leaderboard_${component}_obj)
    set_target_properties(leaderboard_${component}_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
endforeach ()
# Only BOSTree and structural invariant tests can see the representation.
target_include_directories(leaderboard_containers_obj PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal)
target_include_directories(leaderboard_ranking_obj PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal)

add_library(leaderboard STATIC
        $<TARGET_OBJECTS:leaderboard_containers_obj>
        $<TARGET_OBJECTS:leaderboard_ranking_obj>
        $<TARGET_OBJECTS:leaderboard_legacy_obj>
)
add_library(Leaderboard::leaderboard ALIAS leaderboard)

set_target_properties(leaderboard PROPERTIES
        OUTPUT_NAME leaderboard
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(leaderboard PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
leaderboard_apply_dev_options(leaderboard)

add_executable(leaderboard_cli
        examples/leaderboard_cli.c
)
target_link_libraries(leaderboard_cli PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(leaderboard_cli)

# Deterministic, non-interactive public ranking API example.
add_executable(leaderboard_ranking_batch examples/ranking_batch.c)
target_link_libraries(leaderboard_ranking_batch PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(leaderboard_ranking_batch)

if (LEADERBOARD_BUILD_BENCHMARKS)
    add_executable(leaderboard_bench benchmarks/benchmark.c)
    target_link_libraries(leaderboard_bench PRIVATE Leaderboard::leaderboard)
    leaderboard_apply_dev_options(leaderboard_bench)
    set_target_properties(leaderboard_bench PROPERTIES C_EXTENSIONS OFF)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(leaderboard_bench PRIVATE -pedantic-errors)
    endif ()
endif ()

if (BUILD_TESTING)
    add_subdirectory(tests)
    if (LEADERBOARD_BUILD_BENCHMARKS)
        # Correctness-only smoke check. Never gate CI on wall/CPU times.
        add_test(NAME benchmark_smoke COMMAND leaderboard_bench 1000 1)
    endif ()
endif ()

# Build-tree consumers keep using Leaderboard::leaderboard; installed consumers
# get the same target via find_package(Leaderboard CONFIG REQUIRED).
install(TARGETS leaderboard EXPORT LeaderboardTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING PATTERN "*.h")
set(package_dir "${CMAKE_INSTALL_LIBDIR}/cmake/Leaderboard")
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/LeaderboardConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/LeaderboardConfig.cmake"
    INSTALL_DESTINATION "${package_dir}")
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/LeaderboardConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMajorVersion)
install(FILES "${PROJECT_BINARY_DIR}/LeaderboardConfig.cmake"
              "${PROJECT_BINARY_DIR}/LeaderboardConfigVersion.cmake"
        DESTINATION "${package_dir}")
install(EXPORT LeaderboardTargets NAMESPACE Leaderboard::
        DESTINATION "${package_dir}")
