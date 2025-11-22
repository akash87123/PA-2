#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include "command.h"
#include "hashtable.h"
#include "jenkins.h"
#include "logging.h"
#include "rwlock.h"
#include "process.h"

static pthread_mutex_t turn_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  turn_cv    = PTHREAD_COND_INITIALIZER;
static int current_turn = 0;

extern Command *g_commands;
extern int g_num_threads;

static void *thread_main(void *arg) {
    Command *cmd = (Command*)arg;
    int tid = cmd->thread_id;

    log_event(WAIT, tid);

    pthread_mutex_lock(&turn_mutex);
    while (tid != current_turn) {
        pthread_cond_wait(&turn_cv, &turn_mutex);
    }
    log_event(AWAKENED, tid);
    log_cmd(cmd);

    switch (cmd->type) {
        case CMD_INSERT: {
            uint32_t h = jenkins_one_at_a_time_hash(cmd->name);

            log_event(WRITE_LOCK_ACQUIRE, tid);
            rwlock_acquire_write(&g_rwlock);

            int inserted = ht_insert(h, cmd->name, cmd->salary);
            if (inserted) {
                printf("Inserted %u,%s,%u\n", h, cmd->name, cmd->salary);
            } else {
                printf("Insert failed. Entry %u is a duplicate.\n", h);
            }

            rwlock_release_write(&g_rwlock);
            log_event(WRITE_LOCK_RELEASE, tid);
            break;
        }

        case CMD_DELETE: {
            uint32_t h = jenkins_one_at_a_time_hash(cmd->name);

            log_event(WRITE_LOCK_ACQUIRE, tid);
            rwlock_acquire_write(&g_rwlock);

            hashRecord deleted;
            int ok = ht_delete(h, &deleted);

            if (ok) {
                printf("Deleted record for %u,%s,%u\n",
                       deleted.hash, deleted.name, deleted.salary);
            } else {
                printf("Entry %u not deleted. Not in database.\n", h);
            }

            rwlock_release_write(&g_rwlock);
            log_event(WRITE_LOCK_RELEASE, tid);
            break;
        }

        case CMD_UPDATE: {
            uint32_t h = jenkins_one_at_a_time_hash(cmd->name);

            log_event(WRITE_LOCK_ACQUIRE, tid);
            rwlock_acquire_write(&g_rwlock);

            hashRecord *rec = ht_find(h);
            if (rec) {
                uint32_t old_salary = rec->salary;
                printf("Updated record %u from %u,%s,%u to %u,%s,%u\n",
                       rec->hash,
                       rec->hash, rec->name, old_salary,
                       rec->hash, rec->name, cmd->salary);
                rec->salary = cmd->salary;
            } else {
                printf("Update failed. Entry %u not found.\n", h);
            }

            rwlock_release_write(&g_rwlock);
            log_event(WRITE_LOCK_RELEASE, tid);
            break;
        }

        case CMD_SEARCH: {
            uint32_t h = jenkins_one_at_a_time_hash(cmd->name);

            log_event(READ_LOCK_ACQUIRE, tid);
            rwlock_acquire_read(&g_rwlock);

            hashRecord *rec = ht_find(h);
            if (rec) {
                printf("Found: %u,%s,%u\n",
                       rec->hash, rec->name, rec->salary);
            } else {
                printf("%s not found.\n", cmd->name);
            }

            rwlock_release_read(&g_rwlock);
            log_event(READ_LOCK_RELEASE, tid);
            break;
        }

        case CMD_PRINT: {
            log_event(READ_LOCK_ACQUIRE, tid);
            rwlock_acquire_read(&g_rwlock);

            ht_print_stdout();

            rwlock_release_read(&g_rwlock);
            log_event(READ_LOCK_RELEASE, tid);
            break;
        }

        default:
            break;
    }

    current_turn++;
    pthread_cond_broadcast(&turn_cv);
    pthread_mutex_unlock(&turn_mutex);

    return NULL;
}

static void launch_threads(void) {
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * g_num_threads);
    if (!threads) {
        perror("malloc threads");
        exit(1);
    }

    for (int i = 0; i < g_num_threads; i++) {
        if (!g_commands[i].valid) continue;
        int rc = pthread_create(&threads[i], NULL, thread_main, &g_commands[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed for %d\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < g_num_threads; i++) {
        if (!g_commands[i].valid) continue;
        pthread_join(threads[i], NULL);
    }

    free(threads);
}

#endif

