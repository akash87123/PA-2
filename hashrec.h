#ifndef HASHREC_H
#define HASHREC_H

#include <stdint.h>

#define MAX_NAME_LEN 50

typedef struct hash_struct {
    uint32_t hash;
    char     name[MAX_NAME_LEN + 1];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

#endif

