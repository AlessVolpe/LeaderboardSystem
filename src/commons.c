//
// Created by aless on 08/03/2024.
//

#include "commons.h"

char* generate_uuid() {
    const char v[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    static char buf[37] = {0};
    for (int i = 0; i < 36; i++) buf[i] = v[rand()%16];

    buf[8] = '-';
    buf[13] = '-';
    buf[18] = '-';
    buf[23] = '-';
    buf[36] = '\0';
    return buf;
}

DWORD WINAPI thread_routine(const LPVOID lpArg) {
    const int a = *(int *)lpArg;
    fprintf(stderr,"My argument is %d\n",a);
    return 0;
}