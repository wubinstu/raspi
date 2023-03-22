
#include "global.h"
#include "socket_fd.h"
#include "log.h"

void setSockFdLogLevel (int logLevel)
{
    if (logLevel >= 0 && logLevel <= 7)
        socket_fd_logLevel = logLevel;
}

char * NameToHost (char * domain)
{
    struct hostent * host = gethostbyname (domain);
    perr (host == NULL, socket_fd_logLevel,
          "function gethostbyname returns NULL when called NameToHost");
    return inet_ntoa (* (struct in_addr *) host->h_addr_list[0]);
}

int creatServSock (unsigned long ip_addr, unsigned short host_port, int listen_queue)
{
    int status;
    // to accept function's return value
    int serv = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv == -1)
    {
        perr (true, socket_fd_logLevel,
              "function socket returns -1 when called createServSock");
        return -1;
    }
    // create a tcp socket

    struct sockaddr_in serv_addr;
    // to storage server addr info
    memset (& serv_addr, 0, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ip_addr;//htonl(ip_addr);
    serv_addr.sin_port = htons (host_port);
    status = bind (serv, (struct sockaddr *) & serv_addr, sizeof (serv_addr));
    if (status == -1)
    {
        perr (true, socket_fd_logLevel,
              "function bind returns -1 when called createServSock");
        return -1;
    }
    // bind addr info to socket

    status = listen (serv, listen_queue);
    if (status == -1)
    {
        perr (true, socket_fd_logLevel,
              "function listen returns -1 when called createServSock");
        return -1;
    }
    // SERVER socket created!
    perr (true, LOG_INFO,
          "Server Socket Created, fd = %d, port = %d",
          serv, ntohs (serv_addr.sin_port));

    return serv;
}

int connectServ (unsigned long ip_addr, unsigned short host_port)
{
    int status;
    // to accept function's return value
    int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        perr (true, socket_fd_logLevel,
              "function socket returns -1 when called connectServ");
        return sock;
    }
    // create a tcp socket

    struct sockaddr_in addr;
    // to storage server addr info
    socklen_t addr_size = sizeof (addr);
    memset (& addr, 0, (size_t) addr_size);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip_addr;
    addr.sin_port = htons (host_port);
    // send connection request
    status = connect (sock, (struct sockaddr *) & addr, addr_size);
    if (status == -1)
    {
        perr (true, socket_fd_logLevel,
              "function connect returns -1 when called connectServ");
        return status;
    }
    perr (true, LOG_INFO,
          "Successfully connected to server[%d] %s:%d",
          sock, inet_ntoa (addr.sin_addr), ntohs (addr.sin_port));

    return sock;
}

int acceptClnt (int server_fd, struct sockaddr_in * clnt_addr)
{
    socklen_t clnt_addr_size = sizeof (* clnt_addr);

    int clnt = accept (server_fd, (struct sockaddr *) & clnt_addr, & clnt_addr_size);
    perr (clnt == -1, socket_fd_logLevel,
          "function accept returns -1 when called acceptClnt");

    return clnt;
}

int createServEpoll (int sock_fd)
{
    int epfd = epoll_create1 (0);
    if (epfd == -1)
    {
        perr (true, LOG_WARNING,
              "function epoll_create1 returns -1 when called createServEpoll");
        return -1;
    }
    setSockFlag (sock_fd, O_NONBLOCK, true);
    struct epoll_event ev;
    ev.data.fd = sock_fd;
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl (epfd, EPOLL_CTL_ADD, sock_fd, & ev) == -1)
    {
        perr (true, LOG_WARNING,
              "function epoll_ctl returns -1 when called createServEpoll");
        close (epfd);
        return -1;
    }
    return epfd;
}

void sockReuseAddr (int fd)
{
    int opt = 1;
    int status = setsockopt
            (fd, SOL_SOCKET, SO_REUSEADDR, (void *) & opt, (socklen_t) sizeof (opt));
    perr (status == -1, socket_fd_logLevel,
          "function setsockopt returns -1 when called sockReuseAddr");
}

void sockNagle (int fd)
{
    int opt = 1;
    int status = setsockopt
            (fd, IPPROTO_TCP, TCP_NODELAY, (void *) & opt, (socklen_t) sizeof (opt));
    perr (status == -1, socket_fd_logLevel,
          "function setsockopt returns -1 when called sockNagle");
}

int setSockFlag (int fd, int flags, bool isTrue)
{
    int val;
    if ((val = fcntl (fd, F_GETFL, 0)) < 0)
        perr (true, socket_fd_logLevel,
              "function fcntl F_GETFL error when called set_fl");
    if (isTrue) val |= flags;//set
    else val &= ~flags;//clear
    if (fcntl (fd, F_SETFL, val) < 0)
        perr (true, socket_fd_logLevel,
              "function fcntl F_SETFL error when called set_fl");
    return val;
}

bool checkFd (int fd)
{
    if (fcntl (fd, F_GETFL, 0) == -1)
        return false;
    else return true;
}