
#ifndef __TYPES_H_
#define __TYPES_H_

#include "head.h"
#include "stdbool.h"

enum ServClnt
{
    server = 0, client = 1
};

enum ClntSSL
{
    client_ssl_disable = -1,
    client_ssl_empty = 0,
    client_ssl_only_ca = 1
};

enum ServSSL
{
    server_ssl_disable = -1,
    server_ssl_enable = 1
};

/** For configuration file */
typedef struct element
{
    char name[10];
    char value[90];
} KeyValuePair;

/**
 * Define the structure of linked list nodes
 * The use of pointer type data fields will
 * make the linked list more widely applicable */
typedef struct LinkNode
{
    KeyValuePair * opt;
    struct LinkNode * next;
} Node, * LNode;

/** For data detection */
typedef struct monitoring
{
    float distance;
    float cpu_temper;
    float env_temper;
    float env_humidity;
} RaspiMonitData;

typedef struct configuration_options_client
{
    unsigned long servIp;
    unsigned short servPort;
    int interval;

    char caFile[256];
    char pidFile[256];
    bool modeDaemon;
} ConfOptClnt;

typedef struct configuration_options_server
{
    unsigned long bindIp;
    unsigned short bindPort;
    unsigned short httpPort;
    unsigned short sqlPort;


    bool modeDaemon;
    bool modeSSL;
    char caFile[256];
    char servCert[256];
    char servKey[256];
    char pidFile[256];

    char sqlHost[256];
    char sqlUser[256];
    char sqlPass[256];
    char sqlName[256];

} ConfOptServ;

// 客户端信息结构体
typedef struct client_info
{
    int fd;                       // 客户端套接字描述符
    struct sockaddr_in addr;      // 客户端地址
    char id[32];                  // 客户端唯一标识
    char buf[BUFSIZ];             // 接收缓冲区
    int len;                      // 接收缓冲区中数据长度
    MYSQL * mysql;                // MySQL 数据库连接
} client_info_t;

// 线程池任务结构体
typedef struct thread_task
{
    void * (* func) (void *);     // 任务函数
    void * args;                   // 任务参数
    client_info_t * client;       // 客户端信息
    struct thread_task * next;    // 指向下一个任务的指针
} thread_task_t;

// 线程池结构体
typedef struct thread_pool
{
    bool shutdown;                      // 关闭标志
    unsigned int thread_max;            // 最大线程数
    unsigned int thread_min;            // 最小线程池
    unsigned int thread_alive;          // 当前存活线程数
    unsigned int thread_busy;           // 当前工作中线程数
    unsigned int thread_exitcode;       // 线程数量调节器

    pthread_t * tids;                   // 线程数组
    pthread_t mtid;                     // 管理者线程

    unsigned int queue_max;             // 任务队列最大值
    unsigned int queue_cur;             // 任务队列当前任务数量
    thread_task_t * queue_head;         // 任务队列头指针
    thread_task_t * queue_tail;         // 任务队列尾指针


    pthread_mutex_t lock;               // 互斥锁
    pthread_cond_t cond;                // 条件变量
    pthread_cond_t not_full;            // 条件变量
    pthread_cond_t not_empty;           // 条件变量
} thread_pool_t;

typedef struct mysql_conn_node
{
    MYSQL * connection;                 // MySQL连接
    struct mysql_conn_node * next;      // 指向下一个连接节点的指针
} mysql_conn_node_t;

typedef struct mysql_conn_pool
{
    char * host;                        // MySQL服务器地址
    int port;                           // MySQL服务器端口号
    char * user;                        // MySQL用户名
    char * password;                    // MySQL密码
    char * db;                          // 默认数据库名
    unsigned int max_connection;        // 最大连接数
    unsigned int cur_connection;        // 当前连接数
    mysql_conn_node_t * queue_head;     // 队列头部指针
    mysql_conn_node_t * queue_tail;     // 队列尾部指针
    pthread_mutex_t lock;               // 互斥锁
    pthread_cond_t cond;                // 条件变量
} mysql_conn_pool_t;

/** Fill data */
extern void setElem (KeyValuePair * e, const char * name, const char * value);

/** Free elem */
extern void delElem (KeyValuePair * e);

/** Print elem */
extern void catElem (KeyValuePair e);

/** Print elems */
extern void catElems (KeyValuePair e[], int length);

/** set struct value to zero */
extern void clearMonit (RaspiMonitData * mn);

/**
 * set default value to configuration */
extern void defaultConfOptClnt (ConfOptClnt * co);

extern void defaultConfOptServ (ConfOptServ * co);

#endif