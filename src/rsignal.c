//
// Created by Einc on 2023/01/29.
//

#include "rsignal.h"
#include "head.h"
#include "load_clean.h"
#include "global.h"
#include "log.h"

void sigHandlerServ (int signo, siginfo_t * siginfo, void * context)
{

}

void sigHandlerClnt (int signo, siginfo_t * siginfo, void * context)
{
    SIGALRM, SIGHUP, SIGPIPE;  // reset
    SIGINT, SIGQUIT, SIGTERM, SIGABRT, SIGSEGV; // exit
    SIGTSTP, SIGTTIN, SIGTTOU, SIGURG;  // undefined

    if (signo == SIGINT || signo == SIGQUIT || signo == SIGABRT)
        perr (true, LOG_NOTICE,
              "%s received, Stopping", strsignal (signo)),
//                sendFINtoServ (),
                downLightsCloseServ (),
                exitCleanupClnt ();

    if (signo == SIGTERM)  // server send FIN
    {
        /**
         * 客户端对SIGTERM的用法:
         * 客户端在向服务器发送一个数据包后要求在10秒内接收到服务器响应包(人为定义)
         * 当响应内容为"FIN"时向自身发送SIGTERM, 表示服务器主动断开连接 */
        perr (true, LOG_NOTICE,
              "SIGTERM received, server send FIN, Stopping");

        downLightsCloseServ ();
        exitCleanupClnt ();
    }

    if (signo == SIGSEGV)  // mem err
    {
        psiginfo (siginfo, "Segmentation Fault: ");
        perr (true, LOG_ERR,
              "SIGSEGV received, Exiting Immediately without cleaning");
        _exit (127);
    }

    if (signo == SIGALRM)  // server respond time out (bad network)
    {
        /**
         * 客户端对SIGALRM的用法:
         * 客户端在向服务器发送一个数据包后要求在10秒内接收到服务器响应包(人为定义)
         * 超时会被alarm(10)发送SIGALRM信号,这时我们认为是网络不通畅所导致的
         * 因此我们定义发生该信号时重置客户端关闭连接然后重新尝试建立连接 */
        perr (true, LOG_WARNING,
              "SIGALRM received, Server response timeout, resetClnt.");


        downLightsCloseServ ();
        resetClnt ();
    }
    if (signo == SIGHUP)  // reload or quit
    {
//        sendFINtoServ (),
        downLightsCloseServ ();
        /**
         * 客户端对SIGHUP的用法:
         * 当进程处于前台,占用控制终端时SIGHUP表示用户从控制终端断开连接
         * 由于以非守护进程模式运行程序时被默认为用户在进行调试,所以这里终止程序
         * 当程序以守护进程模式运行在后台时, 认为该信号目的是重读配置文件 */
        if (tcgetpgrp (STDIN_FILENO) == getpgrp () || config_client.modeDaemon == false)
            perr (true, LOG_NOTICE,
                  "SIGHUP received, terminal disconnect, Stopping"),
                    exitCleanupClnt ();
        else
            perr (true, LOG_INFO,
                  "SIGHUP received, configure file reload, resetClnt."),
                    resetClnt ();
    }


    if (signo == SIGPIPE)  // server close
    {
        perr (true, LOG_ERR,
              "SIGPIPE received, cant not write data to server");

        downLightsCloseServ ();
        resetClnt ();
    }
    psignal (signo, "other");
}

void sigRegisterServ ()
{

}

void sigRegisterClnt ()
{
    // define vars
    struct sigaction act;

    memset (& act, 0, sizeof (struct sigaction));
    sigfillset (& act.sa_mask);
    act.sa_flags = SA_INTERRUPT | SA_SIGINFO;
    act.sa_sigaction = sigHandlerClnt;

    // 各种退出信号, 清理后退出
    sigaction (SIGINT, & act, NULL);
    sigaction (SIGQUIT, & act, NULL);
    sigaction (SIGTERM, & act, NULL);
    sigaction (SIGABRT, & act, NULL);
    sigaction (SIGSEGV, & act, NULL);

    // 其他信号, 允许被退出信号打断
    sigdelset (& act.sa_mask, SIGINT);
    sigdelset (& act.sa_mask, SIGQUIT);
    sigdelset (& act.sa_mask, SIGTERM);
    sigdelset (& act.sa_mask, SIGABRT);
    sigdelset (& act.sa_mask, SIGSEGV);
    sigaction (SIGALRM, & act, NULL);
    sigaction (SIGHUP, & act, NULL);
    sigaction (SIGPIPE, & act, NULL);

    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction (SIGTSTP, & act, NULL);
    sigaction (SIGTTIN, & act, NULL);
    sigaction (SIGTTOU, & act, NULL);
    sigaction (SIGURG, & act, NULL);
}
