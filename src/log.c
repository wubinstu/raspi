
#include "log.h"
#include "head.h"
#include "global.h"

char * log_prefix (int logLevel)
{
    switch (logLevel)
    {
        case LOG_EMERG:
            return EMERE
        case LOG_ALERT:
            return ALERT
        case LOG_CRIT:
            return CRIT
        case LOG_ERR:
            return ERR
        case LOG_WARNING:
            return WARNING
        case LOG_NOTICE:
            return NOTICE
        case LOG_INFO:
            return INFO
        case LOG_DEBUG:
            return DEBUG
        default:
            return UNKNOWN
    }
}


void perr (bool condition, int logLevel, const char * message, ...)
{
    if (!condition)
        return;

    // The space size here is not set strictly
    char with_prefix[BUF_SIZE + PREFIX_LEN + CACHE_SIZE];
    strcpy (with_prefix, log_prefix (logLevel));
    if (errno != 0)
    {
        strcat (with_prefix, "(errno) ");
        strcat (with_prefix, strerror (errno));
        strcat (with_prefix, ": ");
        errno = 0;
    }
    strcat (with_prefix, message);
    char storage_args[2 * (BUF_SIZE + CACHE_SIZE)];
    va_list args;
    va_start(args, message);
    vsprintf (storage_args, with_prefix, args);
    va_end(args);
//    openlog (PROJECT_NAME, LOG_CONS | LOG_PID, LOG_DAEMON);

// syslog 一般来说是线程安全函数
    syslog (logLevel | LOG_PID, "%s", storage_args);
//    closelog ();
// printf 可能造成多线程输出混乱, 这里只是测试使用
    printf ("%s\n", storage_args);

}