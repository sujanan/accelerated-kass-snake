/// MIT License
/// 
/// Copyright (c) 2020 Sujanan Bhathiya
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "pool.h"

#define ARRAYLEN(a) (sizeof(a)/sizeof(a[0]))

/// taskQueue implementation
/// ========================
#define taskQueueInit(q) ((q)->head = (q)->tail = (q)->n = 0)
#define taskQueueFree(q) taskQueueInit(q)
#define taskQueueIsEmpty(q) ((q)->n == 0)

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

/// Semaphore manipulation
/// ====================== 
#define UP(s) (sem_post(s))
#define DOWN(s) (sem_wait(s))
#define FILLCOUNT_UP(p) UP(&p->fcount)
#define FILLCOUNT_DOWN(p) DOWN(&p->fcount)
#define EMPTYCOUNT_UP(p) UP(&p->ecount)
#define EMPTYCOUNT_DOWN(p) DOWN(&p->ecount)
#define LOCK(p) DOWN(&p->lock)
#define UNLOCK(p) UP(&p->lock)

/// worker
/// ======
/// 1. Decrement the fill count.   
/// 2. Enter the critical section.
/// 3. Check whether the task queue is empty.
///    If it is empty that means the pool is shutdown
///    and this is only a wake up call. So, worker should
///    stop working.
/// 4. Get a task from the queue.
/// 5. Leave the critical section.   
/// 6. Increment the empty count.
///
static void *worker(void *arg) {
    struct pool *p = (struct pool *)arg;

    while (1) {
        FILLCOUNT_DOWN(p);
        LOCK(p);
        if (taskQueueIsEmpty(&p->q))
            break;
        struct task t = taskQueueDequeue(&p->q);
        UNLOCK(p);
        EMPTYCOUNT_UP(p);
        (*t.fn)(t.arg);
    }
    UNLOCK(p);
}

/// poolInit
/// ========
/// Init queue and all the semaphores.
///
void poolInit(struct pool *p) {
    taskQueueInit(&p->q);
    sem_init(&p->fcount, 0, 0);
    sem_init(&p->ecount, 0, TASK_QUEUE_SIZE);
    sem_init(&p->lock, 0, 1);
}

/// poolFree
/// ========
/// Destroy queue and all the semaphores.
///
void poolFree(struct pool *p) {
    sem_destroy(&p->fcount);
    sem_destroy(&p->ecount);
    sem_destroy(&p->lock);
    taskQueueFree(&p->q);
}

/// poolCreateWorkers
/// =================
/// Create all the workers in the pool.
/// Workers will start working immediately.
///
void poolCreateWorkers(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        pthread_create(&p->workers[i], NULL, worker, (void *)p);
}

/// poolShutdown
/// ============
/// Let workers know we have to shutdown. 
/// This is not a immediate shutdown. Before the exit,
/// Workers will finish all the work they are currently doing.
/// Here poolShutdown will trigger extra ARRAYLEN(p->workers)
/// (number of threads) increments to fill count to wake up the
/// sleeping workers.
///
void poolShutdown(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        FILLCOUNT_UP(p);
}

/// poolDestroyWorkers
/// ==================
/// Wait until all workers have finished their tasks
/// to join them. Wokers will stop working when the pool
/// is shutdown.
///
void poolDestroyWorkers(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        pthread_join(p->workers[i], NULL);
}

/// poolAddTask
/// ===========
/// Add a new task to the queue.
/// 1. Decrement the empty count.
/// 2. Enter the critical section.
/// 3. Add the new task.
/// 4. Leave the critical section.
/// 5. increment the fill count. 
///
/// If the empty count is 0 that means the task queue is full.
/// poolAddTask will wait at this point until any of the workers
/// pick a task from the queue.
///
void poolAddTask(struct pool *p, struct task t) {
    EMPTYCOUNT_DOWN(p);
    LOCK(p);
    taskQueueEnqueue(&p->q, t);
    UNLOCK(p);
    FILLCOUNT_UP(p);
}

