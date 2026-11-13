/*
 * LeaderboardSystem common definitions and error helpers (legacy).
 * Copyright (C) 2024-2026 Alessandro Volpe
 * Originally adapted from common.h used for the Sistemi di Calcolo II
 * university exam (original header dated 04/03/2024).
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strerror in the public error-handling macros */
#include <errno.h>

#define MAX_PLAYERS     10
#define MIN_PLAYERS     5
#define MAX_ID_SIZE     40

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

char *uuid_generate(void);

#endif //ERROR_HANDLING_H
