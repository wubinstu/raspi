//
// Created by Einc on 2023/03/21.
//

#include "run_server.h"
#include "global.h"
#include "types.h"
#include "socket_fd.h"
#include "rssl.h"
#include "load_clean.h"
#include "log.h"
#include "thread_pool.h"
#include "sql_pool.h"
#include "self_sql.h"
#include "self_thread.h"
#include "filed.h"
#include "web_http.h"

int readClient (client_info_t clientInfo, void * buf, int size)
{
    int read_size;
    if (clientInfo.sslEnable)
        read_size = SSL_read (clientInfo.ssl_fd, buf, size);
    else read_size = (int) read (clientInfo.fd, buf, size);
    return read_size;
}

int writeClient (client_info_t clientInfo, void * buf, int size)
{
    int write_size;
    if (clientInfo.sslEnable)
        write_size = SSL_write (clientInfo.ssl_fd, buf, size);
    else write_size = (int) write (clientInfo.fd, buf, size);
    return write_size;
}

bool initServerSocket ()
{
    memset (& server_accept_raspi, 0, sizeof (server_accept_raspi));
    memset (& server_accept_http, 0, sizeof (server_accept_http));

    server_accept_raspi.addr.sin_addr.s_addr = config_server.bindIp;
    server_accept_raspi.addr.sin_port = htons (config_server.bindPort);
    server_accept_raspi.addr.sin_family = AF_INET;
    server_accept_raspi.addr_len = sizeof (server_accept_raspi.addr);
    server_accept_raspi.fd = createServSock (server_accept_raspi);

    server_accept_http.addr.sin_addr.s_addr = config_server.bindIp;
    server_accept_http.addr.sin_port = htons (config_server.httpPort);
    server_accept_http.addr.sin_family = AF_INET;
    server_accept_http.addr_len = sizeof (server_accept_http.addr);
    server_accept_http.fd = createServSock (server_accept_http);

    if (config_server.modeSSL)
        server_accept_raspi.sslEnable = true,
                server_accept_http.sslEnable = true;

    if (server_accept_raspi.fd == -1 || server_accept_http.fd == -1)
        return false;
    sockReuseAddr (server_accept_raspi.fd);
    sockReuseAddr (server_accept_http.fd);
    return true;
}

void loadSSLServ ()
{
    if (server_accept_raspi.sslEnable)
    {
        server_accept_raspi.ssl_ctx = initSSL (true);
        if (server_accept_raspi.ssl_ctx == NULL) exitCleanupServ ();

        int rtn_flag = loadCA (server_accept_raspi.ssl_ctx, config_server.caFile);
        if (!rtn_flag) exitCleanupServ ();

        rtn_flag = loadCert (server_accept_raspi.ssl_ctx, config_server.servCert);
        if (!rtn_flag) exitCleanupServ ();

        rtn_flag = loadKey (server_accept_raspi.ssl_ctx, config_server.servKey);
        if (!rtn_flag) exitCleanupServ ();

        rtn_flag = checkKey (server_accept_raspi.ssl_ctx);
        if (!rtn_flag) exitCleanupServ ();

        server_accept_raspi.ssl_fd = SSL_fd (server_accept_raspi.ssl_ctx, server_accept_raspi.fd);
        if (server_accept_raspi.ssl_fd == NULL) exitCleanupServ ();
        setVerifyPeer (server_accept_raspi.ssl_ctx, false);
    }
    if (server_accept_http.sslEnable)
    {
        server_accept_http.ssl_ctx = initSSL (true);
        if (server_accept_http.ssl_ctx == NULL) exitCleanupServ ();

        int rtn_flag = loadCA (server_accept_http.ssl_ctx, config_server.caFile);
        if (!rtn_flag) exitCleanupServ ();

        rtn_flag = loadCert (server_accept_http.ssl_ctx, config_server.servCert);
        if (!rtn_flag) exitCleanupServ ();

        rtn_flag = loadKey (server_accept_http.ssl_ctx, config_server.servKey);
        if (!rtn_flag) exitCleanupServ ();

        rtn_flag = checkKey (server_accept_http.ssl_ctx);
        if (!rtn_flag) exitCleanupServ ();

        server_accept_http.ssl_fd = SSL_fd (server_accept_http.ssl_ctx, server_accept_http.fd);
        if (server_accept_http.ssl_fd == NULL) exitCleanupServ ();
        setVerifyPeer (server_accept_http.ssl_ctx, false);
    }
}

void negotiateUUID (client_info_t * clientInfo)
{
    uuid_t uuid_client;
    uuid_t uuid_zero = {0};
    char uuid_client_string[40];
    memset (uuid_client, 0, sizeof (uuid_t));
    memset (uuid_client_string, 0, sizeof (uuid_client_string));

    readClient (* clientInfo, uuid_client, sizeof (uuid_t));
//    if (uuid_compare (uuid_client, (uuid_t) {UUID_NONE}) == 1)
    if (memcmp (uuid_client, UUID_NONE, strlen (UUID_NONE)) == 0 ||
        memcmp (uuid_client, uuid_zero, sizeof (uuid_t)) == 0)
        uuid_generate (uuid_client),
                writeClient (* clientInfo, uuid_client, sizeof (uuid_t));
    memcpy (clientInfo->id, uuid_client, sizeof (uuid_t));

    uuid_unparse (clientInfo->id, uuid_client_string);

    perr (true, LOG_INFO, "UUID: %s for client[%d] form %s:%d",
          (char *) uuid_client_string, clientInfo->fd,
          inet_ntoa (clientInfo->addr.sin_addr),
          ntohs (clientInfo->addr.sin_port));

}

void * raspiEventPoll (void * args)
{
    if (args == NULL)
        return NULL;
    server_info_t serverInfo = * (server_info_t *) args;
    int ev_num;
    thread_task_t task;
//    const int EPOLL_SIZE = 50;
    socklen_t client_addr_size;
    hash_node_client_t * client_hash_node;
    struct sockaddr_in client_addr;
    struct epoll_event /* ev[EPOLL_SIZE], */ client_ev;

    while (true)
    {
        ev_num = epoll_wait (serverInfo.epfd, event_server_raspi, SERVER_EPOLL_SIZE, 5000);
        if (ev_num == -1)
            break;

        for (int i = 0; i < ev_num; i++)
        {
            // 手动调用一次 read 函数得到 0 或者 -1 的返回值才会发生此事件(服务器和客户端调用close皆可)
            // 这里用于处理非预期的连接断开,客户端软件遭遇SIGKILL信号,客户端遭遇遭遇突然断电等
            // 及时清理掉无效连接
            if (event_server_raspi[i].events & EPOLLRDHUP ||
                event_server_raspi[i].events & EPOLLERR ||
                event_server_raspi[i].events & EPOLLHUP)
            {
                client_hash_node = hash_table_client_get (hash_table_raspi, event_server_raspi[i].data.fd);
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_DEL, event_server_raspi[i].data.fd, NULL);

                if (client_hash_node != NULL)
                {
                    lock_robust_mutex (& client_hash_node->clientInfo.onProcess);
                    perr (true, LOG_INFO, "Raspi Client Disconnect! Delete: client[%d] form %s:%d",
                          client_hash_node->clientInfo.fd,
                          inet_ntoa (client_hash_node->clientInfo.addr.sin_addr),
                          ntohs (client_hash_node->clientInfo.addr.sin_port));
                    if (client_hash_node->clientInfo.sql != NULL)
                        sql_pool_conn_release (sql_pool_accept_raspi, & client_hash_node->clientInfo.sql);
                    if (client_hash_node->clientInfo.sslEnable && client_hash_node->clientInfo.ssl_fd != NULL)
                    {
                        SSL_shutdown (client_hash_node->clientInfo.ssl_fd);
                        SSL_free (client_hash_node->clientInfo.ssl_fd);
                    }
                    close (client_hash_node->clientInfo.fd);
                    pthread_mutex_destroy (& client_hash_node->clientInfo.onProcess);
                    hash_table_info_delete (hash_table_info_raspi_http, client_hash_node->clientInfo.fd);
                    hash_table_client_del (hash_table_raspi, client_hash_node->clientInfo.fd);
                }
                continue;
            }
            // 服务器套接字事件, 一般是有新连接
            if (event_server_raspi[i].data.fd == serverInfo.fd)
            {
                client_addr_size = sizeof (client_addr);  // 实际上是个定值
                client_ev.data.fd = accept (serverInfo.fd, (struct sockaddr *) & client_addr, & client_addr_size);
                if (client_ev.data.fd == -1)
                {
                    perr (true, LOG_WARNING,
                          "function accept returns -1 when called raspiEventPoll");
                    continue;
                }
                // 设置客户端套接字: 非阻塞
                setSockFlag (client_ev.data.fd, O_NONBLOCK, true);
                // 设置客户端套接字: KeepAlive
                sockKeepAlive (client_ev.data.fd);
                // 设置客户端套接字: 关注的事件
                client_ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_ADD, client_ev.data.fd, & client_ev);
                perr (true, LOG_INFO,
                      "raspiEventPoll: accept a client[%d] form %s:%d",
                      client_ev.data.fd, inet_ntoa (client_addr.sin_addr), ntohs (client_addr.sin_port));
                client_hash_node = calloc (1, sizeof (hash_node_client_t));


                client_hash_node->next = NULL;
                client_hash_node->clientInfo.ssl_fd = NULL;
                client_hash_node->clientInfo.sslEnable = false;
                client_hash_node->hash_node_key = client_ev.data.fd;
                client_hash_node->clientInfo.fd = client_ev.data.fd;
                client_hash_node->clientInfo.addr = client_addr;
                client_hash_node->clientInfo.addr_len = (int) client_addr_size;
                client_hash_node->clientInfo.sql = NULL;  // sql_pool_conn_fetch (sql_pool_accept_raspi);
                create_default_mutex (& client_hash_node->clientInfo.onProcess);
                memset (client_hash_node->clientInfo.id, 0, sizeof (uuid_t));
                hash_table_client_add (hash_table_raspi, (int) client_hash_node->hash_node_key, client_hash_node);

            }
                // 其他套接字事件, 一般是客户端套接字
            else
            {
                client_hash_node = hash_table_client_get (hash_table_raspi, event_server_raspi[i].data.fd);
                memset (& task, 0, sizeof (task));
                task.state = newlyBuild;
                task.ctime = time (NULL);
//                if (task.client->sql == NULL)
//                    task.client->sql = sql_pool_conn_fetch (sql_pool_accept_raspi);
                task.args = (void *) & client_hash_node->clientInfo.fd;
                task.func = processRaspiClient;
                thread_pool_add_task (thread_pool_accept_raspi, & task);
//                task.func (task.args);
            }
        }
    }
    return (void *) -1;
}

// 线程[不]安全函数常见的特征有:
// 使用静态数据结构: 如果函数使用静态变量,全局变量或者文件作为数据源,则在多线程环境下,可能会发生数据竞争
// 修改参数: 如果函数会修改传入的指针或引用类型参数,那么在多线程环境下,如果多个线程同时调用该函数并传入相同的参数,可能会导致数据竞争
// 未加锁: 如果函数在内部没有使用锁对共享资源进行保护,那么在多线程环境下,可能会发生数据竞争

// 线程安全函数通常具有以下特征:
// 使用栈上数据结构: 线程安全函数通常使用栈上的数据结构或者动态内存分配来保证多个线程不会产生数据竞争
// 不会修改传入参数: 线程安全函数通常不会修改传入的参数,因为这可能导致不可预测的行为
// 使用锁: 线程安全函数通常使用锁来保护共享资源,以避免数据竞争,锁可以是互斥量,读写锁或信号量等

void * processRaspiClient (void * args)
{
    int client_fd = * (int *) args;
    int read_len;
    uuid_t id_zero = {0};
    hash_node_client_t * task_client = hash_table_client_get (hash_table_raspi, client_fd);
    if (pthread_mutex_trylock (& task_client->clientInfo.onProcess) != 0)
        return NULL;

    // SSL 握手: "如果服务器套接字为SSL模式,但是客户套接字仍然为非SSL模式"
    if (server_accept_raspi.sslEnable && !task_client->clientInfo.sslEnable)
    {
        if (task_client->clientInfo.ssl_fd == NULL)
            task_client->clientInfo.ssl_fd = SSL_new (server_accept_raspi.ssl_ctx);

        SSL_set_fd (task_client->clientInfo.ssl_fd, task_client->clientInfo.fd);

        int ssl_accept_rtn = SSL_accept (task_client->clientInfo.ssl_fd);
        int ssl_rtn = SSL_get_error (task_client->clientInfo.ssl_fd, ssl_accept_rtn);
        if (ssl_rtn == SSL_ERROR_WANT_READ || ssl_rtn == SSL_ERROR_WANT_WRITE)
        {
            task_client->clientInfo.sslEnable = false;
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return NULL;
        }

        if (ssl_accept_rtn != -1)
        {
            task_client->clientInfo.sslEnable = true;
            perr (true, LOG_INFO,
                  "raspiEventPoll: SSL_accept success: client[%d] form %s:%d",
                  task_client->clientInfo.fd,
                  inet_ntoa (task_client->clientInfo.addr.sin_addr),
                  ntohs (task_client->clientInfo.addr.sin_port));
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return NULL;
        } else
        {
            perr (true, LOG_NOTICE,
                  "raspiEventPoll: SSL_accept failed: client[%d] form %s:%d",
                  task_client->clientInfo.fd,
                  inet_ntoa (task_client->clientInfo.addr.sin_addr),
                  ntohs (task_client->clientInfo.addr.sin_port));
//            ERR_print_errors_fp (stdout);
//            epoll_ctl (server_accept_raspi.epfd, EPOLL_CTL_DEL, task_client->clientInfo.fd, NULL);
//            hash_table_client_del (hash_table_raspi, task_client->clientInfo.fd, task_client->clientInfo.fd);

            // 服务器关闭套接字, 触发HUP事件
            close (task_client->clientInfo.fd);
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return (void *) -1;
        }
    }

    // 客户端 UUID 同步消息
    if (memcmp (task_client->clientInfo.id, id_zero, sizeof (uuid_t)) == 0)
    {

        negotiateUUID (& task_client->clientInfo);
        memcpy (task_client->clientInfo.id, task_client->clientInfo.id, sizeof (uuid_t));
        pthread_mutex_unlock (& task_client->clientInfo.onProcess);
        return NULL;
    }


    memset (task_client->clientInfo.buf, 0, BUFSIZ);
    read_len = readClient (task_client->clientInfo, task_client->clientInfo.buf, BUFSIZ);


    // 常规消息
    if (read_len == sizeof (RaspiMonitData))
    {
        printf ("Data From Client %d:\n", task_client->clientInfo.fd);
        printf ("cpu temp = %.2fC\n", (* (RaspiMonitData *) task_client->clientInfo.buf).cpu_temper);
        printf ("distance = %.2fcm\n", (* (RaspiMonitData *) task_client->clientInfo.buf).distance);
        printf ("env temp = %.2fC\n", (* (RaspiMonitData *) task_client->clientInfo.buf).env_temper);
        printf ("env humid = %.2f\n\n", (* (RaspiMonitData *) task_client->clientInfo.buf).env_humidity);
        time_t now = time (NULL);
        struct tm tp;
        localtime_r (& now, & tp); //转换为struct tm类型
        char date[11]; //用于存储日期字符串
        char time[9]; //用于存储时间字符串
        strftime (date, 11, "%Y-%m-%d", & tp); //格式化日期
        strftime (time, 9, "%H:%M:%S", & tp); //格式化时间

        char uuid_string[40] = {0};
        uuid_unparse (task_client->clientInfo.id, uuid_string);
        char insert_filed[] = "(record_date,record_time,client_fd,client_uuid,cpu_temp,distance,env_humi,env_temp)";
        char insert_values[300] = {0};
        sprintf (insert_values, "('%s','%s',%d,'%s',%.2f,%.2f,%.2f,%.2f)",
                 date, time, task_client->clientInfo.fd, uuid_string,
                 (* (RaspiMonitData *) task_client->clientInfo.buf).cpu_temper,
                 (* (RaspiMonitData *) task_client->clientInfo.buf).distance,
                 (* (RaspiMonitData *) task_client->clientInfo.buf).env_temper,
                 (* (RaspiMonitData *) task_client->clientInfo.buf).env_humidity);
//        while(task_client->clientInfo.sql != NULL)
        task_client->clientInfo.sql = sql_pool_conn_fetch (sql_pool_accept_raspi);
        if (task_client->clientInfo.sql == NULL)
        {
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return (void *) -1;
        }
        mysql_insert_values (task_client->clientInfo.sql->connection, SQL_TABLE_RASPI, insert_filed, insert_values);
        sql_pool_conn_release (sql_pool_accept_raspi, & task_client->clientInfo.sql);

        hash_node_sql_data_t real_time_data;
        real_time_data.next = NULL;
        real_time_data.socket_fd = task_client->clientInfo.fd;
        sprintf (real_time_data.time_stamp, "%s %s", date, time);
        real_time_data.monitData = * (RaspiMonitData *) task_client->clientInfo.buf;
        sprintf (real_time_data.uuid, "%s", uuid_string);
        struct timespec wait;
        clock_gettime (CLOCK_REALTIME, & wait);
        wait.tv_sec += 5;
        hash_table_info_update (hash_table_info_raspi_http, & real_time_data, wait);
    }
    pthread_mutex_unlock (& task_client->clientInfo.onProcess);
    return NULL;
}

void * httpEventPoll (void * args)
{
    if (args == NULL)
        return NULL;
    server_info_t serverInfo = * (server_info_t *) args;

    int ev_num;
    thread_task_t task;
    socklen_t client_addr_size;
    hash_node_client_t * client_hash_node;
    struct sockaddr_in client_addr;
    struct epoll_event client_ev;

    while (true)
    {
        ev_num = epoll_wait (serverInfo.epfd, event_server_http, SERVER_EPOLL_SIZE, 5000);
        if (ev_num == -1)
            break;

        for (int i = 0; i < ev_num; i++)
        {
            // 手动调用一次 read 函数得到 0 或者 -1 的返回值才会发生此事件(服务器和客户端调用close皆可)
            // 这里用于处理非预期的连接断开,客户端软件遭遇SIGKILL信号,客户端遭遇遭遇突然断电等
            // 及时清理掉无效连接
            if (event_server_http[i].events & EPOLLRDHUP ||
                event_server_http[i].events & EPOLLERR ||
                event_server_http[i].events & EPOLLHUP)
            {
                client_hash_node = hash_table_client_get (hash_table_http, event_server_http[i].data.fd);
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_DEL, event_server_http[i].data.fd, NULL);

                if (client_hash_node != NULL)
                {
                    lock_robust_mutex (& client_hash_node->clientInfo.onProcess);
                    perr (true, LOG_INFO, "HTTP Client Disconnect! Delete: client[%d] form %s:%d",
                          client_hash_node->clientInfo.fd,
                          inet_ntoa (client_hash_node->clientInfo.addr.sin_addr),
                          ntohs (client_hash_node->clientInfo.addr.sin_port));
                    if (client_hash_node->clientInfo.sslEnable && client_hash_node->clientInfo.ssl_fd != NULL)
                    {
                        SSL_shutdown (client_hash_node->clientInfo.ssl_fd);
                        SSL_free (client_hash_node->clientInfo.ssl_fd);
                    }
                    close (client_hash_node->clientInfo.fd);
                    pthread_mutex_destroy (& client_hash_node->clientInfo.onProcess);
                    hash_table_client_del (hash_table_http, client_hash_node->clientInfo.fd);
                }
                continue;
            }
            // 服务器套接字事件, 一般是有新连接
            if (event_server_http[i].data.fd == serverInfo.fd)
            {
                client_addr_size = sizeof (client_addr);  // 实际上是个定值
                client_ev.data.fd = accept (serverInfo.fd, (struct sockaddr *) & client_addr, & client_addr_size);
                if (client_ev.data.fd == -1)
                {
                    perr (true, LOG_WARNING,
                          "function accept returns -1 when called httpEventPoll");
                    continue;
                }
                // 设置客户端套接字: 非阻塞
                setSockFlag (client_ev.data.fd, O_NONBLOCK, true);
                // 设置客户端套接字: KeepAlive
                //sockKeepAlive (client_ev.data.fd);
                // 设置客户端套接字: 关注的事件
                client_ev.events = EPOLLIN /*| EPOLLET*/ | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_ADD, client_ev.data.fd, & client_ev);
                perr (true, LOG_INFO,
                      "httpEventPoll: accept a client[%d] form %s:%d",
                      client_ev.data.fd, inet_ntoa (client_addr.sin_addr), ntohs (client_addr.sin_port));
                client_hash_node = calloc (1, sizeof (hash_node_client_t));


                client_hash_node->next = NULL;
                client_hash_node->clientInfo.ssl_fd = NULL;
                client_hash_node->clientInfo.sslEnable = false;
                client_hash_node->hash_node_key = client_ev.data.fd;
                client_hash_node->clientInfo.fd = client_ev.data.fd;
                client_hash_node->clientInfo.addr = client_addr;
                client_hash_node->clientInfo.addr_len = (int) client_addr_size;
                client_hash_node->clientInfo.sql = NULL;
                create_default_mutex (& client_hash_node->clientInfo.onProcess);
                memset (client_hash_node->clientInfo.id, 0, sizeof (uuid_t));
                hash_table_client_add (hash_table_http, (int) client_hash_node->hash_node_key, client_hash_node);

            }
                // 其他套接字事件, 一般是客户端套接字
            else
            {
                client_hash_node = hash_table_client_get (hash_table_http, event_server_http[i].data.fd);
                memset (& task, 0, sizeof (task));
                task.state = newlyBuild;
                task.ctime = time (NULL);

                task.args = (void *) & client_hash_node->clientInfo.fd;
                task.func = processHttpClient;
                thread_pool_add_task (thread_pool_accept_http, & task);
//                task.func (task.args);
            }
        }
    }
    return (void *) -1;
}

void * processHttpClient (void * args)
{
    int client_fd = * (int *) args;
    int read_len;
    hash_node_client_t * task_client = hash_table_client_get (hash_table_http, client_fd);
    if (task_client == NULL)return NULL;
    if (pthread_mutex_trylock (& task_client->clientInfo.onProcess) != 0)
        return NULL;

    // SSL 握手: "如果服务器套接字为SSL模式,但是客户套接字仍然为非SSL模式"
    if (server_accept_http.sslEnable && !task_client->clientInfo.sslEnable)
    {
        if (task_client->clientInfo.ssl_fd == NULL)
//            task_client->clientInfo.ssl_fd = SSL_new (server_accept_http.ssl_ctx);
//
//        SSL_set_fd (task_client->clientInfo.ssl_fd, task_client->clientInfo.fd);
            task_client->clientInfo.ssl_fd = SSL_fd (server_accept_http.ssl_ctx, task_client->clientInfo.fd);

        int ssl_accept_rtn = SSL_accept (task_client->clientInfo.ssl_fd);
        int ssl_rtn = SSL_get_error (task_client->clientInfo.ssl_fd, ssl_accept_rtn);
        if (ssl_rtn == SSL_ERROR_WANT_READ || ssl_rtn == SSL_ERROR_WANT_WRITE)
        {
            task_client->clientInfo.sslEnable = false;
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return NULL;
        }

        if (ssl_accept_rtn != -1)
        {
            task_client->clientInfo.sslEnable = true;
            perr (true, LOG_INFO,
                  "httpEventPoll: SSL_accept success: client[%d] form %s:%d",
                  task_client->clientInfo.fd,
                  inet_ntoa (task_client->clientInfo.addr.sin_addr),
                  ntohs (task_client->clientInfo.addr.sin_port));
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return NULL;
        } else
        {
            perr (true, LOG_NOTICE,
                  "httpEventPoll: SSL_accept failed: client[%d] form %s:%d",
                  task_client->clientInfo.fd,
                  inet_ntoa (task_client->clientInfo.addr.sin_addr),
                  ntohs (task_client->clientInfo.addr.sin_port));
//            ERR_print_errors_fp (stdout);
//            epoll_ctl (server_accept_http.epfd, EPOLL_CTL_DEL, task_client->clientInfo.fd, NULL);
//            hash_table_client_del (hash_table_http, task_client->clientInfo.fd, task_client->clientInfo.fd);

            // 手动触发 epoll 函数的 HUP 事件
            close (task_client->clientInfo.fd);
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return (void *) -1;
        }
    }


    memset (task_client->clientInfo.buf, 0, BUFSIZ);
    read_len = readClient (task_client->clientInfo, task_client->clientInfo.buf, BUFSIZ);
    if (read_len <= 0)
    {
        pthread_mutex_unlock (& task_client->clientInfo.onProcess);
        return (void *) -1;
    }

    http_request_t req;
    http_request_get (task_client->clientInfo.buf, & req);
    http_request_print (& req);



    // 初始请求
    if (strlen (req.URI) == 1 && req.URI[0] == '/')
    {
        http_response_t res;
        http_response_generate (& res, req.version, HTTP_200, keepalive);
        char buf_http[web_html_size + 200];
//        http_response_add_content (& res, web_html_buf, (int) web_html_size, text_html);
        res.contentType = text_html;
        res.contentLength = (int) web_html_size;
        http_response_tostring (& res, buf_http);
        writeClient (task_client->clientInfo, buf_http, (int) strlen (buf_http));
        writeClient (task_client->clientInfo, "\r\n", 2);
        writeClient (task_client->clientInfo, web_html_buf, (int) web_html_size);
        pthread_mutex_unlock (& task_client->clientInfo.onProcess);
        return NULL;
    } else if (igStrCmp (req.URI, "/fetch_all_data", (int) strlen (req.URI)) == 0)
    {
        http_response_t res;
        http_response_generate (& res, req.version, HTTP_200, keepalive);
        struct timespec wait;
        clock_gettime (CLOCK_REALTIME, & wait);
        wait.tv_sec += 6;
        if (pthread_rwlock_timedwrlock (& hash_table_info_raspi_http->lock, & wait) != 0)
        {
            res.status = HTTP_204;
            char buf_http[200];
            char * end = http_response_tostring (& res, buf_http);
            writeClient (task_client->clientInfo, buf_http, (int) (end - buf_http));
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return NULL;
        }

        int client_num = hash_table_info_raspi_http->current;
        int alloc_size;
        if (client_num == 0)
            alloc_size = 2;
        else
            alloc_size = 1/*JSON数组左括号*/ +
                         ((client_num - 1) * 140)/*N-1 个 js 对象(JSON字符串,附带逗号)*/ +
                         139/*最后一个js对象*/ + 1/*JSON数组右括号*/ + 8 * client_num /*安全缓冲区*/;
        char * buf_http = calloc (1, alloc_size + 200);
        char * buf_json = (char *) calloc (1, alloc_size);
        char * pointer = buf_json;
        if (buf_json == NULL || buf_http == NULL)
        {
            res.status = HTTP_204;
            char buf_http_back[200];
            char * end = http_response_tostring (& res, buf_http_back);
            writeClient (task_client->clientInfo, buf_http_back, (int) (end - buf_http_back));
            if (buf_http != NULL)
                free (buf_http);
            if (buf_json != NULL)
                free (buf_json);
            pthread_rwlock_unlock (& hash_table_info_raspi_http->lock);
            pthread_mutex_unlock (& task_client->clientInfo.onProcess);
            return NULL;
        }

        sprintf (buf_json, "[");
        pointer++;
        hash_node_sql_data_t * data;
        for (int i = 0, count = hash_table_info_raspi_http->current;
             i < hash_table_info_raspi_http->size; i++)
        {
            if (hash_table_info_raspi_http->hashTable[i] == NULL)
                continue;

            if (count == 0)break;

            data = hash_table_info_raspi_http->hashTable[i];
            while (data != NULL && count != 0)
            {
                pointer += sprintf (pointer,
                                    "{\"timeStamp\":\"%s\","
                                    "\"clientID\":\"%s\","
                                    "\"cpuTemp\":\"%.0f\",\"distance\":\"%.0f\","
                                    "\"envTemp\":\"%.0f\",\"envHumi\":\"%.0f\"},",
                                    data->time_stamp, data->uuid, data->monitData.cpu_temper,
                                    data->monitData.distance, data->monitData.env_temper,
                                    data->monitData.env_humidity);

                count--;
                data = data->next;
            }
        }
        if (* (pointer - 1) == ',')
            * (--pointer) = ']';
        else * pointer = ']';

        pthread_rwlock_unlock (& hash_table_info_raspi_http->lock);


//        http_response_add_content (& res, buf_json, (int) strlen (buf_json), application_json);
        res.contentLength = (int) strlen (buf_json);
        res.contentType = application_json;
        char * end = http_response_tostring (& res, buf_http);
        writeClient (task_client->clientInfo, buf_http, (int) (end - buf_http));
        writeClient (task_client->clientInfo, "\r\n", 2);
        writeClient (task_client->clientInfo, buf_json, (int) strlen (buf_json));
        free (buf_json);
        free (buf_http);
        pthread_mutex_unlock (& task_client->clientInfo.onProcess);
        return NULL;
    } else
    {
        char buf_http[200];
        http_response_t res;
        http_response_generate (& res, req.version, HTTP_404, closed);
        http_response_add_content (& res, "<h1>404 Not Found</h1>", 22, text_html);
        char * end = http_response_tostring (& res, buf_http);
        writeClient (task_client->clientInfo, buf_http, (int) (end - buf_http));
        pthread_mutex_unlock (& task_client->clientInfo.onProcess);
        return NULL;
    }
}