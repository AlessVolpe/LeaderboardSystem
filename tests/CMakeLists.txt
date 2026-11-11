add_executable(test_hashmap test_hashmap.c)
target_link_libraries(test_hashmap PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_hashmap)
add_test(NAME hashmap_smoke COMMAND test_hashmap)

add_executable(test_bostree test_bostree.c)
target_link_libraries(test_bostree PRIVATE Leaderboard::leaderboard)
leaderboard_apply_dev_options(test_bostree)
add_test(NAME bostree_smoke COMMAND test_bostree)
