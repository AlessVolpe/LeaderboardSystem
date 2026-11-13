add_executable(test_hashmap test_hashmap.c)
target_link_libraries(test_hashmap PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_hashmap)
add_test(NAME hashmap_smoke COMMAND test_hashmap)

add_executable(test_hashmap_regressions test_hashmap_regressions.c)
target_link_libraries(test_hashmap_regressions PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_hashmap_regressions)
add_test(NAME hashmap_regressions COMMAND test_hashmap_regressions)

add_executable(test_hashmap_macros test_hashmap_macros.c)
target_link_libraries(test_hashmap_macros PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_hashmap_macros)
add_test(NAME hashmap_typed_macros COMMAND test_hashmap_macros)

add_executable(test_bostree test_bostree.c)
target_link_libraries(test_bostree PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_bostree)
add_test(NAME bostree_smoke COMMAND test_bostree)

add_executable(test_bostree_invariants test_bostree_invariants.c)
target_link_libraries(test_bostree_invariants PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_bostree_invariants)
add_test(NAME bostree_invariants COMMAND test_bostree_invariants)

add_executable(test_bostree_deletion test_bostree_deletion.c)
target_link_libraries(test_bostree_deletion PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_bostree_deletion)
add_test(NAME bostree_deletion COMMAND test_bostree_deletion)

# Exercise the actual interactive example, including cleanup under sanitizers.
add_test(NAME leaderboard_cli_unique COMMAND ${CMAKE_COMMAND}
    "-DCLI=$<TARGET_FILE:leaderboard_cli>"
    "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_unique.txt"
    -DEXPECT_INSERTED=5
    -DEXPECT_DUPLICATES=0
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_leaderboard_cli.cmake)

add_test(NAME leaderboard_cli_duplicate COMMAND ${CMAKE_COMMAND}
    "-DCLI=$<TARGET_FILE:leaderboard_cli>"
    "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_duplicate.txt"
    -DEXPECT_INSERTED=4
    -DEXPECT_DUPLICATES=1
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_leaderboard_cli.cmake)

add_test(NAME leaderboard_cli_invalid_count COMMAND ${CMAKE_COMMAND}
    "-DCLI=$<TARGET_FILE:leaderboard_cli>"
    "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_invalid_count.txt"
    -DEXPECT_ERROR=1
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_leaderboard_cli.cmake)

add_test(NAME leaderboard_cli_incomplete_input COMMAND ${CMAKE_COMMAND}
    "-DCLI=$<TARGET_FILE:leaderboard_cli>"
    "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_incomplete.txt"
    -DEXPECT_INCOMPLETE=1
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_leaderboard_cli.cmake)

# 3.0.5: deterministic ranking API and independent reference-model tests.
add_executable(test_ranking test_ranking.c)
target_link_libraries(test_ranking PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_ranking)
add_test(NAME ranking_regressions COMMAND test_ranking)

add_executable(test_ranking_differential test_ranking_differential.c)
target_link_libraries(test_ranking_differential PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_ranking_differential)
add_test(NAME ranking_differential COMMAND test_ranking_differential)

# 3.0.6: structural white-box tests explicitly opt into private headers;
# applications and public API tests do not receive that include directory.
target_include_directories(test_bostree_invariants PRIVATE
    ${PROJECT_SOURCE_DIR}/src/internal)
target_include_directories(test_bostree_deletion PRIVATE
    ${PROJECT_SOURCE_DIR}/src/internal)

add_executable(test_bostree_public_api test_bostree_public_api.c)
target_link_libraries(test_bostree_public_api PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_bostree_public_api)
add_test(NAME bostree_public_api COMMAND test_bostree_public_api)

# The base regressions inspect the table as an explicitly private white-box test.
target_include_directories(test_hashmap_regressions PRIVATE
    ${PROJECT_SOURCE_DIR}/src/internal)

add_executable(test_hashmap_public_api test_hashmap_public_api.c)
target_link_libraries(test_hashmap_public_api PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_hashmap_public_api)
add_test(NAME hashmap_public_api COMMAND test_hashmap_public_api)

# The original anonymous typed wrappers still require GNU C extensions.
set_target_properties(test_hashmap test_hashmap_macros test_hashmap_public_api
    PROPERTIES C_EXTENSIONS ON)

# New C11 typed wrappers never cast between incompatible function pointer types.
add_executable(test_hashmap_portable test_hashmap_portable.c)
target_link_libraries(test_hashmap_portable PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_hashmap_portable)
set_target_properties(test_hashmap_portable PROPERTIES C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_hashmap_portable PRIVATE -pedantic-errors)
endif()
add_test(NAME hashmap_portable_callbacks COMMAND test_hashmap_portable)

# Fault injection is compiled only into a separate test archive. Production
# library objects do not expose the failure-control functions.
add_library(leaderboard_fault_test STATIC
    ${PROJECT_SOURCE_DIR}/src/hashmap.c
    ${PROJECT_SOURCE_DIR}/src/bostree.c
    ${PROJECT_SOURCE_DIR}/src/ranking.c
    ${PROJECT_SOURCE_DIR}/src/alloc.c)
target_include_directories(leaderboard_fault_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/internal)
target_compile_definitions(leaderboard_fault_test PRIVATE LEADERBOARD_TEST_ALLOCATION_FAULTS)
leaderboard_apply_dev_options(leaderboard_fault_test)
add_executable(test_allocation_failures test_allocation_failures.c)
target_include_directories(test_allocation_failures PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/internal)
target_compile_definitions(test_allocation_failures PRIVATE LEADERBOARD_TEST_ALLOCATION_FAULTS)
target_link_libraries(test_allocation_failures PRIVATE leaderboard_fault_test)
leaderboard_apply_dev_options(test_allocation_failures)
add_test(NAME allocation_failures COMMAND test_allocation_failures)

add_test(NAME installed_consumer
    COMMAND "${CMAKE_COMMAND}"
        "-DROOT=${PROJECT_SOURCE_DIR}"
        "-DBUILD_ROOT=${PROJECT_BINARY_DIR}"
        "-DGENERATOR=${CMAKE_GENERATOR}"
        "-DC_COMPILER=${CMAKE_C_COMPILER}"
        "-DCONFIG=$<CONFIG>"
        "-DSANITIZERS=${LEADERBOARD_ENABLE_SANITIZERS}"
        -P "${PROJECT_SOURCE_DIR}/tests/verify_installed_consumer.cmake")

# 3.0.8: exhaustive logical state checks around controlled allocation failures.
add_executable(test_failure_matrix test_failure_matrix.c)
target_include_directories(test_failure_matrix PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/internal)
target_compile_definitions(test_failure_matrix PRIVATE LEADERBOARD_TEST_ALLOCATION_FAULTS)
target_link_libraries(test_failure_matrix PRIVATE leaderboard_fault_test)
leaderboard_apply_dev_options(test_failure_matrix)
add_test(NAME allocation_failure_matrix COMMAND test_failure_matrix)

# Malformed inputs and boundary-value behavior on the supported public API.
add_executable(test_api_edges test_api_edges.c)
target_link_libraries(test_api_edges PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_api_edges)
add_test(NAME api_adversarial_edges COMMAND test_api_edges)

# Identical input-driven state machines serve both CTest and optional libFuzzer.
foreach (container IN ITEMS hashmap bostree)
    add_executable(test_fuzz_${container} fuzz_${container}.c)
    target_link_libraries(test_fuzz_${container} PRIVATE Leaderboard::leaderboard)
    target_include_directories(test_fuzz_${container} PRIVATE
        ${PROJECT_SOURCE_DIR}/src/internal)
    leaderboard_apply_dev_options(test_fuzz_${container})
    add_test(NAME ${container}_adversarial COMMAND test_fuzz_${container})
endforeach ()

if (LEADERBOARD_BUILD_FUZZERS)
    if (NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "LEADERBOARD_BUILD_FUZZERS requires Clang with libFuzzer")
    endif ()

    # Not every Clang target supports the libFuzzer driver. In particular,
    # MSYS2 UCRT64's x86_64-w64-windows-gnu target rejects -fsanitize=fuzzer.
    # Probe both the instrument-only flag and an actual instrumented link.
    include(CheckCCompilerFlag)
    # The no-link flag needs a compile-only probe: its instrumented object
    # cannot link by itself because libFuzzer supplies coverage callbacks.
    set(_leaderboard_saved_try_compile_type "${CMAKE_TRY_COMPILE_TARGET_TYPE}")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    check_c_compiler_flag("-fsanitize=fuzzer-no-link"
        LEADERBOARD_LIBFUZZER_NO_LINK_SUPPORTED)
    if (_leaderboard_saved_try_compile_type)
        set(CMAKE_TRY_COMPILE_TARGET_TYPE "${_leaderboard_saved_try_compile_type}")
    else ()
        unset(CMAKE_TRY_COMPILE_TARGET_TYPE)
    endif ()
    unset(_leaderboard_saved_try_compile_type)
    include(CheckCSourceCompiles)
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=fuzzer,address,undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=fuzzer,address,undefined")
    check_c_source_compiles("#include <stddef.h>\n#include <stdint.h>\nint LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) { (void)data; (void)size; return 0; }"
        LEADERBOARD_LIBFUZZER_LINK_SUPPORTED)
    unset(CMAKE_REQUIRED_FLAGS)
    unset(CMAKE_REQUIRED_LINK_OPTIONS)
    if (NOT LEADERBOARD_LIBFUZZER_NO_LINK_SUPPORTED OR
        NOT LEADERBOARD_LIBFUZZER_LINK_SUPPORTED)
        message(FATAL_ERROR
            "Clang compiler '${CMAKE_C_COMPILER}' on ${CMAKE_SYSTEM_NAME} "
            "does not support the libFuzzer instrumentation and runtime used by this build. "
            "For deterministic adversarial CTest runs, reconfigure with "
            "-DLEADERBOARD_BUILD_FUZZERS=OFF; for coverage-guided libFuzzer "
            "campaigns use a compatible Clang target (for example Linux/WSL).")
    endif ()

    # Instrument the data structures themselves, not only the fuzz entry points.
    add_library(leaderboard_fuzz_core STATIC
        ${PROJECT_SOURCE_DIR}/src/hashmap.c
        ${PROJECT_SOURCE_DIR}/src/bostree.c
        ${PROJECT_SOURCE_DIR}/src/alloc.c)
    target_include_directories(leaderboard_fuzz_core PRIVATE
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/internal)
    target_compile_options(leaderboard_fuzz_core PRIVATE
        -fsanitize=fuzzer-no-link,address,undefined -fno-omit-frame-pointer)
    leaderboard_apply_dev_options(leaderboard_fuzz_core)
    foreach (container IN ITEMS hashmap bostree)
        add_executable(fuzz_${container}_libfuzzer fuzz_${container}.c)
        target_compile_definitions(fuzz_${container}_libfuzzer PRIVATE LEADERBOARD_LIBFUZZER)
        target_include_directories(fuzz_${container}_libfuzzer PRIVATE
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/internal)
        target_link_libraries(fuzz_${container}_libfuzzer PRIVATE leaderboard_fuzz_core)
        target_compile_options(fuzz_${container}_libfuzzer PRIVATE
            -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
        target_link_options(fuzz_${container}_libfuzzer PRIVATE
            -fsanitize=fuzzer,address,undefined)
        leaderboard_apply_dev_options(fuzz_${container}_libfuzzer)
    endforeach ()
endif ()
