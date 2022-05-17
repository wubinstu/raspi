#include "raspi.h"
#include "head.h"
#include "myprocess.h"
#include "msgd.h"
#include "mythread.h"
#include "myssl.h"
#include "types.h"

/// Used For Configuration File
confOpt co;

/// server fd
int serv_fd = -1;
SSL * ssl_serv_fd = NULL;
/// certificate
SSL_CTX * ctx = NULL;

/// runtime mode
bool mode_daemon = false;
bool mode_strict = false;

/// thread variable
//pthread_t thread_sender;
pthread_t thread_cheker = 0;

/// recovery buffer
jmp_buf myjmp;


// TODO
// 1. [DONE] enable ssl transmission
// 2. [IGNORE] set socket to no blocking mode
// 3. [DONE] Recover in case of network error or server send disconnect (alram read)
// 4. [IGNORE] fd manage
// 5. [DONE] Supports the use of domain names instead of server IPv4 addresses (DNS)
// 6. [DONE] Systemd Unit service file
// 7. [DONE] Create a independent thread for data collection
// 8. [DONE] flash led, Set different meanings for LED lights
// 9. [IGNORE] thread pause resume
// 10. [DONE] compact exit
// 11. [DONE] deal with runtime args
// 12. [WAITING] Create a Controler Program
// 13. [DONE] package configuration file's global variable to structural

// problems:
// 1. [SOLVED] longjmp thread undefined
// 2. [WAITING] segmentation fault when exit

int main(int argc,const char * argv[])
{

	// init wiringPi lib
	initPi();
	// Check Runtime parameters
	dealWithArgs (argc,argv);

	if(mode_strict)
	{
		// Check whether you have root privileges
		if(!check_permission ("To create pid file"))
			return -1;
		// Ensure that only one program runs at the same time according to the PID file
		check_running();
	}
	if(mode_daemon)
		daemonize (PROJECT_NAME);
	// Set up an archive point
	int jmp_rtn = setjmp(myjmp);
	// Register signal processing function
	sig_reg();
	// Read configuration file to global variable
	conf2var();
	
	if(mode_strict)
	{
		int flag;
		ctx = initSSL(client);
		if(ctx == NULL) my_exit();
		flag = loadCA (ctx,co.CAfile);
		if(!flag) my_exit();
		if(co.checkMe)
		{
			flag = loadCert (ctx,co.UCert);
			if(!flag) my_exit();
			
			flag = loadKey (ctx,co.UKey);
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

	
	if(mode_strict)
	{
		ssl_serv_fd = SSL_fd (ctx,serv_fd);
		if(ssl_serv_fd == NULL) my_exit();
		int SSL_handShake = SSL_connect (ssl_serv_fd);
		if (SSL_handShake == -1)
		{
			perr_d (true,LOG_ERR,"Server authentication failed, connection disconnected");
			my_exit();
		}
		if(co.checkMe)
		{
			printf ("Self Cert:\n");
			showSelfCert (ssl_serv_fd);
		}
		printf ("Peer Cert:\n");
		showPeerCert (ssl_serv_fd);
	}
	
	if(jmp_rtn != RESET)
		pthread_create (&thread_cheker, NULL, check_monit, NULL);
	
	sendData (LED_YEL);
	
}