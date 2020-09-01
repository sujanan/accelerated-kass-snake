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

#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include <semaphore.h>

typedef void (*taskFn)(void *);

/// task
/// ====
/// @fn: task function.
/// @arg: Arguments of the @fn.
struct task {
    taskFn fn;
    void *arg; 
};

/// Max number of tasks, task queue can hold.
#define TASK_QUEUE_SIZE 16

/// taskQueue
/// =========
/// @tasks: Tasks array.
/// @head: Queue head position.
/// @tail: Queue tail position.
/// @n: Number of tasks in queue.
struct taskQueue {
    struct task tasks[TASK_QUEUE_SIZE];
    int head;
    int tail;
    int n;
};

/// Number of workers (threads) in the thread pool.
#define POOL_WORKER_SIZE 4

/// pool
/// ====
/// @fcount: Fill count semaphore.
/// @ecount: Empty count semaphore.
/// @lock: Locking semaphore for mutual execution.
/// @q: Task queue.
/// @workers: Working threads of the pool.
struct pool {
    sem_t fcount;
    sem_t ecount;
    sem_t lock;
    struct taskQueue q;
    pthread_t workers[POOL_WORKER_SIZE];
};

/// ========
/// Main API
/// ========
void poolInit(struct pool *p);
void poolFree(struct pool *p);
void poolCreateWorkers(struct pool *p);
void poolShutdown(struct pool *p);
void poolDestroyWorkers(struct pool *p);
void poolAddTask(struct pool *p, struct task t);

#endif

