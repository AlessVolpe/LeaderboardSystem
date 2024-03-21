#include <stdlib.h>
#include <string.h>

#include "commons.h"
#include "hashmap.h"
#include "hashmap_base.h"
#include "mtwister.h"
#include "player.h"

#define MAX_PLAYERS     10
#define MIN_PLAYERS     6
#define MAX_ID_SIZE     32

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

static char* _set_player_id(const int counter) {
    char* id = malloc(MAX_ID_SIZE);
    MTRand seed = seedRand(counter + rand());
    snprintf(id, sizeof(id), "%u", genRandLong(&seed));
    return id;
}

int main(int argc, char** argv) {
    const int player_num = _select_players();
    char** player_names = _name_players(player_num);

    // declare type-specific hashmap structure
    HASHMAP(char, player_t) map;
    char* key;
    player_t* data;

    // initialize with default string key hash function and comparator
    hashmap_init(&map, hashmap_hash_string, strcmp);

    // load players into the map and discard duplicates
    for (int i = 0; i < player_num; i++) {
        char* player_id = _set_player_id(i);
        player_t* player = player_create(player_names[i]);
        const int ret = hashmap_put(&map, player_id, player);
        printf("putting player %s[%s] success!\n", player->name, player_id);
        if (ret < 0) printf("putting player %s[%s] failed: %s\n", player->name, player_id, strerror(-ret));
    }
    free(player_names);

    // iterate through all players and print each one
    printf("\nFormat: player_name[player_id] with score n\n");
    hashmap_foreach(key, data, &map) {
        printf("Player: %s[%s] with score %d\n", data->name, key, data->score);
    }

    // destruct the hashmap
    hashmap_cleanup(&map);
    return 0;
}
