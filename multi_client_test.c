//
// Created by Einc on 2023/03/28.
//

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "signal.h"

int main (int argc, char * argv[])
{
    if (argc < 3)
    {
        printf ("usage: <multi-num> <client_path> [client_args]\n");
        return -1;
    }
    printf ("敬告: 此程序仅供测试使用, 若被多开的客户端软件存在操作相同软件/硬件资源将会引发未定义的后果!\n");
    printf ("输入[Y/n]: ");
    char c = getchar ();
    if (c == 'Y' || c == 'y');
    else if (c == 'N' || c == 'n')
        exit (0);
    else exit (-1);

    int num = (int) strtol (argv[1], NULL, 10);
    if (num <= 0)
    {
        printf ("multi-num error\n");
        return -1;
    } else if (num > 1000)
    {
        printf ("Warning: You set a big number!\n");
        return -1;
    }

    pid_t pid[num];
    for (int i = 0; i < num; i++)
    {
        printf ("forking NO.%d process\n", i);
        pid[i] = fork ();
        if (pid[i] == 0)
        {
            close (0);
            close (1);
            close (2);
            execv (argv[2], & argv[2]);
            exit (-1);  // 确保安全
        }
        sleep (1);
    }

    printf ("Kill process After 10 secs...\n");
    sleep (10);

    for (int i = 0; i < num; i++)
    {
        printf ("killing NO.%d process\n", i);
        kill (pid[i], SIGINT);
        sleep (1);
    }

}