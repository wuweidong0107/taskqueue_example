
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include "taskqueue.h"

#define TASKQUEUE_DEBUG

#if defined(TASKQUEUE_DEBUG)
#define err(str) fprintf(stderr, str)
#else
#define err(str)
#endif

/* Binary semaphore */
typedef struct bsem
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int v;
} bsem;

typedef struct thread {
    int id;
    pthread_t pthread;
    struct thread_pool* thpool;
} thread_t;

typedef struct thread_pool {
    thread_t** threads;              /* pointer to threads */
    volatile int num_threads_alive;  /* threads currently alive */
    volatile int num_thread_working; /* threads currently working */
    pthread_mutex_t thcount_lock;
} thread_pool_t;

struct task_queue {
    pthread_mutex_t rwmutex; /* used for queue r/w access */
    task_t* front;           /* pointer to front of queue */
    task_t* rear;            /* pointer to rear  of queue */
    int len;                 /* number of tasks in queue   */
    thread_pool_t* thpool;
};

static volatile int threads_keepalive;

/* Init semaphore to 1 or 0 */
static void bsem_init(bsem *bsem_p, int value)
{
    if (value < 0 || value > 1)
    {
        err("bsem_init(): Binary semaphore can take only values 1 or 0");
        exit(1);
    }
    pthread_mutex_init(&(bsem_p->mutex), NULL);
    pthread_cond_init(&(bsem_p->cond), NULL);
    bsem_p->v = value;
}

/* Reset semaphore to 0 */
static void bsem_reset(bsem *bsem_p)
{
    bsem_init(bsem_p, 0);
}

/* Post to at least one thread */
static void bsem_post(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_signal(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

/* Post to all threads */
static void bsem_post_all(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_broadcast(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

/* Wait on semaphore until semaphore has value 0 */
static void bsem_wait(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    while (bsem_p->v != 1)
    {
        pthread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
    }
    bsem_p->v = 0;
    pthread_mutex_unlock(&bsem_p->mutex);
}

static void* thread_do(thread_t* thread)
{
    char thread_name[32] = {0};
    snprintf(thread_name, 32, "thread%d", thread->id);
    prctl(PR_SET_NAME, thread_name);

    thread_pool_t* thpool = thread->thpool;

    pthread_mutex_lock(&thpool->thcount_lock);
    thpool->num_threads_alive += 1;
    pthread_mutex_unlock(&thpool->thcount_lock);

    while(1) {

    }
    return NULL;
}

static void thread_destroy(thread_t *thread)
{
    free(thread);
}

static int thread_init(thread_pool_t* thpool, thread_t** thread, int id)
{
    *thread = (thread_t *)malloc(sizeof(thread_t));
    if (*thread == NULL) {
        err("thread_init(): Could not allocate memory for thread\n");
        return -1;
    }
    (*thread)->thpool = thpool;
    (*thread)->id = id;
    pthread_create(&(*thread)->pthread, NULL, (void *(*)(void *))thread_do, (*thread));
    pthread_detach((*thread)->pthread);
    return 0;
}

static void thread_pool_destroy(thread_pool_t *thpool)
{
    int n;
    volatile int threads_total = thpool->num_threads_alive;

    for (n=0; n<threads_total; n++) {
        thread_destroy(thpool->threads[n]);
    }
}

static thread_pool_t* thread_pool_init(int num_threads)
{

    threads_keepalive = 1;

    if (num_threads < 0) {
        num_threads = 0;
    }
    thread_pool_t* thpool;
    thpool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    if (thpool == NULL) {
        err("thpool_init(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    thpool->num_threads_alive = 0;
    thpool->num_thread_working = 0;

    /* Make threads */
    thpool->threads = (thread_t**)malloc(num_threads * sizeof(thread_t*));
    if (thpool->threads == NULL) {
        err("thpool_init(): Could not allocate memory for threads\n");
        free(thpool);
        return NULL;
    }

    /* Thread init */
    int n;
    for (n=0; n<num_threads; n++) {
        thread_init(thpool, &thpool->threads[n], n);
    }

    while (thpool->num_threads_alive != num_threads) {
    }
}

task_queue_t* task_queue_init(int num_threads)
{
    task_queue_t *task_queue;
    task_queue = malloc(sizeof(task_queue_t));
    if (task_queue == NULL) {
        err("task_queue_init(): Could not allocate memory for task queue\n");
        return NULL;
    }

    task_queue->thpool = thread_pool_init(num_threads);
    if (task_queue->thpool == NULL) {
        err("task_queue_init(): Could not init thread pool\n");
        free(task_queue);
        return NULL;
    }

    return task_queue;
}

void task_queue_destroy(task_queue_t *task_queue)
{
    if (task_queue == NULL)
        return;
    
    thread_pool_destroy(task_queue->thpool);
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