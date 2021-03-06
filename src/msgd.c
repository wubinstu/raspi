
#include "msgd.h"

char *
log_prefix(int logLevel)
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


void
perr_d(bool condition,int logLevel,const char * message,...)
{
    if(condition)
    {
        // The space size here is not set strictly
        char * with_prefix = calloc(1,BUF_SIZE + PREFIX_LEN + CACHE_SIZE);
        strcpy(with_prefix,log_prefix(logLevel));
        if(errno != 0)
        {
            strcat(with_prefix,"(errno) ");
            strcat(with_prefix,strerror(errno));
            strcat(with_prefix,": ");
            errno = 0;
        }
        strcat(with_prefix,message);
        char * storage_args  = calloc(2,BUF_SIZE + CACHE_SIZE);
        va_list args;
        va_start(args,message);
        vsprintf(storage_args,with_prefix,args);
        va_end(args);
        openlog(PROJECT_NAME,LOG_CONS | LOG_PID,LOG_DAEMON);
        syslog(logLevel,"%s", storage_args);
	    printf ("%s\n",storage_args);
        closelog();
        free(with_prefix);
        free(storage_args);
//        if(logLevel <= LOG_ERR)
//            exit(logLevel);
	    return ;
    }
    else return ;
}