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


void daemonize (const char *cmd)
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
    if (getrlimit (RLIMIT_NOFILE, &rl) < 0)
        perr_d (true, LOG_NOTICE,
                "%s[daemonize]: can not get file limit", cmd);

    /**
     * Become a session leader to lose controlling TTY
     * fork 后只保留子进程,这样可以脱离控制终端,并且为创建会话做准备 */
    if ((pid = fork ()) < 0)
        perr_d (true, LOG_ERR,
                "%s[daemonize]: cant not fork", cmd);
    else if (pid != 0)
        exit (0);

    setsid ();

    /**
     * 约定成俗的规则,守护进程一般不需要处理SIGHUP, 当然有些软件使用该信号来"重启(配置文件修改时)"软件 */
    sa.sa_handler = SIG_IGN;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction (SIGHUP, &sa, NULL) < 0)
        perr_d (true, LOG_NOTICE,
                "%s[daemonize]: can not ignore SIGHUP", cmd);

    /**
     * Ensure future opens won't allocate controlling TTYs.
     * 再一次 fork, 还是只保留子进程,这样子进程不是会话首进程,那么它永远也无法获取控制终端 */
    if ((pid = fork ()) < 0)
        perr_d (true, LOG_ERR,
                "%s[daemonize]: can not fork", cmd);
    else if (pid != 0)
        exit (0);

    /**
     * Change the current working directory to the root so
     * we won't prevent file system from being unmounted
     * 将当前工作目录更改为根目录,这样就不会阻止卸载文件系统 */
    if (chdir ("/") < 0)
        perr_d (true, LOG_ERR,
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
        perr_d (true, LOG_NOTICE,
                "%s[daemonize]: unexpected file descriptors %d %d %d",
                cmd, fd0, fd1, fd2);
        exit (1);
    }
    perr_d (true, LOG_INFO,
            "%s[daemonize]: process successfully entered daemon mode", cmd);
}

void dealWithArgsServ (int argc, const char *argv[])
{}

void dealWithArgsClnt (int argc, const char *argv[])
{
    errno = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp (argv[i], "--daemon") == 0)
        {
            mode_daemon = true;
            continue;
        } else if (strcmp (argv[i], "--strict") == 0)
        {
            mode_strict = true;
            continue;
        } else if (strcmp (argv[i], "--clean") == 0)
        {
            if (!check_permission ("to delete PID_FILE"))
                exit (-1);
            printf ("%s will be deleted\n", PID_FILE_CLIENT);
            unlink (PID_FILE_CLIENT);
            exit (0);
        } else if (strcmp (argv[i], "--default-conf") == 0)
        {
            printf ("resetClnt %s\n", CONF_FILE_CLIENT);
            unlink (CONF_FILE_CLIENT);
            defaultConfClnt (CONF_FILE_CLIENT);
            exit (0);
        } else if (strcmp (argv[i], "--settings") == 0)
        {
            if (!check_permission
                    ("or it will open in read-only mode(Press Any Key To Continue)"))
                getchar ();
            execlp ("vim", "vim", CONF_FILE_CLIENT, NULL);
            printf ("vim editor not available,use vi editor instead (3 secs...)\n");
            sleep (3);
            execlp ("vi", "vi", CONF_FILE_CLIENT, NULL);
            printf ("can not open conf file with vi too! "
                    "please operate manually (3 secs...)\n");
            sleep (3);
            exit (-1);
        } else if (strcmp (argv[i], "--help") == 0)
        {
            printf ("Usage:  "
                    "[--daemon] \t Runing in daemon mode\n\t"
                    "[--strict] \t Entering strict mode\n\t"
                    "[--clean] \t Delete PID file\n\t"
                    "[--default-conf] Create(Overwrite) default configuration file\n\t"
                    "[--settings] \t Open(VIM editor) configuration file\n");
            exit (0);
        } else
        {
            printf ("Unknown Parameter, Use \"--help\" for help information\n");
            exit (-1);
        }
    }
    printf ("daemonMode = %s,strictMode = %s\n",
            mode_daemon ? "true" : "false", mode_strict ? "true" : "false");
}

bool check_permission (const char *require_reason)
{
    errno = 0;
    bool flag = (geteuid () == 0);
    perr_d (!flag, LOG_NOTICE,
            "root permission are required: %s", require_reason);
    return flag;
}

void checkPidFile ()
{
    errno = 0;
    char *pid_file;
    if (mode_serv_clnt == server)pid_file = PID_FILE_SERVER;
    if (mode_serv_clnt == client)pid_file = PID_FILE_CLIENT;
    if (access (PID_FILE_CLIENT, F_OK) == 0)
    {
        perr_d (true, LOG_ERR,
                "service already running! stopped. "
                "(If Not,Please type \"--clean\" to remove %s)",
                PID_FILE_CLIENT);
        exitCleanupClnt ();
    }
    pid_file_fd = open (pid_file, O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    if (pid_file_fd < 0)
    {
        perr_d (true, LOG_ERR, "PID pid_file can NOT be created");
        exitCleanupClnt ();
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    fcntl (pid_file_fd, F_SETLK, &fl);

    char pid_string[16] = {0};
    sprintf (pid_string, "%ld", (long) getpid ());
    write (pid_file_fd, pid_string, strlen (pid_string));
    fflush (NULL);
    // we do not close pid_file_fd, it will close at exit
}

void conf2var ()
{
    errno = 0;
    perr_d (true, LOG_INFO, "Reading conf: %s", CONF_FILE_CLIENT);
    if (!checkconf (CONF_FILE_CLIENT))
        exitCleanupClnt ();
    LNode conf = readconf (CONF_FILE_CLIENT);
    checkread (conf);

    KeyValuePair e[LengthOfLinkList (conf)];
    int len = ListToArry (conf, e);
    DestoryLinkList (&conf);


    for (int i = 0; i < len; i++)
    {
        if (strcmp (e[i].name, "SERVADDR") == 0)
        {
            file_client_config.serv_ip = inet_addr (NameToHost_d (e[i].value));
            continue;
        } else if (strcmp (e[i].name, "SERVPORT") == 0)
        {
            file_client_config.serv_port = (unsigned short)
                    strtol (e[i].value, NULL, 10);
            continue;
        } else if (strcmp (e[i].name, "INTERVAL") == 0)
        {
            file_client_config.interval = (int) strtol (e[i].value, NULL, 10);
            continue;
        } else if (strcmp (e[i].name, "FRECTIME") == 0)
        {
            file_client_config.frectime = (int) strtol (e[i].value, NULL, 10);
            continue;
        } else if (strcmp (e[i].name, "FRECATPS") == 0)
        {
            file_client_config.frecatps = (int) strtol (e[i].value, NULL, 10);
            continue;
        } else if (strcmp (e[i].name, "CAFILE") == 0)
        {
            strcpy (file_client_config.CAfile, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "SSLMODE") == 0)
        {
            int sslmode = (int) strtol (e[i].value, NULL, 10);
            if (sslmode == 0 || sslmode == 1 || sslmode == 2)
                mode_ssl_client = sslmode;
            else
                perr_d (true, LOG_NOTICE,
                        "Unknown configure option %s=%s", e[i].name, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "UCERT") == 0)
        {
            strcpy (file_client_config.UCert, e[i].value);
            continue;
        } else if (strcmp (e[i].name, "UKEY") == 0)
        {
            strcpy (file_client_config.UKey, e[i].value);
            continue;
        } else
            perr_d (true, LOG_NOTICE,
                    "Unknown configure option %s=%s", e[i].name, e[i].value);
    }
    if (file_client_config.serv_ip == -1)
    {
        perr_d (true, LOG_ERR,
                "The server IP address could not be read correctly "
                "from the configuration file");
        exitCleanupClnt ();
    }
    if (file_client_config.serv_port == 0)
    {
        perr_d (true, LOG_ERR,
                "The server port could not be read correctly "
                "from the configuration file");
        exitCleanupClnt ();
    }
    if (mode_ssl_client == client_only_ca && strlen (file_client_config.CAfile) == 0)
    {
        perr_d (true, LOG_ERR, "CA file not specified");
        exitCleanupClnt ();
    }
    if (mode_ssl_client == client_with_cert_key &&
        (strlen (file_client_config.UCert) == 0 || strlen (file_client_config.UKey) == 0))
    {
        perr_d (true, LOG_ERR, "User cert or key not specified");
        exitCleanupClnt ();
    }
    perr_d (true, LOG_INFO, "mode_ssl_client = %d", mode_ssl_client);
}

void exitCleanupClnt ()
{
    errno = 0;
//    printf ("\n");
    perr_d (true, LOG_INFO, "Service Will Exit After Cleanigup");

    if (thread_client_data_checker != 0)
        pthread_cancel (thread_client_data_checker);
    if (thread_client_data_sender != 0)
        pthread_cancel (thread_client_data_sender);

    printf ("sleep 3 sec...\n");
    sleep (3);

    if (pid_file_fd != -1)
        close (pid_file_fd);
    if (access (PID_FILE_CLIENT, F_OK) == 0)
        unlink (PID_FILE_CLIENT);
    fflush (NULL);
    exit (0);
}

void resetClnt ()
{
    int rtn;
    printf ("a\n");

    pthread_detach (thread_client_data_checker);
    pthread_detach (thread_client_data_sender);
    if (thread_client_data_checker != 0)
    {
        rtn = pthread_cancel (thread_client_data_checker);
        if (rtn == 0)
            printf ("1.data checker cancel sended!\n");
        else if (rtn == ESRCH)
            printf ("1.data checker not found!\n");

        sleep (10);
        printf ("thread sig = %d", pthread_kill (thread_client_data_checker, 0));
//        pthread_join (thread_client_data_checker, NULL);

        rtn = pthread_cancel (thread_client_data_checker);
        if (rtn == 0)
            printf ("2.data checker cancel sended!\n");
        else if (rtn == ESRCH)
            printf ("2.data checker not found!\n");
    }
    printf ("b\n");
    if (thread_client_data_sender != 0)
    {
        rtn = pthread_cancel (thread_client_data_sender);
        if (rtn == 0)
            printf ("3.data sender cancel sended!\n");
        else if (rtn == ESRCH)
            printf ("3.data sender not found!\n");

//        pthread_join (thread_client_data_sender, NULL);

        rtn = pthread_cancel (thread_client_data_sender);
        if (rtn == 0)
            printf ("4.data sender cancel sended!\n");
        else if (rtn == ESRCH)
            printf ("4.data sender not found!\n");
    }
    thread_client_data_checker = 0;
    thread_client_data_sender = 0;

    printf ("sleep 3 sec...\n");
    sleep (3);


    printf ("c\n");
    default_confOpt (&file_client_config);

    printf ("d\n");
    siglongjmp (jmp_client_rest, RESET);
}
