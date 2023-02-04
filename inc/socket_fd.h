
#ifndef __SOCKET_FD_H_
#define __SOCKET_FD_H_

#include "head.h"

/** set log level */
extern void setMysockdLogLevel (int logLevel);

/** Convert domain name to IPv4 address */
extern char *NameToHost_d (char *domain);

/** Create a server socket and return it
 * If an error occurs, return -1 */
extern int creatServSock_d (unsigned long ip_addr, unsigned short host_port, int listen_queue);

/** Specify the address and port, try to connect and return the socket */
extern int connectServ_d (unsigned long ip_addr, unsigned short host_port);

/** Accept the client connection request and return the client socket
 * and save the client address information in the structure */
extern int acceptClnt_d (int server_fd, struct sockaddr_in *clnt_addr);

/** Sets the socket to skip the "time to wait" state when disconnected */
extern void sockReuseAddr_d (int fd);

/** Using Nagle algorithm */
extern void sockNagle_d (int fd);

/** Sets whether the socket has the "flag" attribute and return flag */
extern int set_fl_d (int fd, int flags, bool isTrue);

/** Check whether the file descriptor is valid  */
extern bool check_fd (int fd);

#endif//__SOCKET_FD_H_