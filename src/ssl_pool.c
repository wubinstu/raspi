//
// Created by Einc on 2023/04/07.
//

#include "ssl_pool.h"
#include "types.h"
#include "log.h"
#include "global.h"
#include "self_thread.h"

ssl_pool_t * ssl_pool_init (unsigned int ssl_max, unsigned int ssl_min, SSL_CTX * ctx)
{
    if (ssl_max <= 0 || ssl_min <= 0 || ctx == NULL)
        return NULL;
    ssl_pool_t * pool = calloc (1, sizeof (ssl_pool_t));
    if (pool == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(pool) returns NULL when called ssl_pool_init");
        return NULL;
    }
    pool->shutdown = false;
    pool->ssl_ctx = ctx;
    pool->ssl_max = ssl_max;
    pool->ssl_min = ssl_min;
    pool->ssl_cur = 0;
    pool->ssl_busy = 0;
    pool->ssl_pool = calloc (ssl_max, sizeof (ssl_node_t));
    if (pool->ssl_pool == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(pool->ssl_pool) returns NULL when called ssl_pool_init");
        free (pool);
        return NULL;
    }
    if (create_default_mutex (& pool->lock) == NULL)
    {
        perr (true, LOG_WARNING,
              "function create_default_mutex(pool->lock) returns NULL when called ssl_pool_init");
        free (pool->ssl_pool);
        free (pool);
        return NULL;
    }
    if (pthread_cond_init (& pool->cond, NULL) != 0)
    {
        perr (true, LOG_WARNING,
              "function pthread_cond_init returns error when called ssl_pool_init");
        free (pool->ssl_pool);
        free (pool);
        return NULL;
    }
    for (int i = 0; i < ssl_max; i++)
    {
        pool->ssl_pool[i].index = i;
        pool->ssl_pool[i].onUsed = false;
        pool->ssl_pool[i].ssl_fd = NULL;
    }

    for (int i = 0; i < ssl_max && pool->ssl_cur < pool->ssl_min; i++)
    {
        if ((pool->ssl_pool[i].ssl_fd = SSL_new (pool->ssl_ctx)) != NULL)
            pool->ssl_cur++;
    }
    pool->mtid = create_default_thread (ssl_pool_manage, (void *) pool);
    return pool;
}

void ssl_pool_destroy (ssl_pool_t * pool)
{
    if (pool == NULL || pool->shutdown)
        return;

    lock_robust_mutex (& pool->lock);
    pool->shutdown = true;
    pthread_mutex_unlock (& pool->lock);

    bool all_node_free;
    do
    {
        sleep (3);
        all_node_free = true;
        lock_robust_mutex (& pool->lock);
        for (int i = 0; i < pool->ssl_max; i++)

            if (pool->ssl_pool[i].onUsed)
                all_node_free = false;
            else
            {
                SSL_free (pool->ssl_pool[i].ssl_fd);
                pool->ssl_pool[i].ssl_fd = NULL;
                pool->ssl_pool[i].onUsed = false;
                pool->ssl_cur--;
            }

        pthread_mutex_unlock (& pool->lock);
    } while (!all_node_free);

    while (is_thread_alive (pool->mtid))
        sleep (1);


    free (pool->ssl_pool);
    pthread_mutex_destroy (& pool->lock);
    pthread_cond_destroy (& pool->cond);
    free (pool);
}

ssl_node_t * ssl_pool_node_fetch (ssl_pool_t * pool)
{
    if (pool == NULL || pool->shutdown)
        return NULL;

    if (pool->ssl_cur == pool->ssl_busy)
        return NULL;

    ssl_node_t * ssl_node = NULL;
    lock_robust_mutex (& pool->lock);
    for (int i = 0; i < pool->ssl_max; i++)

        if (pool->ssl_pool[i].onUsed == false && pool->ssl_pool[i].ssl_fd != NULL)
        {
            pool->ssl_pool[i].onUsed = true;
            ssl_node = & pool->ssl_pool[i];
            break;
        }


    if (ssl_node != NULL)
        pool->ssl_busy++;
    pthread_mutex_unlock (& pool->lock);
    return ssl_node;
}

bool ssl_pool_node_release (ssl_pool_t * pool, ssl_node_t ** ssl_node)
{
    if (pool == NULL || ssl_node == NULL || * ssl_node == NULL)
        return false;

    lock_robust_mutex (& pool->lock);
    if (pool->ssl_pool[(* ssl_node)->index].ssl_fd == (* ssl_node)->ssl_fd && (* ssl_node)->onUsed)
    {
//        SSL_clear (pool->ssl_pool[(* ssl_node)->index].ssl_fd);
//        SSL_set_SSL_CTX (pool->ssl_pool[(* ssl_node)->index].ssl_fd, pool->ssl_ctx);

        SSL_free (pool->ssl_pool[(* ssl_node)->index].ssl_fd);
        pool->ssl_pool[(* ssl_node)->index].ssl_fd = SSL_new (pool->ssl_ctx);
        (* ssl_node)->onUsed = false;
        pool->ssl_busy--;
    }
    pthread_mutex_unlock (& pool->lock);
    * ssl_node = NULL;
    return true;
}

bool ssl_pool_node_add (ssl_pool_t * pool, unsigned int add_num)
{
    if (pool == NULL || pool->shutdown)
        return false;

    if (pool->ssl_cur >= pool->ssl_max)
        return false;

    // 对 add_num 进行合法化调整, 不能加"超标"了
    if (pool->ssl_cur + add_num > pool->ssl_max)
        add_num = pool->ssl_max - pool->ssl_cur;

    int counter = 0;
    lock_robust_mutex (& pool->lock);
    for (int i = 0; i < pool->ssl_max && counter < add_num; i++)

        if (pool->ssl_pool[i].ssl_fd == NULL)
        {
            if ((pool->ssl_pool[i].ssl_fd = SSL_new (pool->ssl_ctx)) != NULL)
            {
                pool->ssl_cur++;
                pool->ssl_pool[i].onUsed = false;
                counter++;
            }
        }

    pthread_mutex_unlock (& pool->lock);
    return true;
}

bool ssl_pool_node_del (ssl_pool_t * pool, unsigned int del_num)
{
    if (pool == NULL || pool->shutdown)
        return false;

    if (pool->ssl_cur <= pool->ssl_min)
        return false;

    // 对 del_num 进行合法化调整, 不能减"透支"了
    unsigned int floor = pool->ssl_busy > pool->ssl_min ? pool->ssl_busy : pool->ssl_min;
    if (pool->ssl_cur - del_num < floor)
        del_num = pool->ssl_cur - floor;

    int counter = 0;
    lock_robust_mutex (& pool->lock);
    for (int i = 0; i < pool->ssl_max && counter < del_num; i++)
        if (pool->ssl_pool[i].ssl_fd != NULL && pool->ssl_pool[i].onUsed == false)
        {
            SSL_free (pool->ssl_pool[i].ssl_fd);
            pool->ssl_pool[i].ssl_fd = NULL;
            pool->ssl_pool[i].onUsed = false;
            pool->ssl_cur--;
            counter++;
        }
    pthread_mutex_unlock (& pool->lock);
    return true;
}

void * ssl_pool_manage (void * args)
{
    ssl_pool_t * pool = (ssl_pool_t *) args;

    const unsigned int SSL_NODE_ADJUST = SSL_POOL_MANAGER_ADJUST_BY_PER;
    while (!pool->shutdown)
    {
        sleep (SSL_POOL_MANAGER_SLEEP_TIME);
        if (pool->ssl_cur - pool->ssl_busy <= 1)
            ssl_pool_node_add (pool, SSL_NODE_ADJUST);
        else if ((pool->ssl_cur - pool->ssl_busy) / (double) pool->ssl_max > (double) 0.5)
            ssl_pool_node_del (pool, SSL_NODE_ADJUST);
        else continue;
    }
    return NULL;
}