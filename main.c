#include <stdio.h>
#include <string.h>

#include "headers/error_handling.h"
#include "headers/id_generation.h"
#include "headers/player_pool.h"

int main(void) {
    PlayerPool* player_pool = create_pool(2);

    for (int i = 0; i < player_pool->size; i++) {
        char player_name[30];
        printf("Inserisci il nome del giocatore: \n");
        const int ret_value = scanf("%s", player_name);
        if (ret_value < 0) handle_error("STDIN_ERR");
        pp_insert(player_pool, generate_uuid(), player_name);
    }
    print_table(player_pool);
    return 0;
}
