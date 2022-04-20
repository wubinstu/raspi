#include "raspi.h"
#include "head.h"
#include "myprocess.h"
#include "msgd.h"
#include "mythread.h"
#include "myssl.h"
#include "conf.h"

unsigned long serv_ip = 0;
unsigned short serv_port = 0;
int interval = 10;
int frectime = 42;
int frecatps = 7;
bool checkMe = false;
char CAfile[256] = {0};
char UCert[256] = {0};
char UKey[256] = {0};
int serv_fd = -1;
SSL * ssl_serv_fd = NULL;
SSL_CTX * ctx = NULL;

bool mode_daemon = false;
bool mode_strict = false;

//pthread_t thread_sender;
pthread_t thread_cheker;


jmp_buf myjmp;

// TODO
// -1. ssl
// -2. socket no waiting
// -3. server first stop(alram read)
// -4. fd mg
// -5. DNS
// -6. service file
// -7. data thread
// -8. flash led
// --9. thread pause resume
// all done

// problem:
// 1. longjmp thread undefined

int main(int argc,const char * argv[])
{
	
	// Check Runtime parameters
	for(int i = 0;i < argc;i++)
	{
		if(strcmp (argv[i],"--daemon") == 0)
			mode_daemon = true;
		else if(strcmp (argv[i],"--strict") == 0)
			mode_strict = true;
		else if(strcmp (argv[i],"--clean") == 0)
		{
			printf ("%s will be deleted\n",PID_FILE);
			unlink (PID_FILE);
			exit (0);
		}
		else if(strcmp (argv[i],"--default-conf") == 0)
		{
			printf ("reset %s\n",CONF_FILE);
			unlink (CONF_FILE);
			defaultconf (CONF_FILE);
			exit (0);
		}
		else if(strcmp (argv[i],"--settings") == 0)
			execlp ("vim","vim",CONF_FILE,NULL);
		
		else if(strcmp (argv[i],"--help") == 0)
		{
			printf ("Usage: [--daemon] [--strict] [--clean] [--default-conf] [--settings]");
			exit (0);
		}
	}
	printf ("daemonMode = %d,strictMode = %d\n",mode_daemon,mode_strict);
	if(mode_daemon)
		daemonize (PROJECT_NAME);
	
	if(mode_strict)
	{
		// Check whether you have root privileges
		if(geteuid() != 0)
		{
			printf ("root privileges are required!\n");
			return -1;
		}
		// Ensure that only one program runs at the same time according to the PID file
		check_running();
	}
	// init wiringPi lib
	initPi();
	// Set up an archive point
	int jmp_rtn = setjmp(myjmp);
	// Register signal processing function
	sig_reg();
	// Read configuration file to global variable
	conf2var();
	
	if(mode_strict)
	{
		int flag;
		ctx = initSSL("client");
		if(ctx == NULL) my_exit();
		flag = loadCA (ctx,CAfile);
		if(!flag) my_exit();
		if(checkMe)
		{
			flag = loadCert (ctx,UCert);
			if(!flag) my_exit();
			
			flag = loadKey (ctx,UKey);
			if(!flag) my_exit();
			
			flag = checkKey (ctx);
			if(!flag) my_exit();
		}
	}

	// Trying to connect to the server
	int con_rtn = tryconnect(LED_RED);
	if(con_rtn == -1)
	{
		perr_d (true,LOG_ERR,"Maximum number of reconnections exceeded");
		my_exit();
	}
	if(jmp_rtn != RESET)
		pthread_create (&thread_cheker, NULL, check_monit, NULL);
	
	if(mode_strict)
	{
		ssl_serv_fd = SSL_fd (ctx,serv_fd);
		if(ssl_serv_fd == NULL) my_exit();
		SSL_connect (ssl_serv_fd);
		if(checkMe)
		{
			printf ("Self Cert:\n");
			showSelfCert (ssl_serv_fd);
		}
		printf ("Peer Cert:\n");
		showPeerCert (ssl_serv_fd);
	}
	sendData (LED_YEL);
	
}