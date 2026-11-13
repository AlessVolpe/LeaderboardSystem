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
