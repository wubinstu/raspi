
#ifndef __TYPES_H_
#define __TYPES_H_

#include "head.h"
#include "stdbool.h"
#include "mysql/mysql.h"
#include "openssl/ssl.h"
#include "uuid/uuid.h"

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
    uuid_t id;                          // 客户端唯一标识
    SSL * ssl_fd;                       // SSL 套接字,加密算法,密钥,缓冲区
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
//    client_info_t * client;       // 客户端信息
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
    thread_task_t * queue;              // 任务队列(以数组的形式实现)
    unsigned int queue_head;            // 任务队列头(数值)指针
    unsigned int queue_tail;            // 任务队列尾(数值)指针


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


// 作者真正意义上第一次接触 web 开发(即使本工程并不会写出一个完善的 web 服务器)
// 这里将本人不清楚的前端概念以注释的形式写在这里, 因此篇幅偏长

enum http_version_t
{
    VERSION_UNKNOWN = 0,

    // HTTP0.9只允许客户端发送GET这一种请求,且不支持请求头,只能传输文本
    // HTTP0.9具有典型的无状态性,每个事务独立进行处理,事务结束时就释放这个连接
    // 客户端发起一个请求,然后由Web服务器返回页面内容,然后连接会关闭,如果请求的页面不存在,也不会返回任何错误码
    HTTP_09 = 1,

    // HTTP1.0对于每个连接都只能传送一个请求和响应,请求完服务器返回响应就会关闭
    // 它支持GET,HEAD,POST方法,但没有Host字段
    HTTP_10 = 2,

    // HTTP1.1是目前使用最广泛的协议版本
    // HTTP1.1增加了OPTIONS,PUT,DELETE,TRACE,CONNECT方法
    // HTTP1.1在同一个连接中可以传送多个请求和响应,多个请求可以重叠和同时进行,它必须有Host字段
    // HTTP1.1引入了许多关键性能优化:keepalive连接,chunked编码传输,字节范围请求,请求流水线等
    // KeepAlive: 允许HTTP设备在事务处理结束之后将TCP连接保持在打开的状态,以便未来的HTTP请求重用现在的连接
    // chunked: 该编码将实体分块传送并逐块标明长度,直到长度为0块表示传输结束,这在实体长度未知时特别有用(比如由数据库动态产生的数据)
    // 字节范围请求: 支持传送内容的一部分,比如当客户端已经有内容的一部分,为了节省带宽可以只向服务器请求一部分
    // 该功能通过在请求消息中引入了range头域来实现,它允许只请求资源的某个部分,在响应消息中Content-Range头域声明了返回的这部分对象的偏移值和长度
    // 如果服务器相应地返回了对象所请求范围的内容，则响应码206（Partial Content）
    HTTP_11 = 3,

    // http2.0是一种安全高效的下一代HTTP传输协议,更加安全高效
    // 支持二进制分帧,多路复用,头部压缩,请求优先级,服务端推送
    HTTP_20 = 4,
};

enum http_connection_t
{
    connection_unknown = 0,
    closed = 1,         // 表示请求/响应完成后关闭TCP连接,不再保持连接
    keepalive = 2,      // 表示保持TCP连接,以便在同一连接上发送更多的请求/响应
    upgrade = 3,        // 用于升级协议,如将HTTP协议升级为WebSocket协议等
    te = 4,             // 表示传输编码,常见的取值包括chunked和compress
};

typedef struct HttpRequest
{
    enum
    {
        METHOD_UNKNOWN = 0,

        // 当客户端要从服务器读取文档时
        // 点击网页上的链接或者通过在浏览器的地址栏输入网址来浏览网页,都是GET方式
        // GET方法要求服务器将URL定位的资源放在响应报文的数据部分,会送给客户端
        // 使用GET方法时,请求参数和对应的值附加在URL后面
        // 以一个?代表URL的结尾与请求参数的开始,传递参数长度受限制
        // GET是安全且幂等的,因为它的操作是只读的,无论操作多少次,服务器上的数据都是安全的,且每次结果都相同
        GET = 1,


        // POST方法将请求的参数封装在HTTP请求数据中
        // 以名称/值的方式出现,可以传输大量数据,对传送的大小没有限制,也不会显示在URL中
        // POST 因为是新增或提交数据,因此它会修改服务器上的资源,所以是不安全的
        // 且多次提交数据就会创建多个资源,也是不幂等的
        POST = 2,

        // GET 和 POST 的区别
        // GET方法是请求从服务器获取资源,可以是图片,文本,页面,视频等
        // POST是向URI指定的资源提交数据,数据就放在请求报文的BODY里
        // GET请求会把请求的数据附在URL后,POST提交是将数据放在报文的body里
        // 因此GET提交的数据会在地址栏中显示出来,而POST提交地址栏不会改变
        // POST传输的资源更大,数据类型更多
        // GET获取资源的速度更快


        // HEAD就像GET
        // 只不过服务端接受到HEAD请求后只返回响应头
        // 而不会发送响应内容
        // 当只需要查看某个页面的状态的时候
        // 使用HEAD是非常高效的
        HEAD = 3,

        // 把消息本体中的消息发送到一个URL,跟POST类似,但不常用
        // 通常用于向服务器发送请求,如果URL不存在,则要求服务器根据请求创建资源
        // 如果存在,服务器就接受请求内容,并修改URL资源的原始版本
        PUT = 4,

        // POST 与 PUT 的区别
        // POST请求的URI表示处理该封闭实体的资源
        // 该资源可能是个数据接收过程,某种协议的网关,或者接收注解的独立实体
        // 然而PUT请求中的URI表示请求中封闭的实体-用户代理知道URI的目标
        // 并且服务器无法将请求应用到其他资源
        // 如果服务器希望该请求应用到另一个URI,就必须发送一个301响应
        // 用户代理可通过自己的判断来决定是否转发该请求
        // HTTP/1.1没有定义一个PUT请求如何影响原始服务器的状态
        // PUT请求必须遵守信息传输要求
        // 除非另有说明,PUT请求中的实体头部应该用于PUT创建或修改的资源上

        // 用于删除指定的资源
        // 状态码 202 (Accepted) 表示请求的操作可能会成功执行,但是尚未开始执行
        // 状态码 204 (No Content) 表示操作已执行,但是没有更多的相关信息
        // 状态码 200 (OK) 表示操作已执行,并且响应中提供了相关状态的描述信息
        DELETE = 5,

        // 用于获取目的资源所支持的通信选项
        // 客户端可以对特定的 URL 使用 OPTIONS 方法
        // 也可以对整站(通过将 URL 设置为"*")使用该方法
        // 响应报文包含一个 Allow 首部字段,该字段的值表明了服务器支持的所有 HTTP 方法
        OPTIONS = 6,

        // 实现沿通向目标资源的路径的消息环回(loop-back)测试,提供了一种实用的 debug 机制
        // 请求的最终接收者应当原样反射(reflect)它接收到的消息,除了以下字段部分
        // 作为一个Content-Type 为 message/http 的 200（OK）响应的消息的主体（body）返回给客户端
        TRACE = 7,

        // 可以开启一个客户端与所请求资源之间的双向沟通的通道,它可以用来创建隧道(tunnel)
        // CONNECT 可以用来访问采用了 SSL (HTTPS) 协议的站点
        // 客户端要求代理服务器将 TCP 连接作为通往目的主机隧道
        // 之后该服务器会代替客户端与目的主机建立连接
        // 连接建立好之后, 代理服务器会面向客户端发送或接收 TCP 消息流
        CONNECT = 8,
    } method;

    enum http_version_t version;


    char URI[256];
    char host[64];
    char referer[256];
    char userAgent[256];


    enum http_connection_t connection;

    char accept[256];
    char acceptEncoding[64];
    char acceptLanguage[64];
    char acceptCharset[64];
    int contentLength;
    char contentType[64];
    char cookie[256];
    char * body;
} http_request_t;


typedef struct HttpResponse
{
    enum http_version_t version;
    enum http_status_t
    {
        HTTP_UNKNOWN = 0,
        HTTP_100 = 1,  // 表示客户端可以继续发送请求,指示服务器正在等待请求的剩余部分
        HTTP_101 = 2,  // 表示服务器正在根据客户端的请求切换协议,以便更好地满足客户端的请求
        HTTP_200 = 3,  // 表示服务器已成功处理请求并返回响应
        HTTP_201 = 4,  // 会在POST请求时使用,表示服务器已经成功地处理了请求,并且在服务器上创建了一个新的资源,并返回资源相关信息
        HTTP_202 = 5,  // 表示服务器已经收到请求,但是还没有完成处理
        HTTP_203 = 6,  // 它表示客户端的请求已成功处理,但是返回的信息可能来自另一个来源,而不是原始服务器,通常这种状态码用于反向代理或内容分发网络(CDN)等中间层服务
        HTTP_204 = 7,  // 表示服务器成功处理了请求,但没有返回任何内容,也称为"无内容"状态
        HTTP_205 = 8,  // 没有包含实体的正文部分,服务器成功处理了请求,但不需要返回任何响应体内容
        HTTP_206 = 9,  // 表示服务器成功处理了部分请求,并返回部分实体内容,通常在断点续传或者范围请求的时候使用
        HTTP_300 = 10, // 代表多种选择,当客户端发出请求时,服务器可以返回多个响应选项,这些选项中的每一个都可以被客户端接受,这种情况通常用于重定向用户到新的资源位置,以及在使用负载平衡器时为客户端提供服务器选择
        HTTP_301 = 11, // 表示永久重定向(Moved Permanently),当客户端请求某个 URL 时,服务器会告诉客户端该 URL 已经被永久移动到另一个 URL,客户端需要使用新的 URL 重新发送请求
        HTTP_302 = 12, // 表示临时重定向,指请求的资源已经被临时移动到了另一个URL,需要客户端进一步的操作才能获取到请求的资源
        HTTP_303 = 13, // 表示请求的资源已经被分配了新的URL,客户端应该使用GET方法重新获取资源
        HTTP_304 = 14, // 客户端在发出请求时,通过指定If-Modified-Since或If-None-Match头部信息,告知服务器本地缓存资源的时间戳或实体标识,并询问服务器是否有最新的版本可用,如果服务器认为客户端缓存的版本仍然是最新的,则返回状态码304,告知客户端可以继续使用缓存的资源
        HTTP_305 = 15, // 表示使用代理服务器请求资源,但代理服务器返回的响应指示需要使用代理服务器进行访问的不同URI,客户端应按照代理服务器返回的URI重发请求
        HTTP_307 = 16, // 一个临时重定向状态码,它表示请求的资源已被临时移动到另一个URI,客户端应该使用新的URI发起后续请求,与302 Found状态码不同,307状态码要求客户端使用相同的HTTP方法来重新发起请求
        HTTP_400 = 17, // 表示客户端发送的请求有语法错误,服务器无法理解,具体而言,这意味着请求可能缺少必需的参数,包含无效的请求头字段,格式不正确的请求主体等等
        HTTP_401 = 18, // 表示未经授权,通常在需要身份验证的情况下使用
        HTTP_402 = 18, // 表示"支付必要",它是指客户端请求未被执行,因为它需要有效付款才能完成,这个状态码通常被用于要求付款的在线商店或订阅服务中
        HTTP_403 = 20, // 表示服务器拒绝客户端的请求,因为客户端没有访问权限或没有经过身份验证
        HTTP_404 = 21, // 服务器无法找到对应的请求资源
        HTTP_405 = 22, // 表示客户端发送了一个针对特定资源的请求,但请求中使用的HTTP方法不被服务器允许,这通常发生在尝试使用错误的HTTP方法(如使用GET请求来提交表单数据)或者在服务器端没有实现请求所需的HTTP方法时
        HTTP_406 = 23, // 请求的资源的内容特性无法满足请求头中的条件,因而无法生成响应实体
        HTTP_407 = 24, // 请求者必须先使用代理服务器进行身份验证,然后才能使用请求的资源
        HTTP_408 = 25, // 服务器等候请求时发生超时
        HTTP_409 = 26, // 请求在当前资源状态下会发生冲突
        HTTP_410 = 27, // 请求的资源已被永久删除,且不会再得到的
        HTTP_411 = 28, // 服务器拒绝处理请求,因为请求头中缺少必需的Content-Length字段,这个状态码通常在POST请求中出现,因为在这种情况下,请求中必须包含请求体的长度
        HTTP_412 = 29, // 表示服务器无法满足请求中设置的一个或多个先决条件,这通常是在使用条件请求(如条件GET或条件PUT)时发生的,其中请求包含一个条件头部(如If-Match,If-None-Match,If-Modified-Since,If-Unmodified-Since,If-Range),该头部指示仅当满足特定条件时才应执行请求
        HTTP_413 = 30, //指客户端发送的请求中包含的实体数据太大,服务器无法处理该请求,通常这种情况发生在客户端尝试上传文件时,文件大小超过了服务器允许的最大限制
        HTTP_414 = 31, // 该状态码表示客户端请求的URI超出了服务器能够处理的最大长度,通常HTTP协议中规定的请求URI最大长度为 8KB(8192 bytes)左右,不同的服务器实现可能会有所不同,当客户端发送的URI长度超过这个限制时服务器将返回414状态码
        HTTP_415 = 32, // 表示服务器无法处理客户端发送的请求,因为请求中的媒体类型不受支持或不正确,这通常发生在客户端发送的请求包含服务器不支持的媒体类型时,例如在请求头中指定了一个无效的Content-Type值
        HTTP_416 = 33, // 客户端请求的数据范围无法满足服务器的要求,这通常发生在客户端请求的资源范围超出了服务器端所能提供的范围
        HTTP_417 = 34, // 当客户端在请求中使用Expect请求头字段来期望服务器返回特定的行为或内容时,如果服务器无法满足这些要求,就会返回417状态码
        HTTP_500 = 35, // 当 Web 服务器在处理客户端请求时遇到意外错误时,会返回此状态码,这可能是由于服务器软件出现故障,配置错误或其他类似问题引起的
        HTTP_501 = 36, // 表示服务器不支持客户端所需的功能或者请求方法,即服务器无法满足请求,具体来说,当服务器无法识别客户端所请求的方法,或者服务器无法完成请求的功能时,会返回501状态码
        HTTP_502 = 37, // 表示服务器在充当网关或代理时未能从上游服务器(如Tomcat或Apache)收到有效响应,这通常表示上游服务器出现故障或无法访问
        HTTP_503 = 38, // 表示服务器无法处理当前请求,因为它暂时过载或维护中,这通常是暂时性的,客户端应该稍后重试请求
        HTTP_504 = 39, // 服务器充当代理时,它在等待上游服务器的响应时已经超时了,这通常意味着上游服务器出现问题或无法访问
        HTTP_505 = 40, // 表示服务器不支持请求中所使用的 HTTP 版本,具体来说,它表示服务器无法理解或不愿意支持请求所使用的 HTTP 版本,这通常是由于服务器使用的 HTTP 版本比请求使用的版本要低,或者服务器压根不支持该版本
    } status;

    enum http_content_type_t
    {
        text_plain = 0,                     // 纯文本格式
        text_html = 1,                      // HTML文档
        text_xml = 2,                       // XML文档
        application_json = 3,               // JSON格式
        application_octet_stream = 4,       // 二进制流数据
        multipart_form_data = 5,            // 多部分表单数据
        image_jpeg = 6,                     // JPEG图片格式
        image_png = 7,                      // PNG图片格式
        audio_mpeg = 8,                     // MPEG音频格式
        video_mp4 = 9,                      // MP4视频格式

    } contentType;


    char date[40];
    char server[256];
    enum http_connection_t connection;
//    char contentType[32];
    int contentLength;
    char * body;
} http_response_t;


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

extern void hash_map_add (hash_map_t * map, int hash_index, hash_node_t * new_node);

extern void hash_map_del (hash_map_t * map, int hash_index, int hash_key);


#endif