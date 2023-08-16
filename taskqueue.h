#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

typedef struct task task_t;
typedef void (*task_handler_t)(task_t* task);
typedef struct task_queue task_queue_t;

struct task {
    struct task *prev;
    task_handler_t handler;
    task_handler_t result_cb;
    void *user_data;
};

/* API */
task_queue_t* task_queue_init(int num_threads);
void task_queue_destroy(task_queue_t *task_queue);
int task_queue_push(task_queue_t* task_queue, task_t* task);
void task_queue_wait(task_queue_t* task_queue);
task_t* task_init(void);
#endif