
#ifndef __TYPES_H_
#define __TYPES_H_

#include "head.h"
#include "stdbool.h"
#include "mysql/mysql.h"
#include "openssl/ssl.h"

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

    enum
    {
        ssl_disable = -1,
        ssl_load_none = 0,
        ssl_load_ca = 1
    } sslMode;

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

// 服务器套接字, 地址信息
typedef struct server_info
{
    int fd;                             // 服务器套接字描述符
    struct sockaddr_in addr;            // 服务器地址
    int addr_len;                       // 服务器地址长度
    SSL * ssl_fd;                       // SSL 套接字,加密算法,密钥,缓冲区
    SSL_CTX * ssl_ctx;                  // SSL 会话配置,证书,私钥,协议版本等
    bool sslEnable;                     // 是否启用 SSL 连接
} server_info_t;


typedef struct mysql_conn_node
{
    MYSQL * connection;                 // MySQL连接
    int index;
    bool isConnected;
    bool isBusy;
} sql_node_t;

// MySQL 连接池
typedef struct mysql_conn_pool
{
    bool shutdown;                      // 连接池关闭标志
    char * host;                        // MySQL服务器地址
    unsigned short port;                // MySQL服务器端口号
    char * user;                        // MySQL用户名
    char * pass;                        // MySQL密码
    char * db;                          // 默认数据库名
    unsigned int conn_max;              // 最大连接数量
    unsigned int conn_min;              // 最小连接数量
    unsigned int conn_cur;              // 当前有效连接数量
    unsigned int conn_busy;             // 当前工作中连接数量
    sql_node_t * sql_pool;              // 连接池数组
    pthread_t mtid;                     // 连接池管理者线程
    pthread_mutex_t lock;               // 互斥锁, 保护对连接池数组读写
    pthread_cond_t cond;                // 条件变量
} sql_pool_t;

// 客户端信息结构体
typedef struct client_info
{
    int fd;                             // 客户端套接字描述符
    struct sockaddr_in addr;            // 客户端地址
    int addr_len;                       // 客户端地址长度
    char id[32];                        // 客户端唯一标识
    SSL * ssl_fd;                       // SSL 套接字,加密算法,密钥,缓冲区
    SSL_CTX * ssl_ctx;                  // SSL 会话配置,证书,私钥,协议版本等
    bool sslEnable;                     // 是否启用 SSL 连接
    sql_node_t * sql;                   // MySQL 数据库连接
} client_info_t;

// 线程池任务结构体
typedef struct thread_task
{
    void * (* func) (void *);     // 任务函数
    void * args;                  // 任务参数
    time_t ctime;                 // 任务创建时间
    time_t atime;                 // 任务进队时间
    time_t ptime;                 // 任务开始执行时间
    pthread_t process_thread;     // 处理这个 task 的线程ID
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