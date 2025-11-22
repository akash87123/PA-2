#include "command.h"
#include "hashrec.h"
#include "hashtable.h"
#include "jenkins.h"
#include "logging.h"
#include "process.h"
#include "rwlock.h"
#include "threads.h"

#define COMMAND_FILE "commands.txt"
#define LOG_FILE     "hash.log"

int main(void) {
    logging_init(LOG_FILE);
    rwlock_init(&g_rwlock);

    parse_commands(COMMAND_FILE);

    launch_threads();

    logging_finalize();

    free_commands();
    ht_free_all();

    return 0;
}

