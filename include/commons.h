//
// Created by aless on 04/03/2024.
// Header copied from the common.h used in my Sistemi di Calcolo II exam in university
//

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_PLAYERS 38
#define MIN_PLAYERS 5

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {               \
    if (cond) {                                                     \
        fprintf(stderr, "%s: %s\n", msg, strerror(errCode));        \
        exit(EXIT_FAILURE);                                         \
    }                                                               \
} while(0)

#define ERROR_RETURN(cond, errCode, msg) do {                       \
    if (cond) {                                                     \
        fprintf(stderr, "%s: %s\n", msg, strerror(errCode));        \
        return errCode;                                             \
    }                                                               \
} while(0)

#define ERROR_HELPER(ret, msg) GENERIC_ERROR_HELPER((ret < 0), errno, msg)

char* generate_uuid();

DWORD WINAPI thread_routine(LPVOID lpArg);

#endif //ERROR_HANDLING_H
