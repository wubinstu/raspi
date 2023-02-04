
#include "client_run.h"
#include "global.h"
#include "head.h"
#include "load_clean.h"
#include "log.h"
#include "raspi_drive.h"
#include "rsignal.h"
#include "rssl.h"
#include "types.h"

// TODO
// 1. [DONE] enable ssl transmission
// 2. [IGNORE] set socket to no blocking mode
// 3. [DONE] Recover in case of network error or server send disconnect (alram read)
// 4. [IGNORE] fd manage
// 5. [DONE] Supports the use of domain names instead of server IPv4 addresses (DNS)
// 6. [DONE] Systemd Unit service file
// 7. [DONE] Create a independent thread for data collection
// 8. [DONE] flash led, Set different meanings for LED lights
// 9. [IGNORE] thread pause resume
// 10. [DONE] compact exit
// 11. [DONE] deal with runtime args
// 12. [WAITING] Create a Controler Program
// 13. [DONE] package configuration file's global variable to structural

// problems:
// 1. [SOLVED] longjmp thread undefined
// 2. [WAITING] segmentation fault when exit


// TEST
// signal int,term,quit,alarm,hup
// thread return value check
// mode strict -> single file
// pid file deep check

int main (int argc, const char *argv[])
{
//    atexit (exitCleanupClnt);
    initPi ();  // init wiringPi lib
    set_serv_clnt (client);  // we are client
    dealWithArgsClnt (argc, argv);  // Check Runtime parameters

    if (mode_strict)
    {
        // Check whether you have root privileges
        if (!check_permission ("To create pid file"))
            return -1;
        // Ensure that only one program runs at the same time according to the PID file
        checkPidFile ();
    }
    // DEBUGMODE优先级高于命令行参数 arg
    mode_daemon = !DEBUGMODE;
    if (mode_daemon)
        daemonize (PROJECT_CLIENT_NAME);
    // Set up an archive point
    int jmp_rtn = sigsetjmp(jmp_client_rest, true);
    // Register signal processing function
    sig_register_client ();
    // Read configuration file to global variable
    conf2var ();

    if (mode_strict)
    {
        int rtn_flag;
        ctx = initSSL ();
        if (ctx == NULL) exitCleanupClnt ();
        if (mode_ssl_client == client_only_ca)
        {
            rtn_flag = loadCA (ctx, file_client_config.CAfile);
            if (!rtn_flag) exitCleanupClnt ();
        }
        if (mode_ssl_client == client_with_cert_key)
        {
            rtn_flag = loadCert (ctx, file_client_config.UCert);
            if (!rtn_flag) exitCleanupClnt ();

            rtn_flag = loadKey (ctx, file_client_config.UKey);
            if (!rtn_flag) exitCleanupClnt ();

            rtn_flag = checkKey (ctx);
            if (!rtn_flag) exitCleanupClnt ();
        }
    }

    // Trying to connect to the server
    int con_rtn = tryConnect (LED_RED);
    if (con_rtn == -1)
    {
        perr_d (true, LOG_ERR,
                "Maximum number of reconnections exceeded");
        exitCleanupClnt ();
    }


    if (mode_strict)
    {
        ssl_serv_fd = SSL_fd (ctx, serv_fd);
        if (ssl_serv_fd == NULL) exitCleanupClnt ();
        int SSL_handShake = SSL_connect (ssl_serv_fd);
        if (SSL_handShake == -1)
        {
            perr_d (true, LOG_ERR,
                    "Server authentication failed, connection disconnected");
            exitCleanupClnt ();
        }
        if (mode_ssl_client == client_with_cert_key)
        {
            printf ("Self Cert:\n");
            showSelfCert (ssl_serv_fd);
        }
        printf ("Peer Cert:\n");
        showPeerCert (ssl_serv_fd);
    }

    if (jmp_rtn != RESET)
    {
        int arg_led = LED_YEL;
        pthread_mutexattr_t mutex_raspi_data_attr;
        pthread_mutexattr_init (&mutex_raspi_data_attr);
//        pthread_mutexattr_settype (&mutex_raspi_data_attr, PTHREAD_MUTEX_ERRORCHECK);
//        pthread_mutexattr_setpshared (&mutex_raspi_data_attr, PTHREAD_PROCESS_PRIVATE);
        pthread_mutexattr_setrobust (&mutex_raspi_data_attr, PTHREAD_MUTEX_ROBUST);
        pthread_mutex_destroy (&mutex_monit_data);
        pthread_mutex_init (&mutex_monit_data, &mutex_raspi_data_attr);

        pthread_create (&thread_client_data_checker, NULL, checkMonit, NULL);
        pthread_create (&thread_client_data_sender, NULL, sendData, (void *) &arg_led);
    }

    pause ();
    pause ();
}