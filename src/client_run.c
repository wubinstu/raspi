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
    errno = 0;
    turn_off_led (led);
    do
    {
        perr_d (true, LOG_INFO, "Trying to connect to the server...");
        serv_fd = connectServ_d (file_client_config.serv_ip, file_client_config.serv_port);
        if (serv_fd == -1)
        {
            if (file_client_config.frecatps-- == 0) return -1;
            perr_d (true, LOG_NOTICE,
                    "Miss Connection, Retry in %d secs", file_client_config.frectime);
            sleep (file_client_config.frectime);
        }
    } while (serv_fd == -1);

    errno = 0;
    turn_on_led (led);

    set_fl_d (serv_fd, O_NONBLOCK, false);
    sockNagle_d (serv_fd);

    return 0;
}

void check_monit_cleanup (void *arg)
{
    printf ("check 1\n");
    int rtn = pthread_mutex_lock (&mutex_monit_data);
    if (rtn == EOWNERDEAD)
    {
        rtn = pthread_mutex_consistent (&mutex_monit_data);
        if (rtn != 0)
        {
            pthread_mutex_destroy (&mutex_monit_data);
            pthread_mutex_init (&mutex_monit_data, NULL);
        }
        if (rtn == 0)
            pthread_mutex_unlock (&mutex_monit_data);
        pthread_mutex_lock (&mutex_monit_data);
    }
    printf ("check 2\n");
    clearMonit (&raspi_monit_data);
    pthread_mutex_unlock (&mutex_monit_data);
    printf ("check 3\n");
    turn_off_led (LED_YEL);
    turn_off_led (LED_RED);
    turn_off_led (LED_GRE);
    printf ("check 4\n");
}

_Noreturn void *checkMonit (void *arg)
{
    errno = 0;
    int temp_counts = 0;
    int while_counts = 0;
    int notice_counts = 5;
    int while_counts_copy = -1;
    int sleep_time = file_client_config.interval / 2;
    int rtn;
    bool can_be_record = true;
    pthread_cleanup_push(check_monit_cleanup, NULL) ;
            while (true)
            {
                printf ("5\n");
                rtn = pthread_mutex_lock (&mutex_monit_data);
                if (rtn == EOWNERDEAD)
                {
                    rtn = pthread_mutex_consistent (&mutex_monit_data);
                    if (rtn != 0)
                    {
                        pthread_mutex_destroy (&mutex_monit_data);
                        pthread_mutex_init (&mutex_monit_data, NULL);
                    }
                    if (rtn == 0)
                        pthread_mutex_unlock (&mutex_monit_data);
                    pthread_mutex_lock (&mutex_monit_data);
                }
                clearMonit (&raspi_monit_data);
                raspi_monit_data.cpu_temper = read_cpu_temp ();
                raspi_monit_data.distance = disMeasure (DISTANCE_T, DISTANCE_E);
                readSensorData (TEMP_HUMI, &raspi_monit_data.env_humidity, &raspi_monit_data.env_temper);
                pthread_mutex_unlock (&mutex_monit_data);

                /**
                 * The distence can not be zero
                 * Identify as invalid data and skip this loop */
                if (raspi_monit_data.distance == 0)
                    continue;


                /**
                 * When the measured distance is less than 30
                 * it is deemed that there is an object approaching */
                if (raspi_monit_data.distance <= 30)
                    turn_on_led (LED_GRE);
                else turn_off_led (LED_GRE);

                /**
                  * Environmental exception check rules:
                 * It is recognized as abnormal
                 * if it is detected 5 times in 10 loops
                 * This is to prevent occasional mistakes */
                if (raspi_monit_data.cpu_temper >= MAX_CPU_TEMPER ||
                    raspi_monit_data.env_humidity >= MAX_ENV_HUMIDI)
                {
                    temp_counts++;
                    if (can_be_record)
                    { while_counts_copy = while_counts, can_be_record = false; }
                    if (temp_counts >= notice_counts)
                    {
                        temp_counts = 0;
                        turn_on_led (LED_YEL);
                        perr_d (true, LOG_WARNING,
                                "The temperature or humidity is too high");
                    }
                } else turn_off_led (LED_YEL);
                pthread_testcancel ();

                while_counts++;
                while_counts %= 9;
                if (temp_counts != 0)
                {
                    if (while_counts_copy <= while_counts)
                    {
                        if (while_counts - while_counts_copy >= notice_counts)
                        {
                            temp_counts = 0;
                            can_be_record = true;
                        }
                    } else if (while_counts_copy > while_counts)
                    {
                        if (while_counts + 10 - while_counts >= notice_counts)
                        {
                            temp_counts = 0;
                            can_be_record = true;
                        }
                    }
                }

                pthread_testcancel ();
                sleep (sleep_time);
                pthread_testcancel ();
            }
    pthread_cleanup_pop(0);
}

void send_data_cleanup (void *arg)
{
    printf ("Q\n");
    if (check_fd (serv_fd))
    {
        printf ("W\n");
        if (mode_strict)
        {
            printf ("E\n");
            SSL_write (ssl_serv_fd, "FIN", 4);
            printf ("R\n");
            SSL_shutdown (ssl_serv_fd);
            printf ("T\n");
            SSL_free (ssl_serv_fd);
            printf ("Y\n");
            SSL_CTX_free (ctx);
            printf ("U\n");
        } else write (serv_fd, "FIN", 4);
        printf ("I\n");
        close (serv_fd);
    }
    printf ("O\n");
}

_Noreturn void *sendData (void *led)
{
    char status[4];
    ssize_t read_len, write_len;
    int sleep_time;
    int rtn;
    pthread_cleanup_push(send_data_cleanup, NULL) ;
            printf ("berfore push\n");
            while (true)
            {
                pthread_testcancel ();
                printf ("P\n");
                pthread_testcancel ();
                sleep_time = file_client_config.interval;
                memset (status, 0, 4);

                // The distance cannot be 0, which means no valid data is read by default
                if (raspi_monit_data.distance == 0)
                {
                    pthread_testcancel ();
                    sleep (sleep_time);
                    pthread_testcancel ();
                    continue;
                }

                printf ("F1\n");
                rtn = pthread_mutex_lock (&mutex_monit_data);
                if (rtn == EOWNERDEAD)
                {
                    rtn = pthread_mutex_consistent (&mutex_monit_data);
                    if (rtn != 0)
                    {
                        pthread_mutex_destroy (&mutex_monit_data);
                        pthread_mutex_init (&mutex_monit_data, NULL);
                    }
                    if (rtn == 0)
                        pthread_mutex_unlock (&mutex_monit_data);
                    pthread_mutex_lock (&mutex_monit_data);
                }
                printf ("F2\n");
                if (mode_strict)
                    write_len = SSL_write (ssl_serv_fd, &raspi_monit_data, sizeof (raspi_monit_data));
                else write_len = write (serv_fd, &raspi_monit_data, sizeof (raspi_monit_data));
                pthread_mutex_unlock (&mutex_monit_data);
                printf ("F3\n");

                pthread_testcancel ();

                // write_len == -1 -> SIGPIPE -> exit,clean
                printf ("write done. len = %zd\n", write_len);

                printf ("F4\n");
                alarm (10);  // set a timer
                printf ("F5\n");
                if (mode_strict)
                    read_len = SSL_read (ssl_serv_fd, status, 4);
                else read_len = read (serv_fd, status, 4);
                printf ("F6\n");
                alarm (0);  // unset timer
                printf ("F7\n");
                flash_led (*(int *) led, 90);
                printf ("F8\n");

                if (strcmp (status, "FIN") == 0)
                {
                    printf ("FIN received, len = %zd, exiting in %d secs\n", read_len, sleep_time);
                    sleep (sleep_time);
                    raise (SIGTERM);
                }

                if (read_len == -1)
                {
                    sleep_time = file_client_config.interval * 5;
                    perr_d (true, LOG_ERR, "NetWork Error! Service Will resetClnt in %d secs", sleep_time);
                    sleep (sleep_time);
                    raise (SIGALRM);
                }

                pthread_testcancel ();
                sleep (sleep_time);
                pthread_testcancel ();
            }
            printf ("berfore pop\n");
    pthread_cleanup_pop(0);
}