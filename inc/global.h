//
// Created by Einc on 2023/1/24.
//

#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include "head.h"
#include "rssl.h"
#include "types.h"
#include "cmake_conf.h"

#define PROJECT_SERVER_NAME PROJECT_NAME"sd"
#define PROJECT_CLIENT_NAME PROJECT_NAME"cd"

/** wiring PI Pin Numbers
 * 树莓派4b上的GPIO针脚定义,数字是wiringPi编码,非BCM编码 */
#define TEMP_HUMI       3
#define DISTANCE_T      4
#define DISTANCE_E      5
//#define OLED_DATA       26
//#define OLED_CLOCK      6
#define LED_RED         27  // get tcp connection
#define LED_GRE         28  // meet distance condition
#define LED_YEL         29  // constant light indicates environmental bad, flashing indicates send a tcp package

#define MAXSLEEP        128
#define BUF_SIZE        1024
#define PAGE_4K         4096


/** 定义树莓派CPU最大温湿度,越过此值定义为"不健康的运行状态"
 * 这个设置并没有太大意义,即使是炎热的夏天树莓派温度也不会轻易达到60以上
 * 因此这里的温湿度只是凭感觉定义着玩而已,本工程会操作亮黄灯(常亮)表示不满足环境条件 */
#define MAX_CPU_TEMPER  60
#define MAX_ENV_HUMIDI  75

/** 创建文件(夹)时默认使用的权限组 */
#define    FILE_MODE        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define    DIR_MODE         (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

/** 定义配置文件,PID锁文件,温度监控文件存放位置 */
#define CONF_FILE_CLIENT    CONF_DIR"/rain_client.conf"
#define CONF_FILE_SERVER    CONF_DIR"/rain_server.conf"
#define PID_FILE_CLIENT     "/var/run/rain_client.pid"
#define PID_FILE_SERVER     "/var/run/rain_server.pid"
#define TEMP_PATH           "/sys/class/thermal/thermal_zone0/temp"

// 线程池, 连接池 相关参数
//线程数量最小值 = CPU核心数 + 1
//线程数量最大值 = CPU核心数 * (1 + 平均等待时间 / 平均执行时间)
//线程任务队列最大值 = (maximumPoolSize - corePoolSize) * 平均执行时间 / 平均等待时间
#define THREAD_POOL_MAX     20
#define THREAD_POOL_MIN     5
#define THREAD_POOL_QUEUE   20
#define SQL_POOL_MAX        20
#define SQL_POOL_MIN        5
#define SQL_TABLE_RASPI     "raspi"

// 用来创建客户端结构体哈希表, 大小需要设置为素数
#define HASH_MAP_SIZE      1999
#define SERVER_EPOLL_SIZE  10000
#define UUID_CLIENT_FILE    "/tmp/tmpRaspiRainId"
#define UUID_NONE           "UUID_NONE"


extern int filed_logLevel;
extern int rssl_logLevel;
extern int socket_fd_logLevel;

/** client's configuration file options */
extern ConfOptClnt config_client;
/** server's configuration file options */
extern ConfOptServ config_server;
/** a structural with CPU/environmental temperature,ultrasonic distance,etc */
extern RaspiMonitData raspi_monit_data;

extern server_info_t raspi_connect_server;
extern server_info_t server_accept_raspi;
extern server_info_t server_accept_http;

extern sql_pool_t * sql_pool_accept_raspi;
extern thread_pool_t * thread_pool_accept_raspi;
extern thread_pool_t * thread_pool_accept_http;

extern hash_map_t * hash_map_raspi;

/** pid file fd */
extern int pid_file_fd;

extern struct epoll_event event_server_raspi[SERVER_EPOLL_SIZE];

/** help to reload configuration file,reconnect when meet a problem */
extern jmp_buf jmp_client_rest;
extern jmp_buf jmp_server_rest;

#endif //__GLOBAL_H_
