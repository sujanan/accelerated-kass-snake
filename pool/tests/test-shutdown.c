#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "../pool.h"

#define TASK_COUNT 10000

int left = TASK_COUNT;

pthread_mutex_t lock;

void fakeTask(void *arg) {
    usleep(100);
    pthread_mutex_lock(&lock);
    left -= 1;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv) {
    pthread_mutex_init(&lock, NULL);

    struct pool p; 
    poolInit(&p);
    poolCreateWorkers(&p);
    for (int i = 0; i < TASK_COUNT; i++) {
        struct task t = {.fn = fakeTask, .arg = NULL};
        poolAddTask(&p, t);
    }
    poolShutdown(&p);
    poolDestroyWorkers(&p);
    poolFree(&p);

    assert(left == 0);

    pthread_mutex_destroy(&lock);
    return 0;
}
