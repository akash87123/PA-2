#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashrec.h"

static hashRecord *g_head = NULL;

// Insert node sorted by hash; return 1 if inserted, 0 if duplicate
static int ht_insert(uint32_t hash, const char *name, uint32_t salary) {
    hashRecord *prev = NULL;
    hashRecord *cur  = g_head;

    while (cur && cur->hash < hash) {
        prev = cur;
        cur  = cur->next;
    }

    if (cur && cur->hash == hash) {
        return 0; // duplicate
    }

    hashRecord *node = (hashRecord*)malloc(sizeof(hashRecord));
    if (!node) {
        perror("malloc");
        exit(1);
    }
    node->hash = hash;
    strncpy(node->name, name, MAX_NAME_LEN);
    node->name[MAX_NAME_LEN] = '\0';
    node->salary = salary;
    node->next   = cur;

    if (prev) prev->next = node;
    else      g_head = node;

    return 1;
}

// Find by hash
static hashRecord* ht_find(uint32_t hash) {
    hashRecord *cur = g_head;
    while (cur && cur->hash < hash) {
        cur = cur->next;
    }
    if (cur && cur->hash == hash) return cur;
    return NULL;
}

// Delete by hash; return 1 if deleted, 0 if not found
static int ht_delete(uint32_t hash, hashRecord *outdeleted) {
    hashRecord *prev = NULL;
    hashRecord *cur  = g_head;

    while (cur && cur->hash < hash) {
        prev = cur;
        cur  = cur->next;
    }
    if (!cur || cur->hash != hash) {
        return 0;
    }

    if (prev) prev->next = cur->next;
    else      g_head = cur->next;

    if (outdeleted) {
        outdeleted->hash   = cur->hash;
        strcpy(outdeleted->name, cur->name);
        outdeleted->salary = cur->salary;
        outdeleted->next   = NULL;
    }
    free(cur);
    return 1;
}

// Print entire table to stdout
static void ht_print_stdout(void) {
    printf("Current Database:\n");
    hashRecord *cur = g_head;
    while (cur) {
        printf("%u,%s,%u\n", cur->hash, cur->name, cur->salary);
        cur = cur->next;
    }
}

// Free all records
static void ht_free_all(void) {
    hashRecord *cur = g_head;
    while (cur) {
        hashRecord *next = cur->next;
        free(cur);
        cur = next;
    }
    g_head = NULL;
}

#endif

