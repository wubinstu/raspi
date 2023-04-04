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
                client_hash_node = hash_table_get (hash_map_raspi, event_server_raspi[i].data.fd);
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_DEL, event_server_raspi[i].data.fd, NULL);

                if (client_hash_node != NULL)
                {
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
                    hash_table_del (hash_map_raspi, client_hash_node->clientInfo.fd, client_hash_node->clientInfo.fd);
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
                memset (client_hash_node->clientInfo.id, 0, sizeof (uuid_t));
                hash_table_add (hash_map_raspi, (int) client_hash_node->hash_node_key, client_hash_node);

            }
                // 其他套接字事件, 一般是客户端套接字
            else
            {
                client_hash_node = hash_table_get (hash_map_raspi, event_server_raspi[i].data.fd);
                memset (& task, 0, sizeof (task));
                task.state = newlyBuild;
                task.ctime = time (NULL);
//                if (task.client->sql == NULL)
//                    task.client->sql = sql_pool_conn_fetch (sql_pool_accept_raspi);
                task.args = (void *) & client_hash_node->clientInfo.fd;
                task.func = processRaspiClient;
                thread_pool_add_task (thread_pool_accept_raspi, & task);
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
    hash_node_client_t * task_client = hash_table_get (hash_map_raspi, client_fd);

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
        } else
        {
            perr (true, LOG_INFO,
                  "raspiEventPoll: SSL_accept failed: client[%d] form %s:%d",
                  task_client->clientInfo.fd,
                  inet_ntoa (task_client->clientInfo.addr.sin_addr),
                  ntohs (task_client->clientInfo.addr.sin_port));
            ERR_print_errors_fp (stdout);
//            epoll_ctl (server_accept_raspi.epfd, EPOLL_CTL_DEL, task_client->clientInfo.fd, NULL);
//            hash_table_del (hash_map_raspi, task_client->clientInfo.fd, task_client->clientInfo.fd);

            // 服务器关闭套接字, 触发HUP事件
            close (task_client->clientInfo.fd);
            return (void *) -1;
        }
        return NULL;
    }

    // 客户端 UUID 同步消息
    if (memcmp (task_client->clientInfo.id, id_zero, sizeof (uuid_t)) == 0)
    {

        negotiateUUID (& task_client->clientInfo);
        memcpy (task_client->clientInfo.id, task_client->clientInfo.id, sizeof (uuid_t));
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
            return (void *) -1;
        mysql_insert_values (task_client->clientInfo.sql->connection, SQL_TABLE_RASPI, insert_filed, insert_values);
        sql_pool_conn_release (sql_pool_accept_raspi, & task_client->clientInfo.sql);
    }
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
                client_hash_node = hash_table_get (hash_map_http, event_server_http[i].data.fd);
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_DEL, event_server_http[i].data.fd, NULL);

                if (client_hash_node != NULL)
                {
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
                    hash_table_del (hash_map_http, client_hash_node->clientInfo.fd, client_hash_node->clientInfo.fd);
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
                memset (client_hash_node->clientInfo.id, 0, sizeof (uuid_t));
                hash_table_add (hash_map_http, (int) client_hash_node->hash_node_key, client_hash_node);

            }
                // 其他套接字事件, 一般是客户端套接字
            else
            {
                client_hash_node = hash_table_get (hash_map_http, event_server_http[i].data.fd);
                memset (& task, 0, sizeof (task));
                task.state = newlyBuild;
                task.ctime = time (NULL);

                task.args = (void *) & client_hash_node->clientInfo.fd;
                task.func = processHttpClient;
//                thread_pool_add_task (thread_pool_accept_http, & task);
                task.func (task.args);
            }
        }
    }
    return (void *) -1;
}

void * processHttpClient (void * args)
{
    int client_fd = * (int *) args;
    int read_len;
    hash_node_client_t * task_client = hash_table_get (hash_map_http, client_fd);

    // SSL 握手: "如果服务器套接字为SSL模式,但是客户套接字仍然为非SSL模式"
    if (server_accept_http.sslEnable && !task_client->clientInfo.sslEnable)
    {
        if (task_client->clientInfo.ssl_fd == NULL)
            task_client->clientInfo.ssl_fd = SSL_new (server_accept_http.ssl_ctx);

        SSL_set_fd (task_client->clientInfo.ssl_fd, task_client->clientInfo.fd);

        int ssl_accept_rtn = SSL_accept (task_client->clientInfo.ssl_fd);
        int ssl_rtn = SSL_get_error (task_client->clientInfo.ssl_fd, ssl_accept_rtn);
        if (ssl_rtn == SSL_ERROR_WANT_READ || ssl_rtn == SSL_ERROR_WANT_WRITE)
        {
            task_client->clientInfo.sslEnable = false;
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
            return NULL;
        } else
        {
            perr (true, LOG_INFO,
                  "httpEventPoll: SSL_accept failed: client[%d] form %s:%d",
                  task_client->clientInfo.fd,
                  inet_ntoa (task_client->clientInfo.addr.sin_addr),
                  ntohs (task_client->clientInfo.addr.sin_port));
            ERR_print_errors_fp (stdout);
            epoll_ctl (server_accept_http.epfd, EPOLL_CTL_DEL, task_client->clientInfo.fd, NULL);
            hash_table_del (hash_map_http, task_client->clientInfo.fd, task_client->clientInfo.fd);
            close (task_client->clientInfo.fd);
            return (void *) -1;
        }
    }


    memset (task_client->clientInfo.buf, 0, BUFSIZ);
    read_len = readClient (task_client->clientInfo, task_client->clientInfo.buf, BUFSIZ);
    if (read_len <= 0)return (void *) -1;

    http_request_t * req = http_request_get (task_client->clientInfo.buf);
    http_request_print (req);


    http_response_t * res = http_response_generate (req->version, HTTP_200, keepalive);
    char * buf = calloc (3, 1024 * 1024);
    // 初始请求
    if (strlen (req->URI) == 1)
    {
        http_response_add_content (res, web_html_buf, (int) web_html_size, text_html);
        http_response_tostring (res, buf);
        writeClient (task_client->clientInfo, buf, (int) strlen (buf));
    } else if (igStrCmp (req->URI, "/door.jpg", (int) strlen (req->URI)) == 0)
    {
        http_response_add_content (res, web_html_bg_png_buf, (int) web_html_bg_png_size, image_jpeg);
        char * end = http_response_tostring (res, buf);
        writeClient (task_client->clientInfo, buf, (int) (end - buf));


//        res->contentLength = (int) web_html_bg_png_size;
//        res->contentType = image_jpeg;
//        http_response_tostring (res, buf);
//        writeClient (task_client->clientInfo, buf, (int) strlen (buf));
//        writeClient (task_client->clientInfo, "\r\n", 2);
//        writeClient (task_client->clientInfo, web_html_bg_png_buf, (int) web_html_bg_png_size);
    } else if (igStrCmp (req->URI, "/fetch_all_data", (int) strlen (req->URI)) == 0)
    {
//        char buf_json[] = "{\"data\": [{\"label\": \"Temperature\", \"value\": \"25C\"}, {\"label\": \"Humidity\", \"value\": \"60%\"}]}";
        char buf_json[] = "[{ \"timeStamp\": \"20:07\", \"clientID\": \"abc\", \"cpuTemp\": \"51.2C\", \"distance\": \"123.45\", \"envTemp\": \"23.12\", \"envHumi\": \"34%\"}, "
                          "{ \"timeStamp\": \"12:07\", \"clientID\": \"efg\", \"cpuTemp\": \"44.5C\", \"distance\": \"654.12\", \"envTemp\": \"12.45\", \"envHumi\": \"78%\"}]";
        http_response_add_content (res, buf_json, (int) strlen (buf_json), application_json);
        char * end = http_response_tostring (res, buf);
        writeClient (task_client->clientInfo, buf, (int) (end - buf));
    }


    http_response_free (res);
    http_request_free (req);
    free (buf);


    return NULL;
}