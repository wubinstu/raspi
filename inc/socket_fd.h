
#ifndef __SOCKET_FD_H_
#define __SOCKET_FD_H_

#include "head.h"

/** set log level */
extern void setSockFdLogLevel (int logLevel);

/** Convert domain name to IPv4 address */
extern char * NameToHost (char * domain);

/** Create a server socket and return it
 * If an error occurs, return -1 */
extern int createServSock (server_info_t * serverInfo);

/** Specify the address and port, try to connect and return the socket */
extern int connectServ (server_info_t * serverInfo);

/** Accept the client connection request and return the client socket
 * and save the client address information in the structure */
extern int acceptClnt (int server_fd, struct sockaddr_in * clnt_addr);

/** create a epoll fd with server socket fd */
extern int createServEpoll (int sock_fd);

/** Sets the socket to skip the "time to wait" state when disconnected */
extern void sockReuseAddr (int fd);

extern void sockReusePort (int fd);

/** set tcp socket keep alive */
extern void sockKeepAlive (int fd);


extern void sockNagle (int fd);

/** Using Nagle algorithm */
extern void sockNoNagle (int fd);

/** Sets whether the socket has the "flag" attribute and return flag */
extern int setSockFlag (int fd, int flags, bool isTrue);

/** Set socket buffer */
extern bool setSockBufSize (int fd, int snd_buf_len, int rcv_buf_len);

/** Check whether the file descriptor is valid  */
extern bool checkFd (int fd);

#endif//__SOCKET_FD_H_