
#ifndef __MYSOCK_H_
#define __MYSOCK_H_

#include "head.h"

/** Convert domain name to IPv4 address */
extern unsigned long NameToHost(char * domain,bool show_details);

/** Create a server socket and return it
 * If an error occurs, program will stop */
extern int creatServSock(unsigned long ip_addr,unsigned short host_port,int listen_qeue);

/** Specify the address and port, try to connect and return the socket */
extern int connectServ(unsigned long ip_addr,unsigned short host_port);

/** Accept the client connection request and return the client socket
 * and save the client address information in the structure */
extern int acceptClnt(int server_fd,struct sockaddr_in * clnt_addr);

/** Sets the socket to skip the "time to wait" state when disconnected */
extern void sockReuseAddr(int serv_fd);

/** Using Nagle algorithm */
extern void sockNagle(int fd);


#endif