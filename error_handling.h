//
// Created by aless on 04/03/2024.
//

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H
#include <stdio.h>
#include <stdlib.h>

inline void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

#endif //ERROR_HANDLING_H
