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

/// TODO
/// mysql
/// [Done] thread pool
/// webserver
int main (int argc, const char * argv[])
{
    mysql_lib (true);
    confToVarServ ();
    runTimeArgsServ (argc, argv);


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

    sigRegisterServ ();

    sql_pool_accept_raspi =
            sql_pool_init (config_server.sqlHost,
                           config_server.sqlPort,
                           config_server.sqlUser,
                           config_server.sqlPass,
                           config_server.sqlName,
                           SQL_POOL_MAX, SQL_POOL_MIN);
    thread_pool_accept_raspi =
            thread_pool_init (THREAD_POOL_MAX, THREAD_POOL_MIN, THREAD_POOL_QUEUE);
//    thread_pool_accept_http =
//            thread_pool_init (THREAD_POOL_MAX, THREAD_POOL_MIN, THREAD_POOL_QUEUE);

    hash_map_raspi = hash_map_init (HASH_MAP_SIZE);

    perr (!initServerSocket (), LOG_ERR,
          "function initServerSocket returns false when called main");

    loadSSLServ ();

    server_accept_raspi.epfd = createServEpoll (server_accept_raspi.fd);

    eventPoll (server_accept_raspi);

    exitCleanupServ ();
    return 0;
}