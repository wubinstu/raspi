
#ifndef __SOCKET_FD_H_
#define __SOCKET_FD_H_

#include "head.h"

/** set log level */
extern void setSockFdLogLevel (int logLevel);

/** Convert domain name to IPv4 address */
extern char *NameToHost (char *domain);

/** Create a server socket and return it
 * If an error occurs, return -1 */
extern int creatServSock (unsigned long ip_addr, unsigned short host_port, int listen_queue);

/** Specify the address and port, try to connect and return the socket */
extern int connectServ (unsigned long ip_addr, unsigned short host_port);

/** Accept the client connection request and return the client socket
 * and save the client address information in the structure */
extern int acceptClnt (int server_fd, struct sockaddr_in *clnt_addr);

/** Sets the socket to skip the "time to wait" state when disconnected */
extern void sockReuseAddr (int fd);

/** Using Nagle algorithm */
extern void sockNagle (int fd);

/** Sets whether the socket has the "flag" attribute and return flag */
extern int setSockFlag (int fd, int flags, bool isTrue);

/** Check whether the file descriptor is valid  */
extern bool checkFd (int fd);

#endif//__SOCKET_FD_H_