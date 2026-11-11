#include <assert.h>
#include <string.h>

#include "hashmap.h"

int main(void) {
    HASHMAP(char, int) map;
    int value = 42;

    hashmap_init(&map, hashmap_hash_string, strcmp);

    assert(hashmap_put(&map, "alpha", &value) == 0);
    assert(hashmap_size(&map) == 1);
    assert(hashmap_get(&map, "alpha") == &value);
    assert(hashmap_remove(&map, "alpha") == &value);
    assert(hashmap_get(&map, "alpha") == NULL);
    assert(hashmap_size(&map) == 0);

    hashmap_cleanup(&map);
    return 0;
}
