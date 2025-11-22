#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "command.h"

static Command *g_commands = NULL;
static int g_num_threads = 0;

static void rstrip(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
                       isspace((unsigned char)s[len-1]))) {
        s[len-1] = '\0';
        len--;
    }
}

static void parse_commands(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen commands.txt");
        exit(1);
    }

    char line[256];

    if (!fgets(line, sizeof(line), fp)) {
        fprintf(stderr, "Empty commands file\n");
        exit(1);
    }
    rstrip(line);
    char *tok = strtok(line, ",");
    if (!tok || strcmp(tok, "threads") != 0) {
        fprintf(stderr, "First line must be 'threads,<num>,...'\n");
        exit(1);
    }
    char *num_str = strtok(NULL, ",");
    if (!num_str) {
        fprintf(stderr, "Invalid threads line\n");
        exit(1);
    }
    g_num_threads = atoi(num_str);
    if (g_num_threads <= 0) {
        fprintf(stderr, "Invalid thread count\n");
        exit(1);
    }

    g_commands = (Command*)calloc(g_num_threads, sizeof(Command));
    if (!g_commands) {
        perror("calloc commands");
        exit(1);
    }

    for (int i = 0; i < g_num_threads; i++) {
        g_commands[i].type      = CMD_NONE;
        g_commands[i].valid     = 0;
        g_commands[i].thread_id = i;
    }

    while (fgets(line, sizeof(line), fp)) {
        rstrip(line);
        if (line[0] == '\0') continue;

        char *saveptr = NULL;
        char *cmd_str = strtok_r(line, ",", &saveptr);
        if (!cmd_str) continue;

        if (strcmp(cmd_str, "insert") == 0) {
            char *name  = strtok_r(NULL, ",", &saveptr);
            char *sal_s = strtok_r(NULL, ",", &saveptr);
            char *tid_s = strtok_r(NULL, ",", &saveptr);
            if (!name || !sal_s || !tid_s) continue;
            int tid = atoi(tid_s);
            if (tid < 0 || tid >= g_num_threads) continue;

            Command *c = &g_commands[tid];
            c->type = CMD_INSERT;
            strncpy(c->name, name, sizeof(c->name)-1);
            c->name[sizeof(c->name)-1] = '\0';
            c->salary = (uint32_t)strtoul(sal_s, NULL, 10);
            c->thread_id = tid;
            c->valid = 1;

        } else if (strcmp(cmd_str, "delete") == 0) {
            char *name  = strtok_r(NULL, ",", &saveptr);
            char *prio  = strtok_r(NULL, ",", &saveptr);
            char *tid_s = strtok_r(NULL, ",", &saveptr);
            if (!name || !prio || !tid_s) continue;
            int tid = atoi(tid_s);
            if (tid < 0 || tid >= g_num_threads) continue;

            Command *c = &g_commands[tid];
            c->type = CMD_DELETE;
            strncpy(c->name, name, sizeof(c->name)-1);
            c->name[sizeof(c->name)-1] = '\0';
            c->salary = 0;
            c->thread_id = tid;
            c->valid = 1;

        } else if (strcmp(cmd_str, "update") == 0) {
            char *name  = strtok_r(NULL, ",", &saveptr);
            char *sal_s = strtok_r(NULL, ",", &saveptr);
            char *tid_s = strtok_r(NULL, ",", &saveptr);
            if (!name || !sal_s || !tid_s) continue;
            int tid = atoi(tid_s);
            if (tid < 0 || tid >= g_num_threads) continue;

            Command *c = &g_commands[tid];
            c->type = CMD_UPDATE;
            strncpy(c->name, name, sizeof(c->name)-1);
            c->name[sizeof(c->name)-1] = '\0';
            c->salary = (uint32_t)strtoul(sal_s, NULL, 10);
            c->thread_id = tid;
            c->valid = 1;

        } else if (strcmp(cmd_str, "search") == 0) {
            char *name  = strtok_r(NULL, ",", &saveptr);
            char *prio  = strtok_r(NULL, ",", &saveptr);
            char *tid_s = strtok_r(NULL, ",", &saveptr);
            if (!name || !prio || !tid_s) continue;
            int tid = atoi(tid_s);
            if (tid < 0 || tid >= g_num_threads) continue;

            Command *c = &g_commands[tid];
            c->type = CMD_SEARCH;
            strncpy(c->name, name, sizeof(c->name)-1);
            c->name[sizeof(c->name)-1] = '\0';
            c->salary = 0;
            c->thread_id = tid;
            c->valid = 1;

        } else if (strcmp(cmd_str, "print") == 0) {
            char *dummy1 = strtok_r(NULL, ",", &saveptr);
            char *dummy2 = strtok_r(NULL, ",", &saveptr);
            char *tid_s  = strtok_r(NULL, ",", &saveptr);
            if (!dummy1 || !dummy2 || !tid_s) continue;
            int tid = atoi(tid_s);
            if (tid < 0 || tid >= g_num_threads) continue;

            Command *c = &g_commands[tid];
            c->type = CMD_PRINT;
            c->name[0] = '\0';
            c->salary = 0;
            c->thread_id = tid;
            c->valid = 1;
        }
    }

    fclose(fp);
}

static void free_commands(void) {
    free(g_commands);
    g_commands = NULL;
    g_num_threads = 0;
}

#endif

