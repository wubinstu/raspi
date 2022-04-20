#include "mysock.h"
#include "msg.h"

unsigned long NameToHost(char * domain,bool show_details)
// use gethostbyname to conv domain name to ip addr
{
    struct hostent * host;

    host = gethostbyname(domain);
    perr(host == NULL,false,"can not find host by domain name");

    if(show_details)
    // whether or not to print details
    {
        printf("Official Name: %s\n",host -> h_name);
        // print official name

        for(int i=0;host -> h_aliases[i];i++)//last one is NULL
            printf("Aliases %d: %s\n",i+1,host -> h_aliases[i]);
        // print alias name

        if(host -> h_addrtype == AF_INET)
            printf("AddrType: AF_INET\n");
        else if(host -> h_addrtype == AF_INET6)
            printf("AddrType: AF_INET6\n");
        else
            printf("Unknown AddrType!\n");
        // print network type

        for(int i=0;host -> h_addr_list[i];i++)
            printf("IP addr %d: %s\n",i+1,inet_ntoa(*(struct in_addr *) host -> h_addr_list[i]));
        // print ip addr
    }
    return * host -> h_addr_list[0];
}

int creatServSock(unsigned long ip_addr,unsigned short host_port,int listen_queue)
{
    int status;
    // to accept function's return value
    int serv = socket(PF_INET,SOCK_STREAM,0);
    perr(serv == -1,true,"function socket returns -1");
    // create a tcp socket

    struct sockaddr_in serv_addr;
    // to storage server addr info
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(ip_addr);
    serv_addr.sin_port = htons(host_port);
    status = bind(serv,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    perr(status == -1,true,"function bind returns -1");
    // bind addr info to socket

    status = listen(serv,listen_queue);
    perr(status == -1,true,"function listen returns -1");
    // SERVER socket created!

    return serv;
}

int connectServ(unsigned long ip_addr,unsigned short host_port)
{
    int status;
    // to accept function's return value
    int sock = socket(PF_INET,SOCK_STREAM,0);
    perr(sock == -1,true,"function socket returns -1");
    // create a tcp socket

    struct sockaddr_in addr;
    // to storage server addr info
    socklen_t addr_size = sizeof(addr);
    memset(&addr,0,(size_t)addr_size);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip_addr;
    addr.sin_port = htons(host_port);

    status = connect(sock,(struct sockaddr *)&addr,addr_size);
    perr(status == -1,true,"function connect returns -1");
    // send connection request

    return sock;
}

int acceptClnt(int server_fd,struct sockaddr_in * clnt_addr)
{
    socklen_t clnt_addr_size = sizeof(*clnt_addr);

    int clnt = accept(server_fd,(struct sockaddr *)&clnt_addr,&clnt_addr_size);
    perr(clnt == -1,true,"function accept returns -1");

    return clnt;
}

void sockReuseAddr(int serv_fd)
// ignore Time-to-wait 
{
    int opt = 1;
    int status = setsockopt(serv_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&opt,(socklen_t)sizeof(opt));
    perr(status == -1,false,"function setsockopt returns -1 at sockReuseAddr");
}

void sockNagle(int fd)
// nagle, not recommended when you trans big file
{
    int opt = 1;
    int status = setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(void *)&opt,(socklen_t)sizeof(opt));
    perr(status == -1,false,"function setsockopt returns -1 at sockNagle");
}
