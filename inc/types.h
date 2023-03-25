
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
} LinkNode, * PLinkNode;

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
    int epfd;                           // 专用于服务器套接字,客户端不应该使用
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
    char buf[BUFSIZ];                   // 数据交互缓冲区
    char id[32];                        // 客户端唯一标识
    SSL * ssl_fd;                       // SSL 套接字,加密算法,密钥,缓冲区
//    SSL_CTX * ssl_ctx;                  // SSL 会话配置,证书,私钥,协议版本等
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
    enum
    {
        inError = -1,             // 任务本身参数错误, 或者任务函数返回了非正常值
        ignorable = 0,            // 未初始化的无效任务, 或者已经完成的任务
        newlyBuild = 1,           // 新建的有效任务, 等待处理
        onProcessing = 2,         // 正在处理
    } state;                      // 任务状态
    pthread_t process_thread;     // 处理这个 task 的线程ID
    client_info_t * client;       // 客户端信息
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
    thread_task_t * queue;              // 任务队列头指针
    unsigned int queue_head;            // 任务队列头指针
    unsigned int queue_tail;            // 任务队列尾指针


    pthread_mutex_t lock;               // 互斥锁
    pthread_cond_t cond;                // 条件变量
    pthread_cond_t not_full;            // 条件变量, 表示任务队列有空位(至少存在一个空位)
    pthread_cond_t not_empty;           // 条件变量, 表示任务队列有任务(至少存在一个任务)
} thread_pool_t;

typedef struct hash_node
{
    unsigned int hash_node_key;
    client_info_t clientInfo;
    struct hash_node * next;
} hash_node_t;


typedef struct hash_map
{
    int size;
    hash_node_t ** hashMap;
} hash_map_t;


/** Fill data */
extern void setPair (KeyValuePair * e, const char * name, const char * value);

/** Free elem */
extern void delPair (KeyValuePair * e);

/** Print elem */
extern void catPair (KeyValuePair e);

/** Print elems */
extern void catPairs (KeyValuePair e[], int length);

/** set struct value to zero */
extern void clearMonit (RaspiMonitData * mn);

/**
 * set default value to configuration */
extern void defaultConfOptClnt (ConfOptClnt * co);

extern void defaultConfOptServ (ConfOptServ * co);


/**
 * Initialize the linked list and
 * allocate available space for the queue_head node */
extern void InitLinkList (PLinkNode * L);

/**
 * Given a certain array and its length, create a linked list,
 * and the created linked list will be returned in the form of
 * pointer through the first parameter */
extern void CreateLinkList (PLinkNode * L, KeyValuePair e[], int length);

/**
 * Free the space allocated by all nodes of
 * the whole linked list from the first node */
extern void DestroyLinkList (PLinkNode * L);

/**
 * Return the length of the Given linked list.
 * Note: This refers to the number of effective nodes,
 * excluding the number of remaining nodes of the queue_head node*/
extern int LengthOfLinkList (PLinkNode L);

/**
 * Inserts the given element at the specified location
 * After this operation, the order of the new elements
 * in the valid nodes will be the value of parameter "location".
 * But if the "location" is invalid, then do nothing */
extern void InsertIntoLinkList (PLinkNode * L, int location, KeyValuePair e);

/**
 * Add an element at the end of the linked list*/
extern void AddToLinkList (PLinkNode * L, KeyValuePair e);

/**
 * Given the linked list and location, return the pointer
 * (Space needs to be allocated in advance)
 * where the element is located through the third parameter
 * if the "location" is invalid, then do nothing */
extern void GetFromLinkList (PLinkNode L, int location, KeyValuePair * e);

/**
 * Free  the node and elem by the specified location */
extern void DeleteAtLinkList (PLinkNode * L, int location);

/**
 * Sequential output linked list.
 * Note: it calls the output function suitable
 * for a specific element type, so if you change
 * the data field type, you must update the implementation
 * of the output function synchronously at file "types.c/h" */
extern void DisplayLinkList (PLinkNode L);

/**
 * Write the values in the linked list into the array in order
 * and return the length of the successfully written array.
 * The array space needs to be allocated in advance*/
extern int ListToArry (PLinkNode L, KeyValuePair e[]);

extern hash_map_t * hash_map_init (int size);

extern void hash_map_destroy (hash_map_t * map);

extern hash_node_t * hash_map_get (hash_map_t * map, int hash_index, int hash_key);

extern void hash_map_put (hash_map_t * map, int hash_index, hash_node_t * new_node);

extern void hash_map_del (hash_map_t * map, int hash_index, int hash_key);


#endif