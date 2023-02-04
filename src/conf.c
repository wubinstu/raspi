
#include "arr.h"
#include "conf.h"
#include "filed.h"
#include "global.h"
#include "log.h"
#include "linklist.h"

// 定义此文件中的函数在运行时发生错误时默认使用的消息等级,并且默认不设置setter
static int conf_logLevel = LOG_WARNING;

void defaultConfServ (const char *file_path)
{

}

void defaultConfClnt (const char *file_path)
{
    errno = 0;
    setFiledLogLevel (conf_logLevel);
    int rw_fd = newopen_d (file_path);
    if (rw_fd == -1) return;
    char *default_msg_client[] =
            {
                    "\n",
                    "# This is the configuration file of project rain\n",
                    "# Writing rules: [key = value]\n",
                    "# 1.Case insensitive\n",
                    "# 2.Ignore spaces,double quotes,tabs\n",
                    "# 3.Notes are indicated by pound signs(#)\n",
                    "# 4.Incorrect writing may cause the program to not work properly!\n",
                    "# Here are some usage examples\n\n",
                    "# 配置文件书写规则: key = value\n",
                    "# 1.大小写不敏感\n",
                    "# 2.自动忽略空格,双引号,制表符\n",
                    "# 3.注释请用井号开头,可以写成行尾注释\n",
                    "# 4.请不要乱写一些非法配置,否则程序可能不能正确运行!\n",
                    "# 以下是书写范例:\n",
                    "\n\n\n",
                    "# Specify the address of the server\n",
                    "# Both domain name and IP address are allowed\n",
                    "# 指定服务器地址,IPv4地址和域名都是可以的\n",
                    "#ServAddr = 127.0.0.1\n",
                    "\n",
                    "# Specify the port of the server\n",
                    "# 指定服务器端口号\n",
                    "#ServPort = 1234\n",
                    "\n\n\n",
                    "# [严格模式说明]\n",
                    "# 使用命令行参数--strict以启用严格模式\n",
                    "# 严格模式将会在开始运行程序时检查PID锁文件,并且使用SSL安全连接\n",
                    "# 如果启用严格模式,请根据下面SSLMode的值指定相应的安全文件\n",
                    "\n\n\n",
                    "# set 0,1,2 to explain how client should do\n",
                    "# 0: load NONE, client can connect to any server(even correct certificates)\n",
                    "# 1: load ca certificate, client verify server, but server is not\n",
                    "# 2: load ca certificate,client certificate,client private key,server verify client\n",
                    "# 客户端身份鉴别,此选项仅在严格模式有效,用于解释客户端如何和服务器连接\n",
                    "# 0: 不加载任何安全文件,客户端允许以ssl连接到任何特定服务器,即使服务器证书有误\n",
                    "# 1: 仅加载CA证书,这使得客户端可以鉴别服务器身份,客户端可以信任服务器的合法性\n",
                    "# 2: 加载CA证书,客户端用户证书,客户端私密钥,仅当服务器要求验证客户端的合法性时指定,当然服务器不要求时指定了也无影响\n",
                    "#SSLMode = 0,1,2\n",
                    "\n",
                    "# Specifies the absolute path of the CA certificate\n",
                    "# Used to authenticate the server\n",
                    "# 指定CA证书(绝对路径),此选项仅在严格模式有效,用于验证服务器身份\n",
                    "#CAfile = /home/pi/raspi/keys/ca.crt\n",
                    "\n",
                    "# Specifies the absolute path of the user certificate\n",
                    "# specified when \"CheckMe = true\"\n",
                    "# 指定客户端(用户)证书(绝对路径),此选项仅在严格模式有效,用于让服务器验证客户端身份\n",
                    "#UCert = /home/pi/raspi/keys/client.crt\n",
                    "\n",
                    "# Specifies the absolute path of the user private key\n",
                    "# specified when \"CheckMe = true\"\n",
                    "# 指定客户端(用户)证书密钥(绝对路径),此选项仅在严格模式有效,用于和用书证书配对\n",
                    "#UKey = /home/pi/raspi/keys/client.key\n",
                    "\n",
                    "# Data sampling interval(Seconds)\n",
                    "# 数据采样间隔,单位: 秒\n",
                    "#Interval = 5\n",
                    "\n",
                    "# Failed reconnection time(Seconds)\n",
                    "# Note: this only indicates the time from the failure of the connect function to the next connect request.\n",
                    "# Don't forget that the connect itself will block for a period of time(NO BLOCKING)\n",
                    "# 连接失败重连时间间隔,单位: 秒\n",
                    "# 这仅仅从连接失败时开始计时,即connect函数错误返回\n",
                    "# 因此如果网络套接字为阻塞型套接字,那么下一次连接请求发送的时间就是connect等待时间+FRecTine\n",
                    "#FRecTime = 42\n",
                    "\n",
                    "# Failed reconnection attempts\n",
                    "# 连接失败重连尝试次数\n",
                    "#FRecAtps = 20\n",
                    "\n\n\n\n",
                    "ServAddr = 127.0.0.1\n",
                    "ServPort = 1234\n",
                    "SSLMode = 0\n",
                    "CAfile = /home/pi/raspi/keys/ca.crt\n",
                    "UCert = /home/pi/raspi/keys/client.crt\n",
                    "UKey = /home/pi/raspi/keys/client.key\n",
                    "Interval = 5\n",
                    "FRecTime = 42\n",
                    "FRecAtps = 20\n",
                    "\n"
            };
    /**
     * Remember to modify the loop condition
     * when you change the above array */
    for (int i = 0; i < 80; i++)
        write (rw_fd, default_msg_client[i], strlen (default_msg_client[i]));

    close (rw_fd);
}

LNode readconf (const char *file_path)
{
    errno = 0;
    setFiledLogLevel (conf_logLevel);
    FILE *conf_stream = fopen (file_path, "r");

    if (conf_stream == NULL)
    {
        perr_d (true, conf_logLevel,
                "function fdopen returns NULL when you called readconf");
        return NULL;
    }

    LNode L;
    KeyValuePair e;
    char line[BUF_SIZE] = {'\0'};  // 用于保存配置行
    char aline[BUF_SIZE] = {'\0'}; // 用于保存键值对中的键
    char bline[BUF_SIZE] = {'\0'}; // 用于保存键值对中的值
    InitLinkList (&L);  // 分配头节点空间

    while (fgets (line, BUF_SIZE, conf_stream) != NULL)  // 每次从文件中读取一行文字
    {
        rmCharacter (line, '\t');  // 删除这一行中所有制表符
        rmCharacter (line, ' ');   // 删除这一行中所有空格
        if (isEmptyL (line) || isNotes (line))  // 忽略空行或者注释行
            continue;

        rmNextL (line);
        rmCharacter (line, '"');// rm quotation marks
        if (isContainC (line, '=') && isContainC (line, '#'))
        {
            subString (line, '#', aline, bline);  // 对行尾注释进行处理
            subString (aline, '=', e.name, e.value);
        }
        if (isContainC (line, '=') && !isContainC (line, '#'))
            subString (line, '=', e.name, e.value);

        upperConversion (e.name);

        AddToLinkList (&L, e);  // 将一条配置信息加入链表
        memset (line, '\0', BUF_SIZE);
    }
    fclose (conf_stream);
    return L;
}

bool checkconf (const char *file_path)
{
    errno = 0;
    bool flag = true;
    char *conf;
    if (mode_serv_clnt == client)conf = CONF_FILE_CLIENT;
    if (mode_serv_clnt == server)conf = CONF_FILE_SERVER;

    int fd = readopen_d (file_path);
    if (fd == -1)
    {
        perr_d (true, conf_logLevel,
                "Can not find configuration file, "
                "The configuration file will be created at %s automatically, "
                "For security reasons, the service has stopped, "
                "Please write your settings and restart the service", conf);
        if (mode_serv_clnt == client)defaultConfClnt (file_path);
        if (mode_serv_clnt == server)defaultConfServ (file_path);
        flag = false;
    }
    close (fd);
    return flag;
}

bool checkread (LNode L)
{
    errno = 0;
    bool flag = (L->next != NULL);
    if (flag)
    {
        LNode master = L->next;
        while (master != NULL)
        {
            if (notASCII (master->opt->name) || notASCII (master->opt->value))
            {
                flag = false;
                perr_d (true, conf_logLevel,
                        "unknown word in configuration line %s = %s",
                        master->opt->name, master->opt->value);
            }
            master = master->next;
        }
    }
    return flag;
}