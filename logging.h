#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "hashtable.h"
#include "command.h"
#include "jenkins.h"
#include "rwlock.h"

enum state {
    WAIT,
    AWAKENED,
    READ_LOCK_ACQUIRE,
    READ_LOCK_RELEASE,
    WRITE_LOCK_ACQUIRE,
    WRITE_LOCK_RELEASE
};

static FILE *log_fp = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static long long current_timestamp(void) {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (long long)te.tv_sec * 1000000LL + te.tv_usec;
}

static void logging_init(const char *filename) {
    log_fp = fopen(filename, "w");
}

static void log_line(int tid, const char *msg) {
    if (!log_fp) return;
    long long ts = current_timestamp();
    pthread_mutex_lock(&log_mutex);
    fprintf(log_fp, "%lld: THREAD %d %s\n", ts, tid, msg);
    fflush(log_fp);
    pthread_mutex_unlock(&log_mutex);
}

// Log command description 
static void log_cmd(Command *cmd) {
    if (!log_fp) return;

    char buf[256];
    uint32_t h = jenkins_one_at_a_time_hash(cmd->name);

    switch (cmd->type) {
        case CMD_INSERT:
            snprintf(buf, sizeof(buf),
                     "INSERT,%u,%s,%u", h, cmd->name, cmd->salary);
            break;
        case CMD_DELETE:
            snprintf(buf, sizeof(buf),
                     "DELETE,%u,%s", h, cmd->name);
            break;
        case CMD_UPDATE:
            snprintf(buf, sizeof(buf),
                     "UPDATE,%u,%s,%u", h, cmd->name, cmd->salary);
            break;
        case CMD_SEARCH:
            snprintf(buf, sizeof(buf),
                     "SEARCH,%u,%s", h, cmd->name);
            break;
        case CMD_PRINT:
            snprintf(buf, sizeof(buf), "PRINT");
            break;
        default:
            snprintf(buf, sizeof(buf), "INVALID");
            break;
    }

    log_line(cmd->thread_id, buf);
}

static void log_event(int state, int tid) {
    switch (state) {
        case WAIT:
            log_line(tid, "WAITING FOR MY TURN");
            break;
        case AWAKENED:
            log_line(tid, "AWAKENED FOR WORK");
            break;
        case READ_LOCK_ACQUIRE:
            log_line(tid, "READ LOCK ACQUIRED");
            break;
        case READ_LOCK_RELEASE:
            log_line(tid, "READ LOCK RELEASED");
            break;
        case WRITE_LOCK_ACQUIRE:
            log_line(tid, "WRITE LOCK ACQUIRED");
            break;
        case WRITE_LOCK_RELEASE:
            log_line(tid, "WRITE LOCK RELEASED");
            break;
        default:
            break;
    }
}

// Final summary written to hash.log
static void logging_finalize(void) {
    if (!log_fp) return;

    int total_acq = g_read_lock_acquisitions + g_write_lock_acquisitions;
    int total_rel = g_read_lock_releases + g_write_lock_releases;

    fprintf(log_fp, "Number of lock acquisitions: %d\n", total_acq);
    fprintf(log_fp, "Number of lock releases: %d\n", total_rel);
    fprintf(log_fp, "Final Table:\n");

    hashRecord *cur = g_head;
    while (cur) {
        fprintf(log_fp, "%u,%s,%u\n", cur->hash, cur->name, cur->salary);
        cur = cur->next;
    }

    fclose(log_fp);
    log_fp = NULL;
}

#endif 

