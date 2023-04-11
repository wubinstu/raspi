//
// Created by Einc on 2023/03/19.
//

#include "global.h"
#include "load_clean.h"
#include "self_sql.h"
#include "self_thread.h"
#include "sql_pool.h"
#include "thread_pool.h"
#include "types.h"
#include "rsignal.h"
#include "socket_fd.h"
#include "run_server.h"
#include "log.h"
#include "filed.h"
#include "ssl_pool.h"

/// TODO
/// [Done] mysql
/// [Done] thread pool
/// webserver
int main (int argc, const char * argv[])
{
    mysql_lib (true);
    preRunTimeArgsServ (argc, argv);
    confToVarServ ();
    runTimeArgsServ (argc, argv);
    openlog (PROJECT_SERVER_NAME, LOG_CONS | LOG_PID, LOG_DAEMON);


    if (strcmp (config_server.pidFile, "disable") != 0)
        if (!checkRootPermission ("To create pid file"))
            return -1;

    if (strcmp (config_server.pidFile, "default") == 0)
        checkPidFileServ (PID_FILE_SERVER);
    else if (strcmp (config_server.pidFile, "disable") == 0)
        checkPidFileServ (NULL);
    else checkPidFileServ (config_server.pidFile);

    if (config_server.modeDaemon)
        daemonize (PROJECT_CLIENT_NAME);

    if (sigsetjmp(jmp_server_rest, true) != 0)
        confToVarServ ();

    //// TODO
//    sigRegisterServ ();

    sql_pool_accept_raspi =
            sql_pool_init (config_server.sqlHost,
                           config_server.sqlPort,
                           config_server.sqlUser,
                           config_server.sqlPass,
                           config_server.sqlName,
                           SQL_POOL_MAX, SQL_POOL_MIN);
    thread_pool_accept_raspi =
            thread_pool_init (THREAD_POOL_MAX, THREAD_POOL_MIN, THREAD_POOL_QUEUE);
    thread_pool_accept_http =
            thread_pool_init (THREAD_POOL_MAX, THREAD_POOL_MIN, THREAD_POOL_QUEUE);

    sql_node_t * for_init_table = sql_pool_conn_fetch (sql_pool_accept_raspi);
    char sql[300] = {0};
    sprintf (sql, "create table if not exists %s %s", SQL_TABLE_RASPI,
             "(record_id BIGINT not null auto_increment primary key,"
             "record_date DATE not null,"
             "record_time TIME not null,"
             "client_fd INTEGER not null,"
             "client_uuid VARCHAR(40) not null,"
             "cpu_temp FLOAT(2),"
             "distance FLOAT(2),"
             "env_humi FLOAT(2),"
             "env_temp FLOAT(2))");
    int ret = mysql_query (for_init_table->connection, sql);
    if (ret != 0)
        perr (true, LOG_WARNING, "Create table Error: %d %s\n",
              mysql_errno (for_init_table->connection),
              mysql_error (for_init_table->connection));
    sql_pool_conn_release (sql_pool_accept_raspi, & for_init_table);

    hash_table_raspi = hash_table_client_init (HASH_TABLE_SIZE);
    hash_table_http = hash_table_client_init (HASH_TABLE_SIZE);
    hash_table_info_raspi_http = hash_table_info_init (HASH_TABLE_SIZE);


    web_html_fd = readOpen (WEB_HTML_PAGE);
    web_html_size = fileSize (WEB_HTML_PAGE);
    web_html_buf = mmap (NULL, web_html_size, PROT_READ, MAP_PRIVATE, web_html_fd, 0);

    perr (!initServerSocket (& server_accept_raspi, config_server.bindPort), LOG_ERR,
          "function initServerSocket(raspi) returns false when called main");
    perr (!initServerSocket (& server_accept_http, config_server.httpPort), LOG_ERR,
          "function initServerSocket(http) returns false when called main");

    if (config_server.modeSSL == true)
    {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        OPENSSL_malloc_init();
        OPENSSL_init ();
        OPENSSL_init_ssl (0, NULL);
        CRYPTO_set_locking_callback(locking_function);
        CRYPTO_set_dynlock_create_callback(dyn_create_function);
        CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
        CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
    }
    loadSSLServ (& server_accept_http);
    loadSSLServ (& server_accept_raspi);

    if (server_accept_raspi.sslEnable)
        ssl_pool_accept_raspi = ssl_pool_init (SSL_POOL_MAX, SSL_POOL_MIN, server_accept_raspi.ssl_ctx);
    if (server_accept_http.sslEnable)
        ssl_pool_accept_http = ssl_pool_init (SSL_POOL_MAX, SSL_POOL_MIN, server_accept_http.ssl_ctx);

    server_accept_raspi.epfd = createServEpoll (server_accept_raspi.fd);
    server_accept_http.epfd = createServEpoll (server_accept_http.fd);

    thread_id_server_accept_raspi = create_default_thread (raspiEventPoll, (void *) & server_accept_raspi);
    thread_id_server_accept_http = create_default_thread (httpEventPoll, (void *) & server_accept_http);

//    raspiEventPoll ((void *) & server_accept_raspi);
//    httpEventPoll ((void *) & server_accept_http);

    while (true)
    {
        sleep (10);
        if (time (NULL) == 0)  // 本条件永远不肯能成立, 这里是为了消除CLion软件的无限循环警告
            break;
    }

    exitCleanupServ ();
    return 0;
}