#include <stdlib.h>
#include <string.h>

#include "bostree.h"
#include "commons.h"
#include "hashmap.h"
#include "hashmap_base.h"
#include "mtwister.h"
#include "player.h"


static void _tree_free(const BOSNode* node) {
    free(node->key);
    free(node->data);
}

static int _select_players(void) {
    int n;
    printf("Insert player number: \n");
    const int ret = scanf("%d", &n);
    ERROR_HELPER(ret, "STDIN_ERR: UNEXPECTED_ERROR");
    GENERIC_ERROR_HELPER(((n < MIN_PLAYERS | n > MAX_PLAYERS)), errno, "GENERIC_ERR: WRONG_PLAYER_NUMBER");
    return n;
}

static char** _name_players(const int num) {
    char** names = malloc(num * sizeof(char*));
    for (int i = 0; i < num; i++) {
        char* name = malloc(MAX_NAME_SIZE);
        printf("Insert player name: \n");
        const int ret = scanf("%s", name);
        ERROR_HELPER(ret, "STDIN_ERR: UNEXPECTED_ERROR");
        names[i] = name;
    }
    return names;
}

static char* _set_player_id() {
    char* id = malloc(MAX_ID_SIZE);
    snprintf(id, MAX_ID_SIZE, "%s", uuid_generate());
    return id;
}

static void _free_player_names(const int num, char** name_arr) {
    for (size_t i = 0; i < num; i++) {
        free(name_arr[i]);
    }
    free(name_arr);
}

int main(int argc, char** argv) {
    const int player_num = _select_players();
    char** player_names = _name_players(player_num);

    // declare type-specific hashmap structure and a temp var for safe foreach
    HASHMAP(char, player_t) pool;
    char* key;
    player_t* data;
    void* temp;

    // initialize with default string key hash function and comparator
    hashmap_init(&pool, hashmap_hash_string, strcmp);

    // load players into the map and discard duplicates
    for (int i = 0; i < player_num; i++) {
        char* player_id = _set_player_id();
        player_t* player = player_create(player_names[i]);
        const int ret = hashmap_put(&pool, player_id, player);
        printf("putting player %s[%s] success!\n", player->name, player_id);
        if (ret < 0) printf("putting player %s[%s] failed: %s\n", player->name, player_id, strerror(-ret));
        free(player_id);
    }

    if (player_names) _free_player_names(player_num, player_names);

    // iterate through all players and print each one
    printf("\nFormat: player_name[player_id] with score n\n");
    hashmap_foreach(key, data, &pool) {
        MTRand seed = seedRand(rand());
        player_adding_points(data, (uint32_t)(genRand(&seed) * 100)); // adding a random ammount of points
        printf("Player: %s[%s] with score %d\n", data->name, key, data->score);
    }
    printf("\n");

    BOSTree* leaderboard = bostree_new((BOSTree_cmp_func)strcmp, _tree_free);

    hashmap_foreach_key_safe(key, &pool, temp) {
        const player_t* data_ptr = hashmap_remove(&pool, key);
        char* tr_key = data_ptr->name;
        bostree_insert(leaderboard, tr_key, (void*)data_ptr->score);
        printf("Inserted player %s with %d points.\n", tr_key, data_ptr->score);
    }

#ifndef NDEBUG
    bostree_print(leaderboard);
#endif

    // destroy hashmap
    hashmap_cleanup(&pool);
    //bostree_destroy(leaderboard);

    return 0;
}
