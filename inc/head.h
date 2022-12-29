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
// 此文件将由 cmake 创建(configure_file)
#include "cmake_conf.h"

// wiring PI Pin Numbers
// 树莓派4b上的GPIO针脚定义,数字是wiringPi编码,非BCM编码
#define TEMP_HUMI       3
#define DISTANCE_T      4
#define DISTANCE_E      5
//#define OLED_DATA       26
//#define OLED_CLOCK      6
#define LED_RED         27
#define LED_GRE         28
#define LED_YEL         29

#define BUF_SIZE        1024

#define RESET           1

// 定义树莓派CPU最大温湿度,越过此值定义为"不健康的运行状态"
#define MAX_CPU_TEMPER  60
#define MAX_ENV_HUMIDI  75

// 创建文件(夹)时默认使用的权限组
#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

// 定义配置文件,PID锁文件,温度监控文件存放位置
#define CONF_FILE   CONF_DIR"/"PROJECT_NAME".conf"
#define PID_FILE   "/var/run/"PROJECT_NAME"d.pid"
#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"


#endif