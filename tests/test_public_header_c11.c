/* A public header must compile independently under ISO C11. */
#ifndef LEADERBOARD_PUBLIC_HEADER
#error "Set LEADERBOARD_PUBLIC_HEADER in CMake"
#endif
#include LEADERBOARD_PUBLIC_HEADER

/* commons.h historically omitted string.h despite using strerror in macros;
 * force expansion to verify transitive declarations are complete. */
#if defined(ERROR_HANDLING_H)
static int validate_error_macro(int code) {
    ERROR_RETURN(code != 0, code, "test");
    return 0;
}
#endif

int leaderboard_header_smoke(void) {
#if defined(ERROR_HANDLING_H)
    return validate_error_macro(0);
#else
    return 0;
#endif
}
