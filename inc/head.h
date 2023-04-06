#ifndef __HEAD_H_
#define __HEAD_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <error.h>
#include <errno.h>
#include <syslog.h>
#include <setjmp.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

// socket 网络套接字缓冲区大小
//发送缓冲区大小 = 带宽 * 往返时延
//接收缓冲区大小 = 发送缓冲区大小 * 2
#define SOCK_SND_BUF_SIZE   64 * 1024
#define SOCK_RCV_BUF_SIZE   128 * 1024

#endif