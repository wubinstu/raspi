#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main (int argc, const char* argv[])
{
    int serv_sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf ("serv_sock: %d\n", serv_sock);
    struct sockaddr_in serv_addr, clnt_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons (9190);
    int opt = 1;
    int status = setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,(void *)&opt,(socklen_t)sizeof(opt));
    if (bind (serv_sock, (struct sockaddr*)&serv_addr, sizeof (serv_addr)) < 0)
        printf ("bind() error\n");
    if (listen (serv_sock, 5) < 0)
        printf ("listen() error\n");

    socklen_t clnt_addr_len;
    int clnt_sock = accept (serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
    if (clnt_sock < 0)
        printf ("accept error\n");
    printf ("clnt_sock: %d\n", clnt_sock);

    char buf[1000];
    int read_size = read (clnt_sock, buf, sizeof (buf));
    puts (buf);
    close (serv_sock);
    close (clnt_sock);
    return 0;
}