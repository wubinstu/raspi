#ifndef __HEAD_H_
#define __HEAD_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <error.h>
#include <errno.h>
#include <syslog.h>
#include <setjmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

// It will create by cmake
#include "cmake_conf.h"

// wiring PI Pin Numbers
#define TEMP_HUMI       3
#define DISTANCE_T      4
#define DISTANCE_E      5
#define OLED_DATA       26
#define OLED_CLOCK      6
#define LED_RED         27
#define LED_GRE         28
#define LED_YEL         29

#define BUF_SIZE        1024

#define RESET           1


#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

#define CONF_FILE   CONF_DIR"/"PROJECT_NAME".conf"
#define PID_FILE   "/var/run/"PROJECT_NAME"d.pid"
#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"


#endif