#include "sundry.h"

void procbar(int m)
{
    if(m >= 0 && m <= 100)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        char bar[50] = {0};
        char * lab = "-\\|/";
        for(int i = 0;i < m/2;i++)
            bar[i] = '#';
        bar[m/2+1] = '\0';
        printf ("[ %-50s ][ %.3d%% ][ %c ]\r",bar,m,lab[tv.tv_sec%4]);
        fflush(stdout);
    }
    //else exit(-1);
}

void procbar_f(float m,bool state)
{
    if(m >= 0 && m <= 100.01)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        char bar[51] = {0};
        char * lab = "-\\|/";
        char text[150];
        memset(text,0,150);
        for(int i = 0;i < (int)(m/2);i++)
            bar[i] = '=';
        if((int)m % 2)
        {
            bar[(int)(m/2)] = '-';
            bar[(int)(m/2) + 1] = '\0';
        }
        else
            bar[(int)(m/2)] = '\0';
        if(state == false)
        {
            if((int)m == 100)
                state = true;
            else
                sprintf(text,"\a[\033[0;33m%-50s\033[0m][ %.3d.%.2d%% ][ \033[0;31m\033[5m%s\033[0m ]        \r\033[?25h",bar,(int)m,(int)((m - (int)m) * 100),"Terminated");
        }
        if(state)
        {
            if((int)m == 100)
                sprintf(text,"[%-50s][ %.3d.%.2d%% ][ \033[0;32m%s\033[0m ]\033[0m        \r\033[?25h",bar,(int)m,(int)((m - (int)m) * 100),"OK");
            else
                sprintf(text,"\033[?25l[\033[0;32m%-50s\033[0m][ %.3d.%.2d%% ][ %c ]            \r",bar,(int)m,(int)((m - (int)m) * 100),lab[(tv.tv_sec)%4]);
        }
        //fflush(stdout);
        printf("%s",text);
        fflush(stdout);
    }
}

bool isDebugger()
{
    if(ptrace(PTRACE_TRACEME,0,0,0) == -1)
        return true;
    else return false;
}