#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static char *duplicate_string(const char *key) {
    size_t len = strlen(key) + 1;
    char *copy = malloc(len);
    if (copy != NULL) memcpy(copy, key, len);
    return copy;
}

static void release_string(char *key) { free(key); }

int main(void) {
    HASHMAP(char, int) map;
    int values[] = {10, 20, 30, 40};
    hashmap_init(&map, hashmap_hash_string, strcmp);
    CHECK(hashmap_set_key_alloc_funcs(&map, duplicate_string, release_string) == 0);
    CHECK(hashmap_put(&map, "one", &values[0]) == 0);
    CHECK(hashmap_put(&map, "two", &values[1]) == 0);
    CHECK(hashmap_put(&map, "three", &values[2]) == 0);
    CHECK(hashmap_put(&map, "four", &values[3]) == 0);
    CHECK(hashmap_set_key_alloc_funcs(&map, NULL, NULL) == -EBUSY);

    HASHMAP_ITER(map) it = hashmap_iter(&map);
    CHECK(hashmap_iter_valid(&it));
    CHECK(hashmap_iter_set_data(&it, &values[3]) == 0);
    CHECK(hashmap_iter_get_data(&it) == &values[3]);
    CHECK(hashmap_iter_set_data(&it, NULL) == -EINVAL);

    const char *key = NULL;
    void *temporary = NULL;
    size_t count = 0;
    hashmap_foreach_key_safe(key, &map, temporary) {
        CHECK(key != NULL);
        CHECK(hashmap_remove(&map, key) != NULL);
        ++count;
    }
    CHECK(count == 4);
    CHECK(hashmap_size(&map) == 0);
    hashmap_cleanup(&map);
    puts("hashmap typed macros: passed");
    return 0;
}
