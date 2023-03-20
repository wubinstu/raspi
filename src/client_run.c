//
// Created by Einc on 2022/4/4.
//

#include "arr.h"
#include "conf.h"
#include "global.h"
#include "log.h"
#include "load_clean.h"
#include "socket_fd.h"
#include "rssl.h"
#include "client_run.h"
#include "raspi_drive.h"
#include "types.h"


int tryConnect (int led)
{
    int errno_save = errno, num_sec;
    turn_off_led (led);

    for (num_sec = 1; num_sec <= MAXSLEEP; num_sec <<= 1)
    {
        perr (true, LOG_INFO,
              "Trying to connect to the server...");
        serv_fd = connectServ (config_client.servIp,
                               config_client.servPort);
        if (serv_fd == -1)
        {
            perr (true, LOG_NOTICE,
                  "Miss Connection, Retry in %d secs",
                  num_sec);
            sleep (num_sec);
        } else if (serv_fd > 0)
            break;
    }
    errno = errno_save;
    if (num_sec > MAXSLEEP)return -1;

    errno_save = errno;
    turn_on_led (led);

    setSockFlag (serv_fd, O_NONBLOCK, false);
    sockNagle (serv_fd);
    errno = errno_save;
    return 0;
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

    if (mode_ssl_client > client_ssl_disable)
        write_len = SSL_write (ssl_serv_fd, & raspi_monit_data, sizeof (raspi_monit_data));
    else write_len = write (serv_fd, & raspi_monit_data, sizeof (raspi_monit_data));


    // write_len == -1 -> SIGPIPE -> exit,clean
    printf ("write done. len = %zd\n", write_len);

    alarm (10);  // set a timer
    if (mode_ssl_client > client_ssl_disable)
        read_len = SSL_read (ssl_serv_fd, status, 4);
    else read_len = read (serv_fd, status, 4);
    alarm (0);  // unset timer
    flash_led (led, 90);

    if (read_len == -1)
    {
        sleep_time = config_client.interval * 5;
        perr (true, LOG_ERR, "NetWork Error! Service Will resetClnt in %d secs", sleep_time);
        sleep (sleep_time);
        raise (SIGALRM);
    }
    if (strcmp (status, "FIN") == 0)
    {
        printf ("FIN received, len = %zd, exiting in %d secs\n", read_len, sleep_time);
        sleep (sleep_time);
        raise (SIGTERM);
    }
}