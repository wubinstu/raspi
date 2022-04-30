#include "mysockd.h"
#include "msgd.h"

/// Default LOG_LEVEL : LOG_WARNING
static int mysockd_logLevel = 4;

void setMysockdLogLevel(int logLevel)
{
    mysockd_logLevel = logLevel;
}

char * NameToHost_d(char * domain)
/// use gethostbyname to conv domain name to ip addr
{
    struct hostent * host;
    host = gethostbyname(domain);
    perr_d(host == NULL,mysockd_logLevel,"function gethostbyname returns NULL when called NameToHost");
    return inet_ntoa(*(struct in_addr *) host -> h_addr_list[0]);
}

int creatServSock_d(unsigned long ip_addr,unsigned short host_port,int listen_queue)
{
    int status;
    // to accept function's return value
    int serv = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(serv == -1)
    {
    	perr_d(true,mysockd_logLevel,"function socket returns -1 when called createServSock");
	    return -1;
    }
    // create a tcp socket

    struct sockaddr_in serv_addr;
    // to storage server addr info
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ip_addr;//htonl(ip_addr);
    serv_addr.sin_port = htons(host_port);
    status = bind(serv,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    if(status == -1)
    {
    	perr_d(true,mysockd_logLevel,"function bind returns -1 when called createServSock");
	    return -1;
    }
    // bind addr info to socket

    status = listen(serv,listen_queue);
    if(status == -1)
    {
    	perr_d(true,mysockd_logLevel,"function listen returns -1 when called createServSock");
	    return -1;
    }
    // SERVER socket created!
	perr_d (true,LOG_INFO,"Server Socket Created, fd = %d, port = %d",serv, ntohs (serv_addr.sin_port));

    return serv;
}

int connectServ_d(unsigned long ip_addr,unsigned short host_port)
{
    int status;
    // to accept function's return value
    int sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sock == -1)
    {
    	perr_d(true,mysockd_logLevel,"function socket returns -1 when called connectServ");
	    return -1;
    }
    // create a tcp socket

    struct sockaddr_in addr;
    // to storage server addr info
    socklen_t addr_size = sizeof(addr);
    memset(&addr,0,(size_t)addr_size);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip_addr;
    addr.sin_port = htons(host_port);
    // send connection request
    status = connect(sock,(struct sockaddr *)&addr,addr_size);
    if(status == -1)
    {
    	perr_d(true,mysockd_logLevel,"function connect returns -1 when called connectServ");
	    return -1;
    }
    perr_d (true,LOG_INFO,"Successfully connected to server[%d] %s:%d",sock, inet_ntoa (addr.sin_addr), ntohs (addr.sin_port));

    return sock;
}

int acceptClnt_d(int server_fd,struct sockaddr_in * clnt_addr)
{
    socklen_t clnt_addr_size = sizeof(*clnt_addr);

    int clnt = accept(server_fd,(struct sockaddr *)&clnt_addr,&clnt_addr_size);
    perr_d(clnt == -1,mysockd_logLevel,"function accept returns -1 when called acceptClnt");

    return clnt;
}

void sockReuseAddr_d(int serv_fd)
/// ignore Time-to-wait
{
    int opt = 1;
    int status = setsockopt(serv_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&opt,(socklen_t)sizeof(opt));
    perr_d(status == -1,mysockd_logLevel,"function setsockopt returns -1 when called sockReuseAddr");
}

void sockNagle_d(int fd)
/// nagle, not recommended when you trans big file
{
    int opt = 1;
    int status = setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(void *)&opt,(socklen_t)sizeof(opt));
    perr_d(status == -1,mysockd_logLevel,"function setsockopt returns -1 when called sockNagle");
}

int set_fl_d(int fd,int flags,bool isTrue)
/// Set fd's Properties
{
	int val;
	if((val = fcntl(fd,F_GETFL,0)) < 0)
		perr_d(true,mysockd_logLevel,"function fcntl F_GETFL error when called set_fl");
	if(isTrue) val |= flags;//set
	else val &= ~flags;//clear
	if(fcntl(fd,F_SETFL,val) < 0)
		perr_d(true,mysockd_logLevel,"function fcntl F_SETFL error when called set_fl");
	return val;
}

bool check_fd(int fd)
{
	if(fcntl (fd,F_GETFL,0) == -1)
		return false;
	else return true;
}