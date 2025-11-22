#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

typedef enum {
    CMD_INSERT,
    CMD_DELETE,
    CMD_UPDATE,
    CMD_SEARCH,
    CMD_PRINT,
    CMD_NONE
} CommandType;

typedef struct {
    CommandType type;
    char name[64];
    uint32_t salary;
    int thread_id;  
    int valid;
} Command;

#endif 

