#ifndef RWLOCK_H
#define RWLOCK_H

#include <semaphore.h>
#include <pthread.h>

// Reader–writer lock structure
typedef struct rwlock {
    sem_t lock;
    sem_t writelock;
    int readers;
} rwlock_t;

static rwlock_t g_rwlock;

// lock statistics
static int g_read_lock_acquisitions  = 0;
static int g_write_lock_acquisitions = 0;
static int g_read_lock_releases      = 0;
static int g_write_lock_releases     = 0;

static pthread_mutex_t g_lc_mutex = PTHREAD_MUTEX_INITIALIZER;

static void rwlock_init(rwlock_t *l) {
    l->readers = 0;
    sem_init(&l->lock, 0, 1);
    sem_init(&l->writelock, 0, 1);
}

static void rwlock_acquire_read(rwlock_t *l) {
    sem_wait(&l->lock);
    l->readers++;
    if (l->readers == 1) {
        sem_wait(&l->writelock);
    }
    pthread_mutex_lock(&g_lc_mutex);
    g_read_lock_acquisitions++;
    pthread_mutex_unlock(&g_lc_mutex);
    sem_post(&l->lock);
}

static void rwlock_release_read(rwlock_t *l) {
    sem_wait(&l->lock);
    l->readers--;
    if (l->readers == 0) {
        sem_post(&l->writelock);
    }
    pthread_mutex_lock(&g_lc_mutex);
    g_read_lock_releases++;
    pthread_mutex_unlock(&g_lc_mutex);
    sem_post(&l->lock);
}

static void rwlock_acquire_write(rwlock_t *l) {
    sem_wait(&l->writelock);
    pthread_mutex_lock(&g_lc_mutex);
    g_write_lock_acquisitions++;
    pthread_mutex_unlock(&g_lc_mutex);
}

static void rwlock_release_write(rwlock_t *l) {
    sem_post(&l->writelock);
    pthread_mutex_lock(&g_lc_mutex);
    g_write_lock_releases++;
    pthread_mutex_unlock(&g_lc_mutex);
}

#endif

