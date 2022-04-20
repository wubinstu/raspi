#include "msg.h"

void errors(const char * message)
{
    fputs("[\033[0;31m  ERROR  \033[0m] ",stderr);
    fputs(message,stderr);
}

void warnings(const char * message)
{
    fputs("[\033[0;33m WARNING \033[0m] ",stderr);
    fputs(message,stderr);
}

void success(const char * message,...)
{
    char sprint_buf[BUF_SIZE + BUF_SIZE]={'\0'};
    va_list args;
    va_start(args,message);
    vsprintf(sprint_buf,message,args);
    va_end(args);
    fputs("[\033[0;32m SUCCESS \033[0m] ",stdout);
    fputs(sprint_buf,stdout);
}

void perr(bool condition,bool isEmerge,const char * msg)
{
    if(condition)
    {
        if(errno != 0)
        {
            if(isEmerge) 
            {
                errors(strerror(errno));
                printf("\n");
                errors(msg);
            }
            else 
            {
                warnings(strerror(errno));
                printf("\n");
                warnings(msg);
            }
        }
        if(isEmerge) exit(-1);
        printf("\n");
    }
    errno = 0;
}
