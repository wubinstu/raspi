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


/** to let some function know it's works for server/client */
extern enum ServClnt mode_serv_clnt;
/** to let server and client know the way to ssl connect */
extern enum ClntSSL mode_ssl_client;
/** to let client and server know the way to ssl connect */
extern enum ServSSL mode_ssl_server;

extern int filed_logLevel;
extern int rssl_logLevel;
extern int socket_fd_logLevel;

/** client's configuration file options */
extern ConfOptClnt config_client;
/** server's configuration file options */
extern ConfOptServ config_server;
/** a structural with CPU/environmental temperature,ultrasonic distance,etc */
extern RaspiMonitData raspi_monit_data;


/** pid file fd */
extern int pid_file_fd;
/** server socket fd */
extern int serv_fd;
/** server socket ssl fd */
extern SSL * ssl_serv_fd;
/** a ssl ctx_client_to_server handler */
extern SSL_CTX * ctx_client_to_server;

/** check pid file and enable ssl connection */
extern bool mode_strict;

/** help to reload configuration file,reconnect when meet a problem */
extern jmp_buf jmp_client_rest;
extern jmp_buf jmp_server_rest;

extern void setServClnt (enum ServClnt m);

extern void set_ssl_client (enum ClntSSL m);

#endif //__GLOBAL_H_
