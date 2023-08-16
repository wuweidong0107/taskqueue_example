
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "taskqueue.h"

#define TASKQUEUE_DEBUG

#if defined(TASKQUEUE_DEBUG)
#define err(str) fprintf(stderr, str)
#else
#define err(str)
#endif

typedef struct thread {
    int id;
    pthread_t pthread;
    struct thread_pool *thpool;
} thread_t;

typedef struct thread_pool {
    thread_t **threads;
} thread_pool_t;

struct task_queue {
    pthread_mutex_t rwmutex; /* used for queue r/w access */
    task_t *front;           /* pointer to front of queue */
    task_t *rear;            /* pointer to rear  of queue */
    int len;                 /* number of tasks in queue   */
    thread_pool_t thpool;
};

task_queue_t* task_queue_init(int num_threads)
{
    task_queue_t *task_queue;
    task_queue = malloc(sizeof(task_queue_t));
    if (task_queue == NULL) {
        err("task_queue_init(): Could not allocate memory for task queue\n");
        return NULL;
    }
    return task_queue;
}

void task_queue_destroy(task_queue_t *task_queue)
{
    if (task_queue)
        free(task_queue);
}

int task_queue_push(task_queue_t* task_queue, task_t* task)
{
    return 0;
}

void task_queue_wait(task_queue_t* task_queue)
{
}

task_t* task_init(void)
{
    return NULL;
}