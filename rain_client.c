
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
// 3. [DONE] Recover in case of network error or server send disconnect (alarm read)
// 4. [IGNORE] fd manage
// 5. [DONE] Supports the use of domain names instead of server IPv4 addresses (DNS)
// 6. [DONE] Systemd Unit service file
// 7. [DONE] Create a independent thread for data collection
// 8. [DONE] flash led, Set different meanings for LED lights
// 9. [IGNORE] thread pause resume
// 10. [DONE] compact exit
// 11. [DONE] deal with runtime args
// 12. [WAITING] Create a Controller Program
// 13. [DONE] package configuration file's global variable to structural

// problems:
// 1. [SOLVED] longjmp thread undefined
// 2. [WAITING] segmentation fault when exit


// TEST
// signal int,term,quit,alarm,hup
// thread return value check
// mode strict -> single file
// pid file deep check

int main (int argc, const char * argv[])
{
//    atexit (exitCleanupClnt);
    initPi ();  // init wiringPi lib
    setServClnt (client);  // we are client
    confToVarClnt ();  // read conf file
    runTimeArgsClnt (argc, argv);  // Check Runtime parameters


    // Check whether you have root privileges
    if (!checkRootPermission ("To create pid file"))
        return -1;
    // Ensure that only one program runs at the same time according to the PID file
    if (strcmp (config_client.pidFile, "default") == 0)
        checkPidFileServ (PID_FILE_CLIENT);
    else if (strcmp (config_client.pidFile, "disable") == 0)
        checkPidFileServ (NULL);
    else checkPidFileServ (config_client.pidFile);

    if (strcmp (config_client.caFile, "default") == 0)
        mode_ssl_client = client_ssl_empty;
    else if (strcmp (config_client.caFile, "disable") == 0)
        mode_ssl_client = client_ssl_disable;
    else mode_ssl_client = client_ssl_only_ca;

    if (config_client.modeDaemon)
        daemonize (PROJECT_CLIENT_NAME);
    // Set up an archive point
    if (sigsetjmp(jmp_client_rest, true) != 0)
        confToVarClnt ();
    // Register signal processing function
    sigRegisterClient ();

    // Trying to connect to the server
    if (tryConnect (LED_RED) == -1)
    {
        perr (true, LOG_ERR,
              "Maximum number of reconnections exceeded");
        exitCleanupClnt ();
    }


    if (mode_ssl_client > client_ssl_disable)
    {
        int rtn_flag;
        ctx_client_to_server = initSSL (client);
        if (ctx_client_to_server == NULL) exitCleanupClnt ();
        if (mode_ssl_client == client_ssl_only_ca)
        {
            rtn_flag = loadCA (ctx_client_to_server, config_client.caFile);
            if (!rtn_flag) exitCleanupClnt ();
        }

        ssl_serv_fd = SSL_fd (ctx_client_to_server, serv_fd);
        if (ssl_serv_fd == NULL) exitCleanupClnt ();
        setClientVerify (ctx_client_to_server);

        if (SSL_connect (ssl_serv_fd) == -1)
        {
            perr (true, LOG_ERR,
                  "Server authentication failed, connection disconnected");
            exitCleanupClnt ();
        }
        printf ("Peer Cert:\n");
        showPeerCert (ssl_serv_fd);
    }


    while (true)
    {
        checkMonit ();
        sendData (LED_YEL);
        sleep (config_client.interval);

//        break;
    }
}