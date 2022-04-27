//
// Created by Einc on 2022/4/9.
//
#include "head.h"
//#include "msg.h"
#include "types.h"
#include "myssl.h"


int serv_sock;
int clnt_sock;

bool mode_strict = false;
bool mode_check_clnt = false;

SSL * ssl_clnt_fd[__FD_SETSIZE / __NFDBITS];
SSL_CTX * ctx;

#define UCert "/home/pi/raspi/keys/server.crt"
#define UKey "/home/pi/raspi/keys/server.key"
#define CAfile "/home/pi/raspi/keys/ca.crt"

monit mn;
fd_set reads,copy_reads;
int fd_max;

struct timeval timeout;

socklen_t addr_size;
long str_len,fd_num,state;
char text[BUF_SIZE];
struct sockaddr_in serv_addr,clnt_addr;
unsigned short serv_port = 9190;

void stop(int signo)
{
	printf(" stop called\n");
	copy_reads = reads;
	for(int i=0;i<fd_max+1;i++)
	{
		if(FD_ISSET(i,&copy_reads))
		{
			if(mode_strict)
			{
				SSL_write (ssl_clnt_fd[i],"FIN",4);
				SSL_shutdown (ssl_clnt_fd[i]);
				SSL_free (ssl_clnt_fd[i]);
			}
			else write(i,"FIN",4);
			close(i);
		}
	}
	if(mode_strict) SSL_CTX_free (ctx);
	close(serv_sock);
	// signal(SIGINT,stop);
	exit(0);
}


void handler()
{
	FD_ZERO(&reads);
	FD_SET(serv_sock,&reads);
	fd_max = serv_sock;
	while (true)
	{
		copy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		fd_num = select(fd_max+1,&copy_reads,0,0,&timeout);//It's Also fine set timeout as NULL
		if(fd_num == -1) {printf("Function select() returns -1\n");break;}
		else if(fd_num == 0) continue;
		for(int i=0;i<fd_max+1;i++)
		{
			
			if(FD_ISSET(i,&copy_reads))
			{
				if(i == serv_sock)
				{
					addr_size = sizeof(clnt_addr);
					clnt_sock = accept(serv_sock,(struct sockaddr *)&clnt_addr,&addr_size);
					if(clnt_sock == -1) {printf("Function accept() returns -1\n");exit(-1);}

					FD_SET(clnt_sock,&reads);
					if(mode_strict)
					{
						ssl_clnt_fd[clnt_sock] = SSL_fd (ctx,clnt_sock);
						SSL_accept(ssl_clnt_fd[clnt_sock]);
						if(mode_check_clnt)
							showPeerCert (ssl_clnt_fd[clnt_sock]);
					}
					if(fd_max < clnt_sock) fd_max = clnt_sock;
					sprintf(text,"Connected Client: %d\n",clnt_sock);
					printf("%s",text);
				}
				else
				{
					memset(&mn,0,sizeof(mn));
					if(mode_strict)
						str_len = SSL_read (ssl_clnt_fd[i],&mn, sizeof(mn));
					else str_len  = read(i,&mn,sizeof(mn));//i -> clnt_sock
					
//					if(mn.distance == 0) continue;
					if(str_len == sizeof(mn))
					{
						printf ("Data From Client %d:\n",i);
						printf("cpu temp = %.2fC\n",mn.cpu_temper);
						printf("distance = %.2fcm\n",mn.distance);
						printf("env temp = %.2fC\n",mn.env_temper);
						printf("env humid = %.2f\n\n",mn.env_humidity);
					}

					if(str_len == -1) {printf("Function read() returns -1\n");exit (-1);}
					else if(str_len == 0 || str_len == 4)
					{
						FD_CLR(i,&reads);
						if(mode_strict)
						{
							SSL_shutdown (ssl_clnt_fd[i]);
							SSL_free (ssl_clnt_fd[i]);
						}
						close(i);
						sprintf(text,"[ %s ] Closed Client: %d\n",(char *)&mn,i);
						printf("%s",text);
					}
					else
					{
						if(mode_strict)
							state = SSL_write (ssl_clnt_fd[i],"CON",4);
						else state = write(i,"CON",4);
						if(state == -1) {printf("Function write() returns -1\n");exit(-1);}
					}
				}
			}
		}
	}
}

int main(int argc,char * argv[])
{

	struct sigaction act;
	memset(&act,0,sizeof(struct sigaction));
	act.sa_handler = stop;
	sigaction(SIGINT,&act,NULL);


	if(argc < 2)
	{
		printf("Need More Args\n");
		return -1;
	}
	for (int i = 1;i < argc;i++)
	{
		if(strcmp (argv[i],"--port") == 0)
		{
			if(argv[i+1] != NULL)
			{
				serv_port = strtol (argv[i+1],NULL,10);
				i++;
			}
			else
			{
				printf ("Missing Port\n");
				return -1;
			}
			continue;
		}
		if(strcmp (argv[i],"--strict") == 0)
		{
			mode_strict = true;
			continue;
		}
		if(strcmp (argv[i],"--check-clnt") == 0)
		{
			mode_check_clnt = true;
			continue;
		}
		if(strcmp (argv[i],"--help") == 0)
		{
			printf ("Usage:  [--port] \tSpecify service port(default:9190)"
					"\n\t[--strict] \tenable strict mode,using SSL transmission"
					"\n\t[--check-clnt] \tCheck client identity\n");
			return 0;
		}
		else
		{
			printf ("Unknown Parameter, Use \"--help\" for help information\n");
			return -1;
		}
	}
	if(mode_strict)
	{
		int flag;
        ctx = initSSL (server);
        if(ctx == NULL) {printf("init SSL failed\n");exit(-1);}
		flag = loadCert (ctx,UCert);
		if(!flag) {printf("User Cert Load failed\n");exit (-1);}
		printf("User Cert %s load complete\n",UCert);
		flag = loadKey (ctx,UKey);
		if(!flag) {printf("User Private Key Load failed\n");exit (-1);}
		printf("User Private Key %s load complete\n",UKey);
		flag = checkKey (ctx);
		if(!flag) {printf("User Private Key and Cert does NOT match\n");exit (-1);}
		printf("User Private Key and Cert check complete\n");
		
		if(mode_check_clnt)
		{
			flag = loadCA (ctx,CAfile);
			if(!flag) {printf("CA file Load failed\n");exit(-1);}
			printf("CA Cert %s load complete\n",CAfile);
		}
	}



	serv_sock = socket(PF_INET,SOCK_STREAM,0);
	if(serv_sock == -1) {printf("Function socket() returns -1\n");exit(-1);}
 
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serv_port);

	state = bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	if(state == -1) {printf("Function bind() returns -1\n");exit(-1);}

	state = listen(serv_sock,5);
	if(state == -1) {printf("Function listen() return -1\n");exit(-1);}
	else {printf("The Server Socket Created\n");printf("Server Port: %d\n",serv_port);}
	
	
	handler();
	close (serv_sock);
	
	/*
	socklen_t clnt_addr_size = sizeof (clnt_addr);
	clnt_sock = accept (serv_sock,(struct sockaddr *)&clnt_addr,&clnt_addr_size);
	if(clnt_sock == -1) errors("Function accept() return -1");
	

	ssl_clnt_fd = SSL_fd (ctx,clnt_sock);
	SSL_accept (ssl_clnt_fd);
	
	printf ("ssl accept\n");
	
	showPeerCert (ssl_clnt_fd);
	
	while (1)
	{
		memset(&mn,0,sizeof(mn));
		str_len = SSL_read (ssl_clnt_fd,&mn, sizeof(mn));
		
		if(mn.distance == 0 && mn.cpu_temper == 0 && mn.env_temper == 0 && mn.env_humidity == 0)
			continue;
		
		printf("cpu temp = %.2fC\n",mn.cpu_temper);
		printf("distance = %.2fcm\n",mn.distance);
		printf("env temp = %.2fC\n",mn.env_temper);
		printf("env humid = %.2f\n\n\n",mn.env_humidity);
		
		if(str_len <= 0)
		{
			printf ("read finish\n");
			break;
		}
		
		SSL_write (ssl_clnt_fd,"CON",4);
		
	}
	 
	 */
	

//	 pause();
//	 pthread_t t1;
//	 pthread_create(&t1,NULL,proccess,NULL);
//
//
//	 pthread_join(t1,NULL);

//	set_fl_d(clnt_sock,O_NONBLOCK,true);

//	p1 = fork();
//	if(p1 == 0)
//	{
//		signal(SIGINT,stop);
//		handler();
//	}
//	else
//	{
//		signal(SIGINT,stop);
//		while (true)
//		{
//			pause();
//		}
//	}
	
//	SSL_shutdown (ssl_clnt_fd);
//	SSL_free (ssl_clnt_fd);
//	SSL_CTX_free (ctx);

//	close(serv_sock);
	return 0;
}