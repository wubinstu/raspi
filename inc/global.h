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

#define MAXSLEEP        4096
#define BUF_SIZE        1024
#define PAGE_4K         4096


/** 定义树莓派CPU外部硬件采集的数据的临界值
 * 超过此值将标记这组数据为Emerge状态 */
#define MAX_CPU_TEMPER  60
#define MAX_DISTANCE    120
#define MAX_ENV_TEMPER  70
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

#define WEB_HTML_PAGE       "/home/wubin/raspi/web/page.html"

// 线程池, 连接池 相关参数
//线程数量最小值 = CPU核心数 + 1
//线程数量最大值 = CPU核心数 * (1 + 平均等待时间 / 平均执行时间)
//线程任务队列最大值 = (maximumPoolSize - corePoolSize) * 平均执行时间 / 平均等待时间
#define THREAD_POOL_MAX                     20
#define THREAD_POOL_MIN                     5
#define THREAD_POOL_QUEUE                   100
#define THREAD_POOL_MANAGER_SLEEP_TIME      3
#define THREAD_POOL_MANAGER_ADJUST_BY_PER   3


#define SSL_POOL_MAX                        20000
#define SSL_POOL_MIN                        50
#define SSL_POOL_MANAGER_SLEEP_TIME         3
#define SSL_POOL_MANAGER_ADJUST_BY_PER      50

#define SQL_POOL_MAX                        50
#define SQL_POOL_MIN                        5
#define SQL_TABLE_RASPI                     "raspi"
#define SQL_POOL_MANAGER_SLEEP_TIME         3
#define SQL_POOL_MANAGER_ADJUST_BY_PER      5

// 用来创建客户端结构体哈希表, 大小需要设置为素数
#define HASH_TABLE_SIZE                     1999
#define SERVER_EPOLL_SIZE                   10000
#define UUID_CLIENT_FILE                    "/tmp/tmpRaspiRainId"
#define UUID_NONE                           "UUID_NONE"


/** logs levels */
extern int filed_logLevel;
extern int rssl_logLevel;
extern int socket_fd_logLevel;

/** configuration file options */
extern ConfOptClnt config_client;
extern ConfOptServ config_server;

/** a structural with CPU/environmental temperature,ultrasonic distance,etc */
extern RaspiMonitData raspi_monit_data;

/** server address information */
extern server_info_t raspi_connect_server;
extern server_info_t server_accept_raspi;
extern server_info_t server_accept_http;

/** pools */
extern sql_pool_t * sql_pool_accept_raspi;
extern thread_pool_t * thread_pool_accept_raspi;
extern thread_pool_t * thread_pool_accept_http;
extern ssl_pool_t * ssl_pool_accept_raspi;
extern ssl_pool_t * ssl_pool_accept_http;

/** hash maps (to save clients infos) */
extern hash_table_client_t * hash_table_raspi;
extern hash_table_client_t * hash_table_http;
extern hash_table_info_t * hash_table_info_raspi_http;

/** thread id for server epolls */
extern pthread_t thread_id_server_accept_raspi;
extern pthread_t thread_id_server_accept_http;

/** epoll events for server */
extern struct epoll_event event_server_raspi[SERVER_EPOLL_SIZE];
extern struct epoll_event event_server_http[SERVER_EPOLL_SIZE];

/** server and client pid file fd */
extern int pid_file_fd;

extern int web_html_fd;
extern long web_html_size;
extern char * web_html_buf;

/** help to reload configuration file,reconnect when meet a problem */
extern jmp_buf jmp_client_rest;
extern jmp_buf jmp_server_rest;

#endif //__GLOBAL_H_
