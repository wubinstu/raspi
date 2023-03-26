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
    else return true;
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
    char uuid_client_string[40];
    readClient (* clientInfo, uuid_client, sizeof (uuid_t));
    if (uuid_compare (uuid_client, (uuid_t) {UUID_NONE}) == 1)
        uuid_generate (uuid_client),
                writeClient (* clientInfo, uuid_client, sizeof (uuid_t));
    memcpy (clientInfo->id, uuid_client, sizeof (uuid_t));

    uuid_unparse (uuid_client, uuid_client_string);

    perr (true, LOG_INFO, "client[%d] form %s:%d UUID: %s",
          clientInfo->fd, inet_ntoa (clientInfo->addr.sin_addr),
          ntohs (clientInfo->addr.sin_port), (char *) uuid_client_string);

}

void eventPoll (server_info_t serverInfo)
{
    int ev_num;
    thread_task_t task;
//    const int EPOLL_SIZE = 50;
    socklen_t client_addr_size;
    hash_node_t * client_hash_node;
    struct sockaddr_in client_addr;
    struct epoll_event /* ev[EPOLL_SIZE], */ client_ev;

    while (true)
    {
        ev_num = epoll_wait (serverInfo.epfd, event_server_raspi, SERVER_EPOLL_SIZE, -1);
        if (ev_num == -1)
            break;

        for (int i = 0; i < ev_num; i++)
        {
            // 手动调用一次 read 函数得到 0 或者 -1 的返回值才会发生此事件(服务器和客户端调用close皆可)
            if (event_server_raspi[i].events & EPOLLRDHUP)
            {
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_DEL, event_server_raspi[i].data.fd, NULL);
                client_hash_node = hash_map_get (hash_map_raspi, event_server_raspi[i].data.fd,
                                                 event_server_raspi[i].data.fd);
                if (client_hash_node->clientInfo.sql != NULL)
                    sql_pool_conn_release (sql_pool_accept_raspi, client_hash_node->clientInfo.sql);
                if (client_hash_node->clientInfo.sslEnable)
                {
                    SSL_shutdown (client_hash_node->clientInfo.ssl_fd);
                    SSL_free (client_hash_node->clientInfo.ssl_fd);
                }
                close (client_hash_node->clientInfo.fd);
                hash_map_del (hash_map_raspi, event_server_raspi[i].data.fd, event_server_raspi[i].data.fd);
            }
            // 服务器套接字事件, 一般是有新连接
            if (event_server_raspi[i].data.fd == serverInfo.fd)
            {
                client_addr_size = sizeof (client_addr);  // 实际上是个定值
                client_ev.data.fd = accept (serverInfo.fd, (struct sockaddr *) & client_addr, & client_addr_size);
                if (client_ev.data.fd == -1)
                {
                    perr (true, LOG_WARNING,
                          "function accept returns -1 when called eventPoll");
                    continue;
                }
                // 设置客户端套接字: 非阻塞
                setSockFlag (client_ev.data.fd, O_NONBLOCK, true);
                client_ev.events = EPOLLIN | EPOLLET;
                epoll_ctl (serverInfo.epfd, EPOLL_CTL_ADD, client_ev.data.fd, & client_ev);
                perr (true, LOG_INFO,
                      "eventPoll: accept a client[%d] form %s:%d",
                      client_ev.data.fd, inet_ntoa (client_addr.sin_addr), ntohs (client_addr.sin_port));
                client_hash_node = calloc (1, sizeof (hash_node_t));
                if (serverInfo.sslEnable)
                {
                    client_hash_node->clientInfo.sslEnable = true;
                    client_hash_node->clientInfo.ssl_fd = SSL_new (serverInfo.ssl_ctx);
                    SSL_set_fd (client_hash_node->clientInfo.ssl_fd, client_ev.data.fd);

                    if (SSL_accept (client_hash_node->clientInfo.ssl_fd) == -1)
                    {
                        perr (true, LOG_INFO,
                              "eventPoll: SSL_accept failed: client[%d] form %s:%d",
                              client_ev.data.fd, inet_ntoa (client_addr.sin_addr), ntohs (client_addr.sin_port));
                        free (client_hash_node);
                        epoll_ctl (serverInfo.epfd, EPOLL_CTL_DEL, client_ev.data.fd, & client_ev);
                        close (client_ev.data.fd);

                        continue;
                    }

                }
                client_hash_node->next = NULL;
                client_hash_node->hash_node_key = client_ev.data.fd;
                client_hash_node->clientInfo.fd = client_ev.data.fd;
                client_hash_node->clientInfo.addr = client_addr;
                client_hash_node->clientInfo.addr_len = (int) client_addr_size;
                uuid_generate (client_hash_node->clientInfo.id);
                client_hash_node->clientInfo.sql = sql_pool_conn_fetch (sql_pool_accept_raspi);
                negotiateUUID (& client_hash_node->clientInfo);
                hash_map_put (hash_map_raspi, (int) client_hash_node->hash_node_key, client_hash_node);

            }
                // 其他套接字事件, 一般是客户端套接字
            else
            {
                client_hash_node = hash_map_get (hash_map_raspi, event_server_raspi[i].data.fd,
                                                 event_server_raspi[i].data.fd);
                memset (& task, 0, sizeof (task));
                task.state = newlyBuild;
                task.ctime = time (NULL);
                task.client = & client_hash_node->clientInfo;
                if (task.client->sql == NULL)
                    task.client->sql = sql_pool_conn_fetch (sql_pool_accept_raspi);
                task.args = (void *) task.client;
                task.func = processRaspiClient;
                thread_pool_add_task (thread_pool_accept_raspi, & task);
            }
        }
    }
}

void * processRaspiClient (void * args)
{
    client_info_t * clientInfo = (client_info_t *) args;
    int read_len;
    if (clientInfo->sslEnable)
        read_len = SSL_read (clientInfo->ssl_fd, clientInfo->buf, BUFSIZ);
    else read_len = (int) read (clientInfo->fd, clientInfo->buf, BUFSIZ);
    readClient (* clientInfo, clientInfo->buf, BUFSIZ);

    if (read_len == sizeof (raspi_monit_data))
    {
        printf ("Data From Client %d:\n", clientInfo->fd);
        printf ("cpu temp = %.2fC\n", (* (RaspiMonitData *) clientInfo->buf).cpu_temper);
        printf ("distance = %.2fcm\n", (* (RaspiMonitData *) clientInfo->buf).distance);
        printf ("env temp = %.2fC\n", (* (RaspiMonitData *) clientInfo->buf).env_temper);
        printf ("env humid = %.2f\n\n", (* (RaspiMonitData *) clientInfo->buf).env_humidity);
    }
    if (read_len <= 4)
    {
        if (strcmp (clientInfo->buf, "FIN") == 0)
        {
            epoll_ctl (server_accept_raspi.epfd, EPOLL_CTL_DEL, clientInfo->fd, NULL);
            if (clientInfo->sql != NULL)
                sql_pool_conn_release (sql_pool_accept_raspi, clientInfo->sql);
            if (clientInfo->sslEnable)
            {
                SSL_shutdown (clientInfo->ssl_fd);
                SSL_free (clientInfo->ssl_fd);
            }
            close (clientInfo->fd);
            hash_map_del (hash_map_raspi, clientInfo->fd, clientInfo->fd);
            return NULL;
        }
    }
    if (clientInfo->sslEnable)
        SSL_write (clientInfo->ssl_fd, "CON", 4);
    else write (clientInfo->fd, "CON", 4);

    return NULL;
}

void * processHttpClient (void * args)
{

}