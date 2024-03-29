//
// Created by Einc on 2022/4/4.
//

#include "arr.h"
#include "conf.h"
#include "filed.h"
#include "global.h"
#include "log.h"
#include "load_clean.h"
#include "socket_fd.h"
#include "rssl.h"
#include "run_client.h"
#include "raspi_drive.h"
#include "types.h"

int readServer (void * buf, int size)
{
    int read_size;
    if (raspi_connect_server.sslEnable)
        read_size = SSL_read (raspi_connect_server.ssl_fd, buf, size);
    else read_size = (int) read (raspi_connect_server.fd, buf, size);
    return read_size;
}

int writeServer (void * buf, int size)
{
    int write_size;
    if (raspi_connect_server.sslEnable)
        write_size = SSL_write (raspi_connect_server.ssl_fd, buf, size);
    else write_size = (int) write (raspi_connect_server.fd, buf, size);
    return write_size;
}


int tryConnect (int led)
{
    int errno_save = errno, num_sec;
    turn_off_led (led);
    memset (& raspi_connect_server, 0, sizeof (raspi_connect_server));
    raspi_connect_server.addr.sin_addr.s_addr = config_client.servIp;
    raspi_connect_server.addr.sin_port = htons (config_client.servPort);
    raspi_connect_server.addr.sin_family = AF_INET;
    raspi_connect_server.addr_len = sizeof (raspi_connect_server.addr);
    if (config_client.sslMode > ssl_disable)
        raspi_connect_server.sslEnable = true;

    for (num_sec = 1; num_sec <= MAXSLEEP; num_sec <<= 1)
    {
        perr (true, LOG_INFO,
              "Trying to connect to the server...");
        raspi_connect_server.fd = connectServ (& raspi_connect_server);
        if (raspi_connect_server.fd == -1)
        {
            perr (true, LOG_NOTICE,
                  "Miss Connection, Retry in %d secs",
                  num_sec);
            sleep (num_sec);
        } else if (raspi_connect_server.fd > 0)
            break;
    }


    perr (!setSockBufSize (raspi_connect_server.fd,
                           SOCK_SND_BUF_SIZE,
                           SOCK_RCV_BUF_SIZE), LOG_WARNING,
          "function setSockBufSize return false when called tryConnect");


    errno = errno_save;
    if (num_sec > MAXSLEEP)return -1;

    errno_save = errno;
    turn_on_led (led);

    setSockFlag (raspi_connect_server.fd, O_NONBLOCK, false);
    sockNoNagle (raspi_connect_server.fd);
    errno = errno_save;
    return 0;
}

void loadSSLClnt ()
{
    if (!raspi_connect_server.sslEnable)
        return;

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    raspi_connect_server.ssl_ctx = initSSL (false);
    if (raspi_connect_server.ssl_ctx == NULL) exitCleanupClnt ();
    if (config_client.sslMode == ssl_load_ca)
    {
        int rtn_flag = loadCA (raspi_connect_server.ssl_ctx, config_client.caFile);
        if (!rtn_flag) exitCleanupClnt ();
    }

    raspi_connect_server.ssl_fd = SSL_new (raspi_connect_server.ssl_ctx);

    raspi_connect_server.ssl_fd = SSL_fd (raspi_connect_server.ssl_ctx, raspi_connect_server.fd);
    if (raspi_connect_server.ssl_fd == NULL) exitCleanupClnt ();
    setVerifyPeer (raspi_connect_server.ssl_ctx, config_client.sslMode == ssl_load_ca);

    if (SSL_connect (raspi_connect_server.ssl_fd) == -1)
    {
        perr (true, LOG_ERR,
              "Server authentication failed, connection disconnected");
        exitCleanupClnt ();
    }
    printf ("Peer Cert:\n");
    showPeerCert (raspi_connect_server.ssl_fd);

}

bool fileUUID (uuid_t uu, bool true_for_load_false_for_record)
{
    if (true_for_load_false_for_record)
    {
        memset (uu, 0, sizeof (uuid_t));
        int uu_fd = readOpen (UUID_CLIENT_FILE);
        if (uu_fd == -1)
        {
            strcpy ((char *) uu, UUID_NONE);
            return false;
        }
        read (uu_fd, uu, sizeof (uuid_t) + 1);
        close (uu_fd);
        return true;
    } else
    {
        int uu_fd = writeOpen (UUID_CLIENT_FILE);
        if (uu_fd == -1)return false;
        write (uu_fd, uu, sizeof (uuid_t));
        close (uu_fd);
        return true;
    }
}

void negotiateUUID ()
{
    uuid_t uuid_client;
    uuid_t uuid_zero = {0};
    char uuid_client_string[40];
    fileUUID (uuid_client, true);
    writeServer (uuid_client, sizeof (uuid_t));

//    if (uuid_compare (uuid_client, (uuid_t) {UUID_NONE}) == 1);
    if (memcmp (uuid_client, UUID_NONE, strlen (UUID_NONE)) == 0 ||
        memcmp (uuid_client, uuid_zero, sizeof (uuid_t)) == 0)
        readServer (uuid_client, sizeof (uuid_t)),
                fileUUID (uuid_client, false);
    uuid_unparse (uuid_client, uuid_client_string);
    perr (true, LOG_INFO, "Client UUID: %s", (char *) uuid_client_string);
}

void checkMonit ()
{
    clearMonit (& raspi_monit_data);
    raspi_monit_data.cpu_temper = read_cpu_temp ();
    raspi_monit_data.distance = disMeasure (DISTANCE_T, DISTANCE_E);
    readSensorData (TEMP_HUMI, & raspi_monit_data.env_humidity, & raspi_monit_data.env_temper);

    /**
     * The distance can not be zero
     * Identify as invalid data and skip this loop */
    if (raspi_monit_data.distance == 0)
        return;

    /**
     * When the measured distance is less than 30
     * it is deemed that there is an object approaching */
    if (raspi_monit_data.distance <= 30)
        turn_on_led (LED_GRE);
    else turn_off_led (LED_GRE);
}

void sendData (int led)
{
    char status[4];
    ssize_t read_len, write_len;
    int sleep_time;

    sleep_time = config_client.interval;
    memset (status, 0, 4);

    // The distance cannot be 0, which means no valid data is read by default
    if (raspi_monit_data.distance == 0)
    {
        sleep (sleep_time);
        return;
    }

//    if (raspi_connect_server.sslEnable)
//        write_len = SSL_write (raspi_connect_server.ssl_fd, & raspi_monit_data, sizeof (raspi_monit_data));
//    else write_len = write (raspi_connect_server.fd, & raspi_monit_data, sizeof (raspi_monit_data));
    write_len = writeServer (& raspi_monit_data, sizeof (raspi_monit_data));

    // write_len == -1 -> SIGPIPE -> exit,clean
    printf ("write done. len = %zd\n", write_len);

//    alarm (10);  // set a timer
//    if (raspi_connect_server.sslEnable)
//        read_len = SSL_read (raspi_connect_server.ssl_fd, status, 4);
//    else read_len = read (raspi_connect_server.fd, status, 4);
//    read_len = readServer (status, 4);
//    alarm (0);  // unset timer
    flash_led (led, 90);

//    if (read_len == -1)
//    {
//        sleep_time = config_client.interval * 5;
//        perr (true, LOG_ERR, "NetWork Error! Service Will resetClnt in %d secs", sleep_time);
//        sleep (sleep_time);
//        raise (SIGALRM);
//    }
//    if (strcmp (status, "FIN") == 0)
//    {
//        printf ("FIN received, len = %zd, exiting in %d secs\n", read_len, sleep_time);
//        sleep (sleep_time);
//        raise (SIGTERM);
//    }
}