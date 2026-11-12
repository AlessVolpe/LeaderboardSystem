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
