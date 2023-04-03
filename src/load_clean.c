//
// Created by Einc on 2022/4/1.
//

#include "arr.h"
#include "conf.h"
#include "global.h"
#include "log.h"
#include "load_clean.h"
#include "socket_fd.h"
#include "rssl.h"
#include "raspi_drive.h"
#include "self_sql.h"
#include "self_thread.h"
#include "sql_pool.h"
#include "thread_pool.h"

void daemonize (const char * cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /**
     * Clear file creation mask
     * 清除文件创建权限位屏蔽字 */
    umask (0);

    /**
     * Get maximum number of file descriptors
     * 获取文件描述符最大值 */
    if (getrlimit (RLIMIT_NOFILE, & rl) < 0)
        perr (true, LOG_NOTICE,
              "%s[daemonize]: can not get file limit", cmd);

    /**
     * Become a session leader to lose controlling TTY
     * fork 后只保留子进程,这样可以脱离控制终端,并且为创建会话做准备 */
    if ((pid = fork ()) < 0)
        perr (true, LOG_ERR,
              "%s[daemonize]: cant not fork", cmd);
    else if (pid != 0)
        exit (0);

    setsid ();

    /**
     * 约定成俗的规则,守护进程一般不需要处理SIGHUP, 当然有些软件使用该信号来"重启(配置文件修改时)"软件 */
    sa.sa_handler = SIG_IGN;
    sigemptyset (& sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction (SIGHUP, & sa, NULL) < 0)
        perr (true, LOG_NOTICE,
              "%s[daemonize]: can not ignore SIGHUP", cmd);

    /**
     * Ensure future opens won't allocate controlling TTYs.
     * 再一次 fork, 还是只保留子进程,这样子进程不是会话首进程,那么它永远也无法获取控制终端 */
    if ((pid = fork ()) < 0)
        perr (true, LOG_ERR,
              "%s[daemonize]: can not fork", cmd);
    else if (pid != 0)
        exit (0);

    /**
     * Change the current working directory to the root so
     * we won't prevent file system from being unmounted
     * 将当前工作目录更改为根目录,这样就不会阻止卸载文件系统 */
    if (chdir ("/") < 0)
        perr (true, LOG_ERR,
              "%s[daemonize]: can not change directory to /", cmd);

    /**
     * close all open file descriptors
     * 关闭所有现有文件描述符 */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for (i = 0; i < rl.rlim_max; i++)
        close (i);

    /**
     * Attach file descriptors 0,1 and 2 to /dev/null
     * 重定向stdin,stdout,stderr.守护进程是不需要它们的 */
    fd0 = open ("/dev/null", O_RDWR);
    fd1 = dup (0);  // 相当于 dup(fd0) 或者 dup2(fd0,STDIN_FILENO)
    fd2 = dup (0);

    /**
     * Initialize the log file */
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        perr (true, LOG_NOTICE,
              "%s[daemonize]: unexpected file descriptors %d %d %d",
              cmd, fd0, fd1, fd2);
        exit (1);
    }
    perr (true, LOG_INFO,
          "%s[daemonize]: process successfully entered daemon mode,"
          "pid = %d,ppid = %d,pgid = %d,sid = %d",
          cmd, getpid (), getppid (), getpgrp (), getsid (getpid ()));
}

void preRunTimeArgsServ (int argc, const char * argv[])
{
    int errno_save = errno;
    char * args = calloc (1, 30);
    char * value = calloc (1, 30);
    for (int i = 1; i < argc; i++)
    {
        memset (args, 0, 30);
        memset (value, 0, 30);
        strcpy (args, argv[i]);
        if (argv[i + 1] != NULL)
            strcpy (value, argv[i + 1]);
        lowerConversion (args);

        if (strcmp (args, "--clean-pid") == 0)
        {
            char * pid_file;
            if (strcmp (config_server.pidFile, "default") == 0)
                pid_file = PID_FILE_SERVER;
            else if (strcmp (config_server.pidFile, "disable") == 0)
                pid_file = NULL;
            else pid_file = config_server.pidFile;
            if (!checkRootPermission ("to delete PID_FILE"))
                exit (-1);
            if (pid_file != NULL)
            {
                printf ("%s will be deleted\n", pid_file);
                unlink (pid_file);
            } else printf ("PIDFILE is already disable\n");
            exit (0);
        } else if (strcmp (args, "--default-conf") == 0)
        {
            printf ("resetServ %s\n", CONF_FILE_SERVER);
            unlink (CONF_FILE_SERVER);
            defaultConfServ (CONF_FILE_SERVER);
            exit (0);
        } else if (strcmp (args, "--settings") == 0)
        {
            if (!checkRootPermission ("or it will open in read-only mode"
                                      "(Press Any Key To Continue)"))
                getchar ();
            execlp ("vim", "vim", CONF_FILE_SERVER, NULL);
            printf ("vim editor not available,use vi editor instead (3 secs...)\n");
            sleep (3);
            execlp ("vi", "vi", CONF_FILE_SERVER, NULL);
            printf ("can not open conf file with vi too! "
                    "please operate manually (3 secs...)\n");
            sleep (3);
            exit (-1);
        } else if (strcmp (args, "--help") == 0)
        {
            printf ("Usage:  "
                    "[--bindaddr] [ip / hostname]\t\t\t\tspecify server bind ip address\n\t"
                    "[--bindport] [port]\t\t\t\t\tspecify server ip bind port\n\t"
                    "[--httpport] [port]\t\t\t\t\tspecify server http port\n\t"
                    "[--sslmode] [disable,default]\t\t\t\tspecify server ssl mode\n\t"
                    "[--cafile] [filepath]\t\t\t\t\tspecify ca certificate file path\n\t"
                    "[--servcert] [filepath]\t\t\t\t\tspecify server certificate file path\n\t"
                    "[--servkey] [filepath]\t\t\t\t\tspecify server private key file path\n\t"
                    "[--pidfile] [disable,default,\"file path\"]\t\tspecify pid file path\n\t"
                    "[--daemon] [disable,default]\t\t\t\tspecify daemon mode\n\t"
                    "[--sqlhost] [ip / hostname]\t\t\t\tspecify MySQL server host name\n\t"
                    "[--sqlport] [port]\t\t\t\t\tspecify MySQL server port\n\t"
                    "[--sqluser] [user]\t\t\t\t\tspecify MySQL server user\n\t"
                    "[--sqlpass] [pass]\t\t\t\t\tspecify MySQL server pass\n\t"
                    "[--sqlname] [name]\t\t\t\t\tspecify MySQL server DataBase Name\n\t"
                    "[--clean-pid]\t\t\t\t\t\tDelete PID file\n\t"
                    "[--default-conf]\t\t\t\t\tCreate(Overwrite) default configuration file\n\t"
                    "[--settings]\t\t\t\t\t\tOpen(VIM editor) configuration file\n");
            exit (0);
        } else
        {
            if (args[0] != '-')
                continue;
        }
    }
    free (args);
    free (value);
}


void preRunTimeArgsClnt (int argc, const char * argv[])
{
    int errno_save = errno;
    char * args = calloc (1, 30);
    char * value = calloc (1, 30);
    for (int i = 1; i < argc; i++)
    {
        memset (args, 0, 30);
        memset (value, 0, 30);
        strcpy (args, argv[i]);
        if (argv[i + 1] != NULL)
            strcpy (value, argv[i + 1]);
        lowerConversion (args);

        if (strcmp (args, "--clean-pid") == 0)
        {
            char * pid_file;
            if (strcmp (config_client.pidFile, "default") == 0)
                pid_file = PID_FILE_CLIENT;
            else if (strcmp (config_client.pidFile, "disable") == 0)
                pid_file = NULL;
            else pid_file = config_client.pidFile;
            if (!checkRootPermission ("to delete PID_FILE"))
                exit (-1);
            if (pid_file != NULL)
            {
                printf ("%s will be deleted\n", pid_file);
                unlink (pid_file);
            } else printf ("PIDFILE is already disable\n");
            exit (0);
        } else if (strcmp (args, "--default-conf") == 0)
        {
            printf ("resetClnt %s\n", CONF_FILE_CLIENT);
            unlink (CONF_FILE_CLIENT);
            defaultConfClnt (CONF_FILE_CLIENT);
            exit (0);
        } else if (strcmp (args, "--settings") == 0)
        {
            if (!checkRootPermission ("or it will open in read-only mode"
                                      "(Press Any Key To Continue)"))
                getchar ();
            execlp ("vim", "vim", CONF_FILE_CLIENT, NULL);
            printf ("vim editor not available,use vi editor instead (3 secs...)\n");
            sleep (3);
            execlp ("vi", "vi", CONF_FILE_CLIENT, NULL);
            printf ("can not open conf file with vi too! "
                    "please operate manually (3 secs...)\n");
            sleep (3);
            exit (-1);
        } else if (strcmp (args, "--help") == 0)
        {
            printf ("Usage:  "
                    "[--servaddr] [ip / hostname]\t\t\t\tspecify server ip address\n\t"
                    "[--servport] [port]\t\t\t\t\tspecify server ip port\n\t"
                    "[--interval] [interval]\t\t\t\t\tspecify data collection interval\n\t"
                    "[--cafile] [disable,default,\"file path\"]\t\tspecify ssl mode or ca path\n\t"
                    "[--pidfile] [disable,default,\"file path\"]\t\tspecify pid file path\n\t"
                    "[--daemon] [disable,default]\t\t\t\tspecify daemon mode\n\t"
                    "[--clean-pid]\t\t\t\t\t\tDelete PID file\n\t"
                    "[--default-conf]\t\t\t\t\tCreate(Overwrite) default configuration file\n\t"
                    "[--settings]\t\t\t\t\t\tOpen(VIM editor) configuration file\n");
            exit (0);
        } else
        {
            if (args[0] != '-')
                continue;
        }
    }
    free (args);
    free (value);
    errno = errno_save;
}

void runTimeArgsServ (int argc, const char * argv[])
{
    int errno_save = errno;
    perr (true, LOG_INFO, "Reading runTime Arguments");
    char * args = calloc (1, 30);
    char * value = calloc (1, 30);
    for (int i = 1; i < argc; i++)
    {
        memset (args, 0, 30);
        memset (value, 0, 30);
        strcpy (args, argv[i]);
        if (argv[i + 1] != NULL)
            strcpy (value, argv[i + 1]);
        lowerConversion (args);

        if (strcmp (args, "--bindaddr") == 0)
        {
            config_server.bindIp = inet_addr (NameToHost (value));
            if (config_server.bindIp == INADDR_NONE)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments --bindaddr = %s,use localhost instead",
                      value);
                config_server.bindIp = INADDR_ANY;
            }
            continue;
        } else if (strcmp (args, "--bindport") == 0)
        {
            config_server.bindPort = (unsigned short)
                    strtol (value, NULL, 10);
            if (config_server.bindPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments --bindport = %s,use default 9190 instead",
                      value);
                config_server.bindPort = 9190;
            }
            continue;
        } else if (strcmp (args, "--httpport") == 0)
        {
            config_server.httpPort = (unsigned short)
                    strtol (value, NULL, 10);
            if (config_server.httpPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments --bindport = %s,use default 8080 instead",
                      value);
                config_server.httpPort = 8080;
            }
            continue;
        } else if (strcmp (args, "--sslmode") == 0)
        {
            if (strcmp (value, "default") == 0)
                config_server.modeSSL = true;
            else if (strcmp (value, "disable") == 0)
                config_server.modeSSL = false;
            else
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default instead",
                      args, value);
                config_server.modeSSL = true;
            }
            continue;
        } else if (strcmp (args, "--cafile") == 0)
        {
            strcpy (config_server.caFile, value);
            continue;
        } else if (strcmp (args, "--servcert") == 0)
        {
            strcpy (config_server.servCert, value);
            continue;
        } else if (strcmp (args, "--servkey") == 0)
        {
            strcpy (config_server.servKey, value);
            continue;
        } else if (strcmp (args, "--pidfile") == 0)
        {
            strcpy (config_server.pidFile, value);
            continue;
        } else if (strcmp (args, "--daemon") == 0)
        {
            if (strcmp (value, "default") == 0)
                config_server.modeDaemon = true;
            else if (strcmp (value, "disable") == 0)
                config_server.modeDaemon = false;
            else
            {
                perr (true, LOG_NOTICE,
                      "Invalid Arguments --daemon = %s,use default instead",
                      value);
                config_server.modeDaemon = true;
            }
            continue;
        } else if (strcmp (args, "--sqlhost") == 0)
        {
            strcpy (config_server.sqlHost, value);
            continue;
        } else if (strcmp (args, "--sqlport") == 0)
        {
            config_server.sqlPort = (unsigned short)
                    strtol (value, NULL, 10);
            if (config_server.sqlPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments --bindport = %s,use default 3306 instead",
                      value);
                config_server.sqlPort = 3306;
            }
            continue;
        } else if (strcmp (args, "--sqluser") == 0)
        {
            strcpy (config_server.sqlUser, value);
            continue;
        } else if (strcmp (args, "--sqlpass") == 0)
        {
            strcpy (config_server.sqlPass, value);
            continue;
        } else if (strcmp (args, "--sqlname") == 0)
        {
            strcpy (config_server.sqlName, value);
            continue;
        }
//        else if (strcmp (args, "--clean-pid") == 0)
//        {
//            char * pid_file;
//            if (strcmp (config_server.pidFile, "default") == 0)
//                pid_file = PID_FILE_SERVER;
//            else if (strcmp (config_server.pidFile, "disable") == 0)
//                pid_file = NULL;
//            else pid_file = config_server.pidFile;
//            if (!checkRootPermission ("to delete PID_FILE"))
//                exit (-1);
//            if (pid_file != NULL)
//            {
//                printf ("%s will be deleted\n", pid_file);
//                unlink (pid_file);
//            } else printf ("PIDFILE is already disable\n");
//            exit (0);
//        } else if (strcmp (args, "--default-conf") == 0)
//        {
//            printf ("resetServ %s\n", CONF_FILE_SERVER);
//            unlink (CONF_FILE_SERVER);
//            defaultConfServ (CONF_FILE_SERVER);
//            exit (0);
//        } else if (strcmp (args, "--settings") == 0)
//        {
//            if (!checkRootPermission ("or it will open in read-only mode"
//                                      "(Press Any Key To Continue)"))
//                getchar ();
//            execlp ("vim", "vim", CONF_FILE_SERVER, NULL);
//            printf ("vim editor not available,use vi editor instead (3 secs...)\n");
//            sleep (3);
//            execlp ("vi", "vi", CONF_FILE_SERVER, NULL);
//            printf ("can not open conf file with vi too! "
//                    "please operate manually (3 secs...)\n");
//            sleep (3);
//            exit (-1);
//        } else if (strcmp (args, "--help") == 0)
//        {
//            printf ("Usage:  "
//                    "[--bindaddr] [ip / hostname]\t\t\t\tspecify server bind ip address\n\t"
//                    "[--bindport] [port]\t\t\t\t\tspecify server ip bind port\n\t"
//                    "[--httpport] [port]\t\t\t\t\tspecify server http port\n\t"
//                    "[--sslmode] [disable,default]\t\t\t\tspecify server ip bind port\n\t"
//                    "[--cafile] [filepath]\t\t\t\t\tspecify ca certificate file path\n\t"
//                    "[--servcert] [filepath]\t\t\t\t\tspecify server certificate file path\n\t"
//                    "[--servkey] [filepath]\t\t\t\t\tspecify server private key file path\n\t"
//                    "[--pidfile] [disable,default,\"file path\"]\t\tspecify pid file path\n\t"
//                    "[--daemon] [disable,default]\t\t\t\tspecify daemon mode\n\t"
//                    "[--sqlhost] [ip / hostname]\t\t\t\tspecify MySQL server host name\n\t"
//                    "[--sqlport] [port]\t\t\t\t\tspecify server MySQL port\n\t"
//                    "[--sqluser] [user]\t\t\t\t\tspecify server MySQL user\n\t"
//                    "[--sqlpass] [pass]\t\t\t\t\tspecify server MySQL pass\n\t"
//                    "[--sqlname] [name]\t\t\t\t\tspecify server MySQL DataBase Name\n\t"
//                    "[--clean-pid]\t\t\t\t\t\tDelete PID file\n\t"
//                    "[--default-conf]\t\t\t\t\tCreate(Overwrite) default configuration file\n\t"
//                    "[--settings]\t\t\t\t\t\tOpen(VIM editor) configuration file\n");
//            exit (0);
//        }
        else
        {
            if (args[0] != '-')
                continue;
            printf ("Unknown Parameter %s, "
                    "Use \"--help\" for help information\n", args);
            exit (-1);
        }
    }
    free (args);
    free (value);
    perr (true, LOG_INFO,
          "runTime Arguments Read Done! The current configuration is: BindAddr = %s, "
          "BindPort = %d, "
          "HttpPort = %d, "
          "SSLMode = %s, "
          "CAFILE = %s, "
          "SERVCERT = %s, "
          "SERVKEY = %s, "
          "PIDFILE = %s, "
          "DAEMON = %s, "
          "SQLHOST = %s, "
          "SQLPORT = %d, "
          "SQLUSER = %s, "
          "SQLPASS = %s, "
          "SQLNAME = %s ",
          inet_ntoa ((struct in_addr) {config_server.bindIp}),
          config_server.bindPort,
          config_server.httpPort,
          config_server.modeSSL ? "default" : "disable",
          config_server.caFile,
          config_server.servCert,
          config_server.servKey,
          config_server.pidFile,
          config_server.modeDaemon ? "default" : "disable",
          config_server.sqlHost,
          config_server.sqlPort,
          config_server.sqlUser,
          config_server.sqlPass,
          config_server.sqlName);
    errno = errno_save;
}

void runTimeArgsClnt (int argc, const char * argv[])
{
    int errno_save = errno;
    perr (true, LOG_INFO, "Reading runTime Arguments");
    char * args = calloc (1, 30);
    char * value = calloc (1, 30);
    for (int i = 1; i < argc; i++)
    {
        memset (args, 0, 30);
        memset (value, 0, 30);
        strcpy (args, argv[i]);
        if (argv[i + 1] != NULL)
            strcpy (value, argv[i + 1]);
        lowerConversion (args);

        if (strcmp (args, "--servaddr") == 0)
        {
            config_client.servIp = inet_addr (NameToHost (value));
            if (config_client.servIp == INADDR_NONE)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments --servaddr = %s,use localhost instead",
                      value);
                config_client.servIp = inet_addr ("127.0.0.1");
            }
            continue;
        } else if (strcmp (args, "--servport") == 0)
        {
            config_client.servPort = (unsigned short)
                    strtol (value, NULL, 10);
            if (config_client.servPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments --servport = %s,use default 9190 instead",
                      value);
                config_client.servPort = 9190;
            }
            continue;
        } else if (strcmp (args, "--interval") == 0)
        {
            config_client.interval = (int) strtol (value, NULL, 10);
            if (config_client.interval == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid Arguments = %s,use default 5 instead",
                      value);
                config_client.interval = 5;
            }
            continue;
        } else if (strcmp (args, "--cafile") == 0)
        {
            strcpy (config_client.caFile, value);
            if (strcmp (config_client.caFile, "default") == 0)
                config_client.sslMode = ssl_load_none;
            else if (strcmp (config_client.caFile, "disable") == 0)
                config_client.sslMode = ssl_disable;
            else config_client.sslMode = ssl_load_ca;
            continue;
        } else if (strcmp (args, "--pidfile") == 0)
        {
            strcpy (config_client.pidFile, value);
            continue;
        } else if (strcmp (args, "--daemon") == 0)
        {
            if (strcmp (value, "default") == 0)
                config_client.modeDaemon = true;
            else if (strcmp (value, "disable") == 0)
                config_client.modeDaemon = false;
            else
            {
                perr (true, LOG_NOTICE,
                      "Invalid Arguments --daemon = %s,use default instead",
                      value);
                config_client.modeDaemon = true;
            }
            continue;
        }
//        else if (strcmp (args, "--clean-pid") == 0)
//        {
//            char * pid_file;
//            if (strcmp (config_client.pidFile, "default") == 0)
//                pid_file = PID_FILE_CLIENT;
//            else if (strcmp (config_client.pidFile, "disable") == 0)
//                pid_file = NULL;
//            else pid_file = config_client.pidFile;
//            if (!checkRootPermission ("to delete PID_FILE"))
//                exit (-1);
//            if (pid_file != NULL)
//            {
//                printf ("%s will be deleted\n", pid_file);
//                unlink (pid_file);
//            } else printf ("PIDFILE is already disable\n");
//            exit (0);
//        } else if (strcmp (args, "--default-conf") == 0)
//        {
//            printf ("resetClnt %s\n", CONF_FILE_CLIENT);
//            unlink (CONF_FILE_CLIENT);
//            defaultConfClnt (CONF_FILE_CLIENT);
//            exit (0);
//        } else if (strcmp (args, "--settings") == 0)
//        {
//            if (!checkRootPermission ("or it will open in read-only mode"
//                                      "(Press Any Key To Continue)"))
//                getchar ();
//            execlp ("vim", "vim", CONF_FILE_CLIENT, NULL);
//            printf ("vim editor not available,use vi editor instead (3 secs...)\n");
//            sleep (3);
//            execlp ("vi", "vi", CONF_FILE_CLIENT, NULL);
//            printf ("can not open conf file with vi too! "
//                    "please operate manually (3 secs...)\n");
//            sleep (3);
//            exit (-1);
//        } else if (strcmp (args, "--help") == 0)
//        {
//            printf ("Usage:  "
//                    "[--servaddr] [ip / hostname]\t\t\t\tspecify server ip address\n\t"
//                    "[--servport] [port]\t\t\t\t\tspecify server ip port\n\t"
//                    "[--interval] [interval]\t\t\t\t\tspecify data collection interval\n\t"
//                    "[--cafile] [disable,default,\"file path\"]\t\tspecify ssl mode or ca path\n\t"
//                    "[--pidfile] [disable,default,\"file path\"]\t\tspecify pid file path\n\t"
//                    "[--daemon] [disable,default]\t\t\t\tspecify daemon mode\n\t"
//                    "[--clean-pid]\t\t\t\t\t\tDelete PID file\n\t"
//                    "[--default-conf]\t\t\t\t\tCreate(Overwrite) default configuration file\n\t"
//                    "[--settings]\t\t\t\t\t\tOpen(VIM editor) configuration file\n");
//            exit (0);
//        }
        else
        {
            if (args[0] != '-')
                continue;
            printf ("Unknown Parameter %s, "
                    "Use \"--help\" for help information\n", args);
            exit (-1);
        }
    }
    free (args);
    free (value);
    perr (true, LOG_INFO,
          "runTime Arguments Read Done! The current configuration is ServAddr = %s, "
          "ServPort = %d, "
          "Interval = %d, "
          "CAFILE = %s, "
          "PIDFILE = %s, "
          "DAEMON = %s ",
          inet_ntoa ((struct in_addr) {config_client.servIp}),
          config_client.servPort,
          config_client.interval,
          config_client.caFile,
          config_client.pidFile,
          config_client.modeDaemon ? "default" : "disable");
    errno = errno_save;
}

void checkPidFileServ (char * pid_file)
{
    if (pid_file == NULL)return;
    int errno_save = errno;
    if (access (pid_file, F_OK) == 0)
    {
        perr (true, LOG_ERR,
              "service already running! stopped. "
              "(If Not,Please type \"--clean-pid\" to remove %s)",
              pid_file);
        strcpy (config_server.pidFile, "disable");
        exitCleanupServ ();
    }
    pid_file_fd = open (pid_file, O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    if (pid_file_fd < 0)
    {
        perr (true, LOG_ERR, "PID pid_file can NOT be created");
        exitCleanupServ ();
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    fcntl (pid_file_fd, F_SETLK, & fl);  // 一般都是建议性锁,这取决于文件系统的实现

    char pid_string[16] = {0};
    sprintf (pid_string, "%ld", (long) getpid ());
    write (pid_file_fd, pid_string, strlen (pid_string));
    fflush (NULL);
    errno = errno_save;
    // we do not close pid_file_fd, it will close at exit
}

void checkPidFileClnt (char * pid_file)
{
    if (pid_file == NULL)return;
    int errno_save = errno;
    if (access (pid_file, F_OK) == 0)
    {
        perr (true, LOG_ERR,
              "service already running! stopped. "
              "(If Not,Please type \"--clean-pid\" to remove %s)",
              pid_file);
        exitCleanupClnt ();
    }
    pid_file_fd = open (pid_file, O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    if (pid_file_fd < 0)
    {
        perr (true, LOG_ERR, "PID pid_file can NOT be created");
        exitCleanupClnt ();
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    fcntl (pid_file_fd, F_SETLK, & fl);  // 一般都是建议性锁,这取决于文件系统的实现

    char pid_string[16] = {0};
    sprintf (pid_string, "%ld", (long) getpid ());
    write (pid_file_fd, pid_string, strlen (pid_string));
    fflush (NULL);
    errno = errno_save;
    // we do not close pid_file_fd, it will close at exit
}

void confToVarServ ()
{
    int errno_save = errno;
    perr (true, LOG_INFO, "Reading conf: %s", CONF_FILE_SERVER);
    if (!checkConf (CONF_FILE_SERVER))
        defaultConfServ (CONF_FILE_SERVER),
                exitCleanupServ ();
    PLinkNode conf = readConf (CONF_FILE_SERVER);
    checkRead (conf);

    KeyValuePair e[LengthOfLinkList (conf)];
    int len = ListToArry (conf, e);
    DestroyLinkList (& conf);


    for (int i = 0; i < len; i++)
    {
        if (strcmp (e[i].name, "BINDADDR") == 0)
        {
            config_server.bindIp = inet_addr (NameToHost (e[i].value));
            if (config_server.bindIp == INADDR_NONE)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use localhost instead",
                      e[i].name, e[i].value);
                config_server.bindIp = INADDR_ANY;
            }
            continue;
        } else if (strcmp (e[i].name, "BINDPORT") == 0)
        {
            config_server.bindPort = (unsigned short)
                    strtol (e[i].value, NULL, 10);
            if (config_server.bindPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default 9190 instead",
                      e[i].name, e[i].value);
                config_server.bindPort = 9190;
            }
            continue;
        } else if (strcmp (e[i].name, "HTTPPORT") == 0)
        {
            config_server.httpPort = (unsigned short)
                    strtol (e[i].value, NULL, 10);
            if (config_server.httpPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default 8080 instead",
                      e[i].name, e[i].value);
                config_server.httpPort = 8080;
            }
            continue;
        } else if (strcmp (e[i].name, "SSLMODE") == 0)
        {
            if (strcmp (e[i].value, "default") == 0)
                config_server.modeSSL = true;
            else if (strcmp (e[i].value, "disable") == 0)
                config_server.modeSSL = false;
            else
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default instead",
                      e[i].name, e[i].value);
                config_server.modeSSL = true;
            }
            continue;
        } else if (strcmp (e[i].name, "CAFILE") == 0)
        {
            strcpy (config_server.caFile, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "SERVCERT") == 0)
        {
            strcpy (config_server.servCert, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "SERVKEY") == 0)
        {
            strcpy (config_server.servKey, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "PIDFILE") == 0)
        {
            strcpy (config_server.pidFile, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "DAEMON") == 0)
        {
            if (strcmp (e[i].value, "default") == 0)
                config_server.modeDaemon = true;
            else if (strcmp (e[i].value, "disable") == 0)
                config_server.modeDaemon = false;
            else
            {
                perr (true, LOG_NOTICE,
                      "Invalid value option %s=%s,use default instead",
                      e[i].name, e[i].value);
                config_server.modeDaemon = true;
            }
            continue;
        } else if (strcmp (e[i].name, "SQLHOST") == 0)
        {
            strcpy (config_server.sqlHost, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "SQLPORT") == 0)
        {
            config_server.sqlPort = (unsigned short)
                    strtol (e[i].value, NULL, 10);
            if (config_server.sqlPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default 3306 instead",
                      e[i].name, e[i].value);
                config_server.sqlPort = 3306;
            }
            continue;
        } else if (strcmp (e[i].name, "SQLUSER") == 0)
        {
            strcpy (config_server.sqlUser, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "SQLPASS") == 0)
        {
            strcpy (config_server.sqlPass, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "SQLNAME") == 0)
        {
            strcpy (config_server.sqlName, e[i].value);
            continue;
        } else
            perr (true, LOG_NOTICE,
                  "Unknown configure option %s=%s", e[i].name, e[i].value);
    }
    perr (true, LOG_INFO,
          "config Read Done! BindAddr = %s, "
          "BindPort = %d, "
          "HttpPort = %d, "
          "SSLMode = %s, "
          "CAFILE = %s, "
          "SERVCERT = %s, "
          "SERVKEY = %s, "
          "PIDFILE = %s, "
          "DAEMON = %s, "
          "SQLHOST = %s, "
          "SQLPORT = %d, "
          "SQLUSER = %s, "
          "SQLPASS = %s, "
          "SQLNAME = %s ",
          inet_ntoa ((struct in_addr) {config_server.bindIp}),
          config_server.bindPort,
          config_server.httpPort,
          config_server.modeSSL ? "default" : "disable",
          config_server.caFile,
          config_server.servCert,
          config_server.servKey,
          config_server.pidFile,
          config_server.modeDaemon ? "default" : "disable",
          config_server.sqlHost,
          config_server.sqlPort,
          config_server.sqlUser,
          config_server.sqlPass,
          config_server.sqlName);
    errno = errno_save;
}

void confToVarClnt ()
{
    int errno_save = errno;
    perr (true, LOG_INFO, "Reading conf: %s", CONF_FILE_CLIENT);
    if (!checkConf (CONF_FILE_CLIENT))
        defaultConfClnt (CONF_FILE_CLIENT),
                exitCleanupClnt ();
    PLinkNode conf = readConf (CONF_FILE_CLIENT);
    checkRead (conf);

    KeyValuePair e[LengthOfLinkList (conf)];
    int len = ListToArry (conf, e);
    DestroyLinkList (& conf);


    for (int i = 0; i < len; i++)
    {
        if (strcmp (e[i].name, "SERVADDR") == 0)
        {
            config_client.servIp = inet_addr (NameToHost (e[i].value));
            if (config_client.servIp == INADDR_NONE)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use localhost instead",
                      e[i].name, e[i].value);
                config_client.servIp = inet_addr ("127.0.0.1");
            }
            continue;
        } else if (strcmp (e[i].name, "SERVPORT") == 0)
        {
            config_client.servPort = (unsigned short)
                    strtol (e[i].value, NULL, 10);
            if (config_client.servPort == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default 9190 instead",
                      e[i].name, e[i].value);
                config_client.servPort = 9190;
            }
            continue;
        } else if (strcmp (e[i].name, "INTERVAL") == 0)
        {
            config_client.interval = (int) strtol (e[i].value, NULL, 10);
            if (config_client.interval == 0)
            {
                perr (true, LOG_ERR,
                      "Invalid value option %s=%s,use default 5 instead",
                      e[i].name, e[i].value);
                config_client.interval = 5;
            }
            continue;
        } else if (strcmp (e[i].name, "CAFILE") == 0)
        {
            strcpy (config_client.caFile, e[i].value);
            if (strcmp (config_client.caFile, "default") == 0)
                config_client.sslMode = ssl_load_none;
            else if (strcmp (config_client.caFile, "disable") == 0)
                config_client.sslMode = ssl_disable;
            else config_client.sslMode = ssl_load_ca;
            continue;
        } else if (strcmp (e[i].name, "PIDFILE") == 0)
        {
            strcpy (config_client.pidFile, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "DAEMON") == 0)
        {
            if (strcmp (e[i].value, "default") == 0)
                config_client.modeDaemon = true;
            else if (strcmp (e[i].value, "disable") == 0)
                config_client.modeDaemon = false;
            else
            {
                perr (true, LOG_NOTICE,
                      "Invalid value option %s=%s,use default instead",
                      e[i].name, e[i].value);
                config_client.modeDaemon = true;
            }
            continue;
        } else
            perr (true, LOG_NOTICE,
                  "Unknown configure option %s=%s", e[i].name, e[i].value);
    }
    perr (true, LOG_INFO,
          "config Read Done! ServAddr = %s, "
          "ServPort = %d, "
          "Interval = %d, "
          "CAFILE = %s, "
          "PIDFILE = %s, "
          "DAEMON = %s ",
          inet_ntoa ((struct in_addr) {config_client.servIp}),
          config_client.servPort,
          config_client.interval,
          config_client.caFile,
          config_client.pidFile,
          config_client.modeDaemon ? "default" : "disable");
    errno = errno_save;
}

void exitCleanupServ ()
{
    perr (true, LOG_INFO,
          "Service Will Exit After Cleaning-up (3secs...)");
    sleep (3);


    char * pid_file;
    if (strcmp (config_server.pidFile, "default") == 0)
        pid_file = PID_FILE_SERVER;
    else if (strcmp (config_server.pidFile, "disable") == 0)
        pid_file = NULL;
    else pid_file = config_server.pidFile;

    if (pid_file_fd != -1)
        close (pid_file_fd);
    if (pid_file != NULL)
    {
        if (access (pid_file, F_OK) == 0)
            unlink (pid_file);
    }
    fflush (NULL);

    sql_pool_destroy (sql_pool_accept_raspi);
    thread_pool_destroy (thread_pool_accept_raspi);
    thread_pool_destroy (thread_pool_accept_http);
    hash_map_destroy (hash_map_http);
    hash_map_destroy (hash_map_raspi);
    if (web_html_buf != NULL)
        munmap (web_html_buf, web_html_size);
    if (web_html_bg_png_buf != NULL)
        munmap (web_html_bg_png_buf, web_html_bg_png_size);


    mysql_lib (false);
    exit (0);
}

void exitCleanupClnt ()
{
    perr (true, LOG_INFO,
          "Service Will Exit After Cleaning-up (3secs...)");

    char * pid_file;
    if (strcmp (config_client.pidFile, "default") == 0)
        pid_file = PID_FILE_CLIENT;
    else if (strcmp (config_client.pidFile, "disable") == 0)
        pid_file = NULL;
    else pid_file = config_client.pidFile;

    if (pid_file_fd != -1)
        close (pid_file_fd);
    if (pid_file != NULL)
    {
        if (access (pid_file, F_OK) == 0)
            unlink (pid_file);
    }
    fflush (NULL);
    exit (0);
}

void resetServ ()
{
    perr (true, LOG_INFO,
          "Service reset (3 secs)");
    sleep (3);


    defaultConfOptServ (& config_server);
    siglongjmp (jmp_server_rest, 1);
}

void resetClnt ()
{
    perr (true, LOG_INFO,
          "Service reset (3 secs)");
    sleep (3);


    defaultConfOptClnt (& config_client);
    siglongjmp (jmp_client_rest, 1);
}

bool checkRootPermission (const char * require_reason)
{
    int errno_save = errno;
    bool flag = (geteuid () == 0);
    perr (!flag, LOG_NOTICE,
          "root permission are required: %s", require_reason);
    errno = errno_save;
    return flag;
}

void sendFINtoServ ()
{
    if (raspi_connect_server.sslEnable)
        SSL_write (raspi_connect_server.ssl_fd, "FIN", 4);
    else write (raspi_connect_server.fd, "FIN", 4);
}

void downLightsCloseServ ()
{
    if (raspi_connect_server.sslEnable && raspi_connect_server.ssl_fd != NULL)
    {
        SSL_shutdown (raspi_connect_server.ssl_fd);
        SSL_free (raspi_connect_server.ssl_fd);
        SSL_CTX_free (raspi_connect_server.ssl_ctx);
    }
    close (raspi_connect_server.fd), raspi_connect_server.fd = -1;
    turn_off_led (LED_GRE);
    turn_off_led (LED_RED);
    turn_off_led (LED_YEL);
}