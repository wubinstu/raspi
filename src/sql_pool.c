//
// Created by Einc on 2023/03/21.
//

#include "sql_pool.h"
#include "types.h"
#include "self_thread.h"
#include "log.h"

sql_pool_t * sql_pool_init (const char * host, unsigned short port,
                            const char * user, const char * pass, const char * db,
                            unsigned int conn_max, unsigned int conn_min)
{
    sql_pool_t * pool = calloc (1, sizeof (sql_pool_t));
    if (pool == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(pool) returns NULL when called sql_pool_init");
        return NULL;
    }
    pool->shutdown = false;
    pool->host = strdup (host);
    pool->port = port;
    pool->user = strdup (user);
    pool->pass = strdup (pass);
    pool->db = strdup (db);
    pool->conn_max = conn_max;
    pool->conn_min = conn_min;
    pool->conn_cur = 0;
    pool->conn_busy = 0;
    pool->sql_pool = calloc (conn_max, sizeof (sql_node_t));
    if (pool->sql_pool == NULL)
    {
        perr (true, LOG_WARNING,
              "function calloc(pool->sql_pool) returns NULL when called sql_pool_init");
        free (pool);
        return NULL;
    }
    if (create_default_mutex (& pool->lock) == NULL)
    {
        perr (true, LOG_WARNING,
              "function create_default_mutex(pool->lock) returns NULL when called sql_pool_init");
        free (pool->sql_pool);
        free (pool);
        return NULL;
    }
    if (pthread_cond_init (& pool->cond, NULL) != 0)
    {
        perr (true, LOG_WARNING,
              "function pthread_cond_init returns error when called sql_pool_init");
        free (pool->sql_pool);
        free (pool);
        return NULL;
    }
    for (int i = 0; i < conn_max; i++)
    {
        pool->sql_pool[i].index = i;
        pool->sql_pool[i].isConnected = false;
        pool->sql_pool[i].isBusy = false;
    }

    for (int i = 0; i < conn_max && pool->conn_cur < pool->conn_min; i++)
    {
        pool->sql_pool[i].connection = mysql_init (NULL);
        if ((pool->sql_pool[i].connection = mysql_real_connect
                (pool->sql_pool[i].connection, pool->host, pool->user, pool->pass,
                 pool->db, pool->port, NULL, 0)) == NULL)
        {
            perr (true, LOG_WARNING,
                  "mysql_real_connect[%d] return NULL when called sql_pool_init", i);
            pool->sql_pool[i].isConnected = false;
            continue;
        }
        pool->conn_cur++;
        pool->sql_pool[i].isConnected = true;
    }
    pool->mtid = create_default_thread (sql_pool_manage, (void *) pool);

    return pool;
}

void sql_pool_destroy (sql_pool_t * pool)
{
    if (pool == NULL || pool->shutdown)
        return;

    lock_robust_mutex (& pool->lock);
    pool->shutdown = true;
    pthread_mutex_unlock (& pool->lock);

    bool all_conn_free;
    do
    {
        sleep (3);
        all_conn_free = true;
        lock_robust_mutex (& pool->lock);
        for (int i = 0; i < pool->conn_max; i++)
            if (pool->sql_pool[i].isConnected)
            {
                if (pool->sql_pool[i].isBusy)
                    all_conn_free = false;
                else
                {
                    mysql_close (pool->sql_pool[i].connection);
                    pool->sql_pool[i].isConnected = false;
                    pool->conn_cur--;
                }
            }
        pthread_mutex_unlock (& pool->lock);
    } while (!all_conn_free);

    while (is_thread_alive (pool->mtid))
        sleep (1);

    free (pool->host);
    free (pool->user);
    free (pool->pass);
    free (pool->db);
    free (pool->sql_pool);
    pthread_mutex_destroy (& pool->lock);
    pthread_cond_destroy (& pool->cond);

    free (pool);

}

sql_node_t * sql_pool_conn_fetch (sql_pool_t * pool)
{
    if (pool == NULL || pool->shutdown)
        return NULL;

    if (pool->conn_cur == pool->conn_busy)
        return NULL;

    sql_node_t * sql_node = NULL;
    lock_robust_mutex (& pool->lock);
    for (int i = 0; i < pool->conn_max; i++)
    {
        if (pool->sql_pool[i].isConnected)
        {
            if (pool->sql_pool[i].isBusy)
                continue;
            else
            {
                pool->sql_pool[i].isBusy = true;
                sql_node = & pool->sql_pool[i];
                break;
            }
        }
    }
    if (sql_node != NULL)
        pool->conn_busy++;
    pthread_mutex_unlock (& pool->lock);
    return sql_node;
}

bool sql_pool_conn_release (sql_pool_t * pool, sql_node_t * sql_node)
{
    if (pool == NULL || sql_node == NULL)
        return false;

    lock_robust_mutex (& pool->lock);
    if (pool->sql_pool[sql_node->index].connection == sql_node->connection)
    {
        sql_node->isBusy = false;
        pool->conn_busy--;
    }
    pthread_mutex_unlock (& pool->lock);
    return true;
}

bool sql_pool_conn_add (sql_pool_t * pool, unsigned int add_num)
{
    if (pool == NULL || pool->shutdown)
        return false;

    if (pool->conn_cur >= pool->conn_max)
        return false;

    // 对 add_num 进行合法化调整, 不能加"超标"了
    if (pool->conn_cur + add_num > pool->conn_max)
        add_num = pool->conn_max - pool->conn_cur;

    int counter = 0;
    lock_robust_mutex (& pool->lock);
    for (int i = 0; i < pool->conn_max && counter < add_num; i++)
    {
        if (pool->sql_pool[i].isConnected)
            continue;
        else
        {
            pool->sql_pool[i].connection = mysql_init (NULL);
            if (mysql_real_connect (pool->sql_pool[i].connection, pool->host,
                                    pool->user, pool->pass, pool->db, pool->port,
                                    NULL, 0) == NULL)
            {
                perr (true, LOG_WARNING,
                      "mysql_real_connect[%d] return NULL when called sql_pool_init", i);
                pool->sql_pool[i].isConnected = false;
                continue;
            }
            pool->conn_cur++;
            pool->sql_pool[i].isConnected = true;
            counter++;
        }
    }
    pthread_mutex_unlock (& pool->lock);
    return true;
}

bool sql_pool_conn_del (sql_pool_t * pool, unsigned int del_num)
{
    if (pool == NULL || pool->shutdown)
        return false;

    if (pool->conn_cur <= pool->conn_min)
        return false;

    // 对 del_num 进行合法化调整, 不能减"透支"了
    unsigned int floor = pool->conn_busy > pool->conn_min ? pool->conn_busy : pool->conn_min;
    if (pool->conn_cur - del_num < floor)
        del_num = pool->conn_cur - floor;

    int counter = 0;
    lock_robust_mutex (& pool->lock);
    for (int i = 0; i < pool->conn_max && counter < del_num; i++)
        if (pool->sql_pool[i].isConnected)
        {
            mysql_close (pool->sql_pool[i].connection);
            pool->sql_pool[i].isConnected = false;
            pool->conn_cur--;
            counter++;
        }
    pthread_mutex_unlock (& pool->lock);
    return true;
}

void * sql_pool_manage (void * args)
{
    sql_pool_t * pool = (sql_pool_t *) args;

    const unsigned int SQL_CONN_ADJUST = 2;
    while (!pool->shutdown)
    {
        sleep (3);
        if (pool->conn_cur - pool->conn_busy <= 1)
            sql_pool_conn_add (pool, SQL_CONN_ADJUST);
        else if ((pool->conn_cur - pool->conn_busy) / (double) pool->conn_max > (double) 0.5)
            sql_pool_conn_del (pool, SQL_CONN_ADJUST);
        else continue;
    }
    return NULL;
}