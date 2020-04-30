#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "pool.h"

void taskQueueEnqueue(struct taskQueue *q, struct task task) {
    q->tasks[q->tail] = task;
    q->tail = (q->tail + 1) % ARRAYLEN((q)->tasks);
    q->n++;
}

struct task taskQueueDequeue(struct taskQueue *q) {
    struct task task = q->tasks[q->head];
    q->head = (q->head + 1) % ARRAYLEN((q)->tasks);
    q->n--;
    return task; 
}

#define LOCK(p) (pthread_mutex_lock(&(p)->lock))
#define UNLOCK(p) (pthread_mutex_unlock(&(p)->lock))
#define WAIT(p) (pthread_cond_wait(&(p)->cond, &(p)->lock))
#define SIGNAL(p) (pthread_cond_signal(&(p)->cond))
#define BROADCAST(p) (pthread_cond_broadcast(&(p)->cond))

#define poolHasShutDown(p) ((p)->done)

static void *worker(void *arg) {
    struct pool *p = (struct pool *) arg;

    while (1) {
        LOCK(p);
        while (taskQueueIsEmpty(&p->q) && !poolHasShutDown(p))
            WAIT(p);
        if (poolHasShutDown(p) && taskQueueIsEmpty(&p->q))
            break;
        struct task t = taskQueueDequeue(&p->q);
        UNLOCK(p);
        (*t.fn)(t.arg);
    }
    UNLOCK(p);
}

void poolInit(struct pool *p) {
    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->cond, NULL);
    taskQueueInit(&p->q);
    p->done = 0;
}

void poolFree(struct pool *p) {
    pthread_mutex_destroy(&p->lock);
    pthread_cond_destroy(&p->cond);
    taskQueueFree(&p->q);
    p->done = 1;
}

void poolCreateWorkers(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        pthread_create(&p->workers[i], NULL, worker, (void *) p);
}

void poolDestroyWorkers(struct pool *p) {
    LOCK(p);
    p->done = 1;
    BROADCAST(p);
    UNLOCK(p);
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        pthread_join(p->workers[i], NULL);
}

void poolAddTask(struct pool *p, struct task t) {
    LOCK(p);
    while (taskQueueIsFull(&p->q)) {
        UNLOCK(p);
        usleep(1);
        LOCK(p);
    }
    taskQueueEnqueue(&p->q, t);
    SIGNAL(p);
    UNLOCK(p);
}

