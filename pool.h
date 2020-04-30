#ifndef POOL_H
#define POOL_H

#include <pthread.h>

#include "util.h"

typedef void *(*taskFn)(void *);

struct task {
    taskFn fn;
    void *arg; 
};

#define TASK_QUEUE_SIZE 16

struct taskQueue {
    struct task tasks[TASK_QUEUE_SIZE];
    int head;
    int tail;
    int n;
};

void taskQueueEnqueue(struct taskQueue *q, struct task task);
struct task taskQueueDequeue(struct taskQueue *q);

#define taskQueueNew() {.head = 0, .tail = 0, .n = 0}
#define taskQueueInit(q) ((q)->head = (q)->tail = (q)->n = 0)
#define taskQueueFree(q) taskQueueInit(q)
#define taskQueueLen(q) ((q)->n) 
#define taskQueueIsEmpty(q) (taskQueueLen(q) == 0)
#define taskQueueIsFull(q) (taskQueueLen(q) == ARRAYLEN((q)->tasks))

#define POOL_WORKER_SIZE 4

struct pool {
    pthread_cond_t cond;
    pthread_mutex_t lock;
    struct taskQueue q;
    pthread_t workers[POOL_WORKER_SIZE];
    int done;
};

void poolInit(struct pool *p);
void poolFree(struct pool *p);
void poolCreateWorkers(struct pool *p);
void poolDestroyWorkers(struct pool *p);
void poolAddTask(struct pool *p, struct task t);

#endif

