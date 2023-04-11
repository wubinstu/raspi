
#include "arr.h"
#include "conf.h"
#include "filed.h"
#include "global.h"
#include "log.h"

// 定义此文件中的函数在运行时发生错误时默认使用的消息等级,并且默认不设置setter
static int conf_logLevel = LOG_WARNING;

void defaultConfServ (const char * file_path)
{
    int errno_save = errno;
    setFiledLogLevel (conf_logLevel);
    int rw_fd = newOpen (file_path);
    if (rw_fd == -1) return;
    char * default_msg_server[] =
            {
                    "\n",
                    "# This is the configuration file of project rainsd\n",
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
                    "# 5.配置文件优先级小于运行参数!\n",
                    "# 以下是书写范例:\n",
                    "\n\n\n",
                    "# Specify the address of the server to bind\n",
                    "# 0.0.0.0,127.0.0.1,address of network interface are all OK\n",
                    "# 指定服务器地址绑定的地址\n",
                    "# 0.0.0.0:     所有网卡网络可见\n",
                    "# 127.0.0.1:   本机网络可见\n",
                    "# 指定网卡地址:  仅特定网卡网络可见\n",
                    "#BindAddr = 0.0.0.0\n",
                    "\n",
                    "# Specify the port of the server to bind\n",
                    "# 指定服务器绑定的端口号\n",
                    "#BindPort = 9190\n",
                    "\n",
                    "# Specify the port of the server HTTP\n",
                    "# 指定服务器HTTP端口号\n",
                    "#HttpPort = 8080\n",
                    "\n\n\n",
                    "# set disable,default to explain how server communicate to client\n",
                    "# disable: disable SSL connection, use tcp only\n",
                    "# default: load CA,Server Certificate,Server Private Key, use SSL connection\n",
                    "# SSL模式选择,用于解释客户端如何和服务器连接\n",
                    "# disable: 禁用SSL连接, 使用TCP明文通信(调试模式优先)\n",
                    "# default: 加载CA证书,服务器证书密钥, 启用SSL连接\n",
                    "#SSLMODE = default\n",
                    "\n",
                    "# specify CA file\n",
                    "# 指定CA文件路径\n",
                    "#CAFILE = /path/to/ca.crt\n",
                    "\n",
                    "# specify Server Certificate file\n",
                    "# 指定服务器证书文件路径\n",
                    "#SERVCERT = /path/to/server.crt\n",
                    "\n",
                    "# specify Server Private Key file\n",
                    "# 指定服务器私秘钥文件路径\n",
                    "#SERVKEY = /path/to/server.key\n",
                    "\n",
                    "# set disable,default,\"PID FILE PATH\" to explain how server runs once a time\n",
                    "# disable: disable PID FILE\n",
                    "# default: use /var/run/*.pid\n",
                    "# \"PID FILE PATH\": use self path,(note: need to modify systemd unit file)\n",
                    "# disable: 禁用PID文件,不检查程序运行的唯一性(调试模式优先)\n",
                    "# default: 使用默认的文件位置: /var/run/*.pid (推荐)\n",
                    "# \"PID FILE PATH\": 自定义pid文件位置(注意: 你可能需要同步手动修改systemd unit文件)\n",
                    "#PIDFILE = default\n",
                    "\n",
                    "# set disable,default to explain whether server enter daemon mode\n",
                    "# disable: 调试模式 \n",
                    "# default: 守护进程模式 \n",
                    "#DAEMON = default \n",
                    "\n",
                    "# set MySQL Host\n",
                    "# 设置 MySQL 服务器\n",
                    "#SQLHOST = 127.0.0.1\n",
                    "\n",
                    "# set MySQL Port\n",
                    "# 设置 MySQL 服务端口\n",
                    "#SQLPORT = 3306\n",
                    "\n",
                    "# set MySQL User\n",
                    "# 设置 MySQL 用户名\n",
                    "#SQLUSER = root\n",
                    "\n",
                    "# set MySQL Pass (not recommend in conf file)\n",
                    "# 设置 MySQL 密码 (不建议设置在配置文件里)\n",
                    "#SQLPASS = mysql\n",
                    "\n",
                    "# set MySQL DataBase Name\n",
                    "# 设置 MySQL 数据库名称\n",
                    "#SQLNAME = rain\n",
                    "\n",
                    "\n\n\n\n",
                    "# 配置文件优先级小于(主函数)运行参数!\n",
                    "BindAddr = 0.0.0.0\n",
                    "BindPort = 9190\n",
                    "HttpPort = 8080\n",
                    "SSLMODE = default\n",
                    "CAFILE = /home/wubin/ssl_fd/keys/einc_fun_root_ca.cer\n",
                    "SERVCERT = /home/wubin/ssl_fd/keys/einc_fun.pem\n",
                    "SERVKEY = /home/wubin/ssl_fd/keys/einc_fun.key\n",
                    "PIDFILE = disable\n",
                    "DAEMON = disable\n",
                    "SQLHOST = 127.0.0.1\n",
                    "SQLPORT = 3306\n",
                    "SQLUSER = root\n",
                    "SQLPASS = mysql\n",
                    "SQLNAME = rain\n",
                    "\n"
            };
    /**
     * Remember to modify the loop condition
     * when you change the above array */
    for (int i = 0; i < 103; i++)
        write (rw_fd, default_msg_server[i], strlen (default_msg_server[i]));

    close (rw_fd);
    errno = errno_save;
}

void defaultConfClnt (const char * file_path)
{
    int errno_save = errno;
    setFiledLogLevel (conf_logLevel);
    int rw_fd = newOpen (file_path);
    if (rw_fd == -1) return;
    char * default_msg_client[] =
            {
                    "\n",
                    "# This is the configuration file of project raincd\n",
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
                    "# 5.配置文件优先级小于运行参数!\n",
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
                    "# set disable,default,\"CA FILE PATH\" to explain how client should connect to server\n",
                    "# disable: disable SSL connection, use tcp only\n",
                    "# default: load NONE, use SSL connection, but client may not check server\n",
                    "# \"CA FILE PATH\": load self ca certificate, use SSL connection, client will check server\n",
                    "# SSL模式选择,用于解释客户端如何和服务器连接\n",
                    "# disable: 禁用SSL连接, 使用TCP明文通信(调试模式优先)\n",
                    "# default: 不加载安全文件, 启用SSL连接, 不要求鉴别服务器身份(推荐)\n",
                    "# \"CA FILE PATH\": 加载CA证书, 启用SSL连接, 验证服务器身份(最可靠)\n",
                    "#CAFILE = default\n",
                    "\n",
                    "# set disable,default,\"PID FILE PATH\" to explain how client runs once a time\n",
                    "# disable: disable PID FILE\n",
                    "# default: use /var/run/*.pid\n",
                    "# \"PID FILE PATH\": use self path,(note: need to modify systemd unit file)\n",
                    "# disable: 禁用PID文件,不检查程序运行的唯一性(调试模式优先)\n",
                    "# default: 使用默认的文件位置: /var/run/*.pid (推荐)\n",
                    "# \"PID FILE PATH\": 自定义pid文件位置(注意: 你可能需要同步手动修改systemd unit文件)\n",
                    "#PIDFILE = default\n",
                    "\n",
                    "# set disable,default to explain whether client enter daemon mode\n",
                    "# disable: 调试模式 \n",
                    "# default: 守护进程模式 \n",
                    "#DAEMON = default \n",
                    "\n",
                    "# Data sampling interval(Seconds)\n",
                    "# 数据采样间隔,单位: 秒\n",
                    "#Interval = 5\n",
                    "\n",
                    "\n\n\n\n",
                    "# 配置文件优先级小于(主函数)运行参数!\n",
                    "ServAddr = 127.0.0.1\n",
                    "ServPort = 9190\n",
                    "CAFILE = disable\n",
                    "PIDFILE = disable\n",
                    "DAEMON = disable\n",
                    "Interval = 5\n",
                    "\n"
            };
    /**
     * Remember to modify the loop condition
     * when you change the above array */
    for (int i = 0; i < 62; i++)
        write (rw_fd, default_msg_client[i], strlen (default_msg_client[i]));

    close (rw_fd);
    errno = errno_save;
}

PLinkNode readConf (const char * file_path)
{
    int errno_save = errno;
    setFiledLogLevel (conf_logLevel);
    FILE * conf_stream = fopen (file_path, "r");

    if (conf_stream == NULL)
    {
        perr (true, conf_logLevel,
              "function fdopen returns NULL when you called readConf");
        return NULL;
    }

    PLinkNode L;
    KeyValuePair e;
    char line[200] = {'\0'};  // 用于保存配置行
    char key[100] = {'\0'}; // 用于保存键值对中的键
    char value[100] = {'\0'}; // 用于保存键值对中的值
    InitLinkList (& L);  // 分配头节点空间

    while (fgets (line, 200, conf_stream) != NULL)  // 每次从文件中读取一行文字
    {
        rmCharacter (line, '\t');  // 删除这一行中所有制表符
        rmCharacter (line, ' ');   // 删除这一行中所有空格
        if (isEmptyL (line) || isNotes (line))  // 忽略空行或者注释行
            continue;

        rmNextL (line);
        rmCharacter (line, '"');// rm quotation marks
        if (isContainC (line, '=') && isContainC (line, '#'))
        {
            subString (line, '#', key, value);  // 对行尾注释进行处理
            subString (key, '=', e.name, e.value);
        }
        if (isContainC (line, '=') && !isContainC (line, '#'))
            subString (line, '=', e.name, e.value);

        upperConversion (e.name);

        AddToLinkList (& L, e);  // 将一条配置信息加入链表
        memset (line, '\0', 200);
    }
    fclose (conf_stream);
    errno = errno_save;
    return L;
}

bool checkConf (const char * file_path)
{
    int errno_save = errno;
    bool flag = true;

    int fd = readOpen (file_path);
    if (fd == -1)
    {
        perr (true, conf_logLevel,
              "Can not find configuration file, "
              "The configuration file will be created at %s automatically, "
              "For security reasons, the service has stopped, "
              "Please write your settings and restart the service", file_path);
        flag = false;
    }
    close (fd);
    errno = errno_save;
    return flag;
}

bool checkRead (PLinkNode L)
{
    int errno_save = errno;
    bool flag = (L->next != NULL);
    if (flag)
    {
        PLinkNode master = L->next;
        while (master != NULL)
        {
            if (notASCII (master->opt->name) || notASCII (master->opt->value))
            {
                flag = false;
                perr (true, conf_logLevel,
                      "unknown word in configuration line %s = %s",
                      master->opt->name, master->opt->value);
            }
            master = master->next;
        }
    }
    errno = errno_save;
    return flag;
}