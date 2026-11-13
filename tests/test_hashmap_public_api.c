/*
 * LeaderboardSystem opaque hashmap API regression (milestone 3.0.6).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap_base.h"
#include "hashmap.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); exit(1); } } while (0)
static size_t hash_key(const void *key) { return hashmap_hash_string((const char*)key); }
static int cmp_key(const void *a,const void *b) { return strcmp((const char*)a,(const char*)b); }
static unsigned released;
static void *copy_key(const void *key) {
    const size_t n = strlen((const char*)key)+1U;
    char *copy=malloc(n);
    if(copy) memcpy(copy,key,n);
    return copy;
}
static void release_key(void *key) { ++released;free(key); }

static void opaque_base(void) {
    hashmap_base_t *map=hashmap_base_new(hash_key,cmp_key);
    CHECK(map != NULL);
    CHECK(hashmap_base_size(map)==0U);
    CHECK(hashmap_base_capacity(map)==0U);
    CHECK(hashmap_base_initial_capacity(map)>=32U);
    CHECK(hashmap_base_set_key_alloc_funcs(map,copy_key,release_key)==0);
    int a=123,b=456;
    char id[]="alpha";
    CHECK(hashmap_base_put(map,id,&a)==0);
    memcpy(id,"other",6);
    CHECK(hashmap_base_get(map,"alpha")==&a);
    CHECK(hashmap_base_put(map,"alpha",&b)==-EEXIST);
    CHECK(hashmap_base_size(map)==1U);
    CHECK(hashmap_base_get(map,"other")==NULL);
    hashmap_entry_t *it=hashmap_base_iter(map,NULL);
    CHECK(it != NULL && hashmap_base_iter_valid(map,it));
    CHECK(strcmp((const char*)hashmap_base_iter_get_key(it),"alpha")==0);
    CHECK(hashmap_base_iter_get_data(it)==&a);
    CHECK(hashmap_base_iter_set_data(it,&b)==0);
    CHECK(hashmap_base_get(map,"alpha")==&b);
    CHECK(hashmap_base_remove(map,"alpha")==&b);
    CHECK(released==1U);
    hashmap_base_destroy(map);
    hashmap_base_destroy(NULL);
    CHECK(hashmap_base_size(NULL)==0U);
    CHECK(hashmap_base_capacity(NULL)==0U);
}

static void typed_wrapper(void) {
    HASHMAP(char,int) map;
    CHECK(hashmap_init(&map,hashmap_hash_string,strcmp)==0);
    CHECK(hashmap_size(&map)==0U);
    int a=10,b=20;
    CHECK(hashmap_put(&map,"alpha",&a)==0);
    CHECK(hashmap_put(&map,"beta",&b)==0);
    CHECK(hashmap_size(&map)==2U);
    CHECK(hashmap_get(&map,"alpha")==&a);
    CHECK(hashmap_remove(&map,"alpha")==&a);
    CHECK(hashmap_get(&map,"alpha")==NULL);
    hashmap_cleanup(&map);
    CHECK(map.map_base==NULL);
    CHECK(hashmap_size(&map)==0U);
}
int main(void){opaque_base();typed_wrapper();return 0;}
