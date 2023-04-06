//
// Created by Einc on 2023/03/21.
//
#include "log.h"
#include "self_thread.h"
#include "thread_pool.h"
#include "global.h"


thread_pool_t * thread_pool_init (unsigned int thread_max_num, unsigned int thread_min_num, unsigned int queue_max_num)
{
    thread_pool_t * pool = (thread_pool_t *) calloc (1, sizeof (thread_pool_t));
    if (pool == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(pool) returns NULL when called thread_pool_init");
        return NULL;
    }
    pool->shutdown = false;
    pool->thread_max = thread_max_num;
    pool->thread_min = thread_min_num;
    pool->thread_alive = thread_min_num;
    pool->thread_busy = 0;
    pool->thread_exitcode = 0;

    pool->queue_max = queue_max_num;
    pool->queue_cur = 0;
    pool->queue_head = 0;
    pool->queue_tail = 0;

    pool->queue = (thread_task_t *) calloc (queue_max_num, sizeof (thread_task_t));
    if (pool->queue == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(pool->queue) returns NULL when called thread_pool_init");
        return NULL;
    }

    if (create_default_mutex (& pool->lock) == NULL)
    {
        perr (true, LOG_WARNING,
              "function create_default_mutex returns NULL when called thread_pool_init");
        free (pool);
        return NULL;
    }
    if (pthread_cond_init (& pool->not_empty, NULL) != 0)
    {
        perr (true, LOG_WARNING,
              "function pthread_cond_init returns error when called thread_pool_init");
        free (pool);
        return NULL;
    }
    if (pthread_cond_init (& pool->not_full, NULL) != 0)
    {
        perr (true, LOG_WARNING,
              "function pthread_cond_init returns error when called thread_pool_init");
        free (pool);
        return NULL;
    }

    pool->tids = (pthread_t *) calloc (thread_max_num, sizeof (pthread_t));
    if (pool->tids == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(tids) returns NULL when called thread_pool_init");
        free (pool);
        return NULL;
    }

    for (int i = 0; i < thread_min_num; i++)
    {
        pool->tids[i] = create_default_thread (thread_pool_process, (void *) pool);
        if (pool->tids[i] == -1)
            perr (true, LOG_WARNING,
                  "function create_default_thread[%d] returns -1 when called thread_pool_init", i);
    }
    pool->mtid = create_default_thread (thread_pool_manage, (void *) pool);
    if (pool->mtid == -1)
        perr (true, LOG_WARNING,
              "function create_default_thread[mtid] returns -1 when called thread_pool_init");

    return pool;
}

void thread_pool_destroy (thread_pool_t * pool)
{
    if (pool == NULL || pool->shutdown)
        return;

    thread_task_t * task;
    lock_robust_mutex (& pool->lock);
    pool->shutdown = true;
    pthread_mutex_unlock (& pool->lock);

    // 唤醒所有线程
    pthread_cond_broadcast (& pool->not_empty);

    unsigned int alive;
    bool is_alive_manager_thread = true;
    while (true)
    {
        sleep (5);
        lock_robust_mutex (& pool->lock);
        alive = pool->thread_alive;
        is_alive_manager_thread = is_thread_alive (pool->mtid);
        pthread_mutex_unlock (& pool->lock);

        if (alive == 0 && !is_alive_manager_thread)
            break;
    }
    free (pool->queue);
    pthread_mutex_destroy (& pool->lock);
    pthread_cond_destroy (& pool->not_empty);
    pthread_cond_destroy (& pool->not_full);
    free (pool->tids);
    free (pool);
}


void * thread_pool_process (void * args)
{
    thread_pool_t * pool = (thread_pool_t *) args;
    thread_task_t * task = NULL;

    while (true)
    {
        lock_robust_mutex (& pool->lock);


        while (pool->queue_cur == 0 && !pool->shutdown)
            pthread_cond_wait (& pool->not_empty, & pool->lock);

        if (pool->shutdown && pool->queue_cur == 0)
        {
            pool->thread_alive--;
            pthread_mutex_unlock (& pool->lock);
            break;
        }

        if (pool->thread_exitcode > 0)
        {
            pool->thread_alive--;
            pool->thread_exitcode--;
            pthread_mutex_unlock (& pool->lock);
            break;
        }

        if (pool->queue_cur != 0)  // 确保程序安全, 实际上这个条件是必然满足的
        {
            task = & pool->queue[pool->queue_head];
            pool->queue_head = (pool->queue_head + 1) % pool->queue_max;
            pool->queue_cur--;
            pthread_cond_signal (& pool->not_full);
        } else task = NULL;  // 这句分支语句理论上不会被执行

        pool->thread_busy++;
        pthread_mutex_unlock (& pool->lock);

        if (task != NULL)
        {
            task->ptime = time (NULL);
            task->process_thread = pthread_self ();
            task->state = onProcessing;

            /// TODO deal with task
            void * rtn = (* task->func) (task->args);


            /// TODO free task struct
            if (rtn == (void *) -1)task->state = inError;
            else task->state = ignorable;
//            free (task->client);
//            free (task);

        }

        lock_robust_mutex (& pool->lock);
        pool->thread_busy--;
        pthread_mutex_unlock (& pool->lock);
    }

    return NULL;
}

void * thread_pool_manage (void * args)
{
    thread_pool_t * pool = (thread_pool_t *) args;

    bool shutdown;
    int counter = 0;
    unsigned int queue_cur;
    unsigned int thread_max, thread_min, thread_alive, thread_busy;
    const int TASK_QUEUE_MAX_WAIT = 10, THREAD_NUM_ADJUST = POOL_MANAGER_ADJUST_BY_PER;
    while (!pool->shutdown)
    {
        sleep (POOL_MANAGER_SLEEP_TIME);
        lock_robust_mutex (& pool->lock);
        shutdown = pool->shutdown;
        thread_max = pool->thread_max;
        thread_min = pool->thread_min;
        thread_alive = pool->thread_alive;
        thread_busy = pool->thread_busy;
        queue_cur = pool->queue_cur;
        pthread_mutex_unlock (& pool->lock);

        if (shutdown)
            break;

        if (queue_cur >= TASK_QUEUE_MAX_WAIT && thread_alive < thread_max)
        {
            lock_robust_mutex (& pool->lock);
            for (int i = 0;
                 i < pool->thread_max &&
                 counter < THREAD_NUM_ADJUST &&
                 pool->thread_alive < pool->thread_max; i++)
            {
                if (pool->tids[i] == 0 || !is_thread_alive (pool->tids[i]))
                {
                    pool->tids[i] = create_default_thread
                            (thread_pool_process, (void *) pool);
                    counter++, pool->thread_alive++;
                }
            }

            pthread_mutex_unlock (& pool->lock);
        }

        if ((thread_busy * 2) < thread_alive && thread_alive > thread_min)
        {
            lock_robust_mutex (& pool->lock);
            pool->thread_exitcode = THREAD_NUM_ADJUST;
            pthread_mutex_unlock (& pool->lock);

            /**
             * 短时间内调用 pthread_cond_signal 可能是无效的?
             * 这个问题的答案是否定的
             * pthread_cond_signal 让一个线程从pthread_cond_wait苏醒
             * 该线程只需要竞争互斥锁即可, 不会再次检查条件变量
             * 所以短时间内多次调用pthread_cond_signal是没有问题的
             * 前提是有足够的线程阻塞再pthread_cond_wait, 否则多余信号会被忽略 */
            for (int i = 0; i < THREAD_NUM_ADJUST; i++)
                pthread_cond_signal (& pool->not_empty);  // 让工作线程尽早退出

        }
    }

    return NULL;
}

bool thread_pool_add_task (thread_pool_t * pool, thread_task_t * task)
{
    if (pool == NULL || task == NULL || pool->shutdown)
        return false;

    lock_robust_mutex (& pool->lock);
    while (pool->queue_cur == pool->queue_max)
        pthread_cond_wait (& pool->not_full, & pool->lock);


    if (pool->queue_cur < pool->queue_max)  // 理论上本条件必然满足
    {
        task->atime = time (NULL);
        if (task->state != newlyBuild)
            task->state = newlyBuild;

        /**
         * 这里我们有queue_max,queue_cur表示队列状态
         * 所以不需要"head == tail 队空", "(head + 1) % max == tail 队满"这样判断*/
        pool->queue[pool->queue_tail] = * task;
        pool->queue_tail = (pool->queue_tail + 1) % pool->queue_max;
        pool->queue_cur++;
        pthread_cond_signal (& pool->not_empty);
    }

    pthread_mutex_unlock (& pool->lock);
    return true;
}