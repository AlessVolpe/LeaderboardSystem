//
// Created by aless on 04/03/2024.
//

#ifndef OVERFLOW_H
#define OVERFLOW_H

#include "player.h"
#include "player_pool.h"

void handle_collision(const PlayerPool* table, unsigned long index, Player* item);

LinkedList** create_overflow(const PlayerPool* table);

void free_overflow(const PlayerPool* table);

#endif //OVERFLOW_H
