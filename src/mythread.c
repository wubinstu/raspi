//
// Created by Einc on 2022/4/4.
//

#include "mythread.h"
#include "types.h"
#include "conf.h"
#include "msgd.h"
#include "mysockd.h"
#include "raspi.h"
#include "myprocess.h"
#include "myssl.h"


extern unsigned long serv_ip;
extern unsigned short serv_port;
extern int interval;
extern int frectime;
extern int frecatps;
extern char CAfile[256];
extern char UCert[256];
extern char UKey[256];
extern bool checkMe;
extern int serv_fd;
extern SSL * ssl_serv_fd;
extern SSL_CTX * ctx;
extern bool mode_strict;
extern jmp_buf myjmp;

static monit mn;
pthread_mutex_t mutex_monit = PTHREAD_MUTEX_INITIALIZER;


void conf2var()
{
	errno = 0;
	perr_d(true,LOG_INFO,"Reading conf: %s",CONF_FILE);
	if(!checkconf (CONF_FILE))
		my_exit();
	LNode conf;
	conf = readconf (CONF_FILE);
	checkread (conf);
	
	Elem  e[LengthOfLinkList (conf)];
	int len = ListToArry (conf,e);
	
	
	for(int i = 0;i < len ;i++)
	{
		if(strcmp (e[i].name,"SERVADDR") == 0)
			serv_ip = inet_addr(NameToHost_d(e[i].value));
		else if(strcmp (e[i].name,"SERVPORT") == 0)
			serv_port = (unsigned short)strtol(e[i].value,NULL,10);
		else if(strcmp (e[i].name,"INTERVAL") == 0)
			interval = (int)strtol(e[i].value,NULL,10);
		else if(strcmp (e[i].name,"FRECTIME") == 0)
			frectime = (int)strtol(e[i].value,NULL,10);
		else if(strcmp (e[i].name,"FRECATPS") == 0)
			frecatps = (int)strtol(e[i].value,NULL,10);
		else if(strcmp (e[i].name,"CAFILE") == 0)
			strcpy (CAfile,e[i].value);
		else if(strcmp (e[i].name,"CHECKME") == 0)
		{
			if(strcmp (e[i].value,"true") == 0)
				checkMe = true;
			else checkMe = false;
		}
		else if(strcmp (e[i].name,"UCERT") == 0)
			strcpy (UCert,e[i].value);
		else if(strcmp (e[i].name,"UKEY") == 0)
			strcpy (UKey,e[i].value);
		else perr_d (true,LOG_NOTICE,"Unknown configure option %s=%s",e[i].name,e[i].value);
	}
	if(serv_ip == -1)
	{
		perr_d (true,LOG_ERR,"The server IP address could not be read correctly from the configuration file");
		my_exit();
	}
	if(serv_port == 0)
	{
		perr_d (true,LOG_ERR,"The server port could not be read correctly from the configuration file");
		my_exit();
	}
	if(checkMe == true && (UCert[0] == 0 || UKey[0] == 0))
	{
		perr_d (true,LOG_ERR,"User cert or key error");
		my_exit();
	}
}

int tryconnect(int led)
{
	turn_off_led (led);
	
	do
	{
		errno = 0;
		serv_fd = connectServ_d (serv_ip,serv_port);
		perr_d (true,LOG_INFO,"Trying to connect to the server...");
		if(serv_fd == -1)
		{
			sleep (frectime);
			if(frecatps-- == 0) return -1;
		}
	}while(serv_fd == -1);
	
	turn_on_led (led);
	errno = 0;
	perr_d (true,LOG_INFO,"Successfully connected to server");
	
	set_fl_d (serv_fd,O_NONBLOCK,false);
	sockNagle_d (serv_fd);
	
	return 0;
}

_Noreturn void * check_monit(void * arg)
{
	while (true)
	{
		pthread_mutex_lock (&mutex_monit);
		memset (&mn,0, sizeof (mn));
		mn.cpu_temper = read_cpu_temp();
		mn.distance = disMeasure (DISTANCE_T,DISTANCE_E);
		readSensorData (TEMP_HUMI,&mn.env_humidity,&mn.env_temper);
	
		if(mn.distance <= 30)
			turn_on_led (LED_GRE);
		else turn_off_led (LED_GRE);
		if(mn.cpu_temper >= 60 || mn.env_humidity >= 75)
		{
			turn_on_led (LED_YEL);
			perr_d (true,LOG_WARNING,"The temperature or humidity is too high");
		}
		else turn_off_led (LED_YEL);
		pthread_mutex_unlock (&mutex_monit);
		sleep (interval/2);
	}
}

_Noreturn void * sendData(int led)
{
	char status[4];
	ssize_t read_len,write_len;
	while (true)
	{
		memset (status,0, 4);
		
		// The distance cannot be 0, which means no valid data is read by default
		if(mn.distance == 0)
		{
			sleep (interval);
			continue;
		}
		pthread_mutex_lock (&mutex_monit);
		
		if(mode_strict)
			write_len = SSL_write (ssl_serv_fd,&mn,sizeof(mn));
		else write_len = write (serv_fd,&mn,sizeof(mn));
		
		pthread_mutex_unlock (&mutex_monit);
		printf ("write done. len = %zd\n",write_len);
		
		alarm (10);
		if(mode_strict)
			read_len = SSL_read (ssl_serv_fd,status,4);
		else read_len = read (serv_fd, status,4);
		alarm (0);
		flash_led (led,90);
//		puts (status);
		
		if(strcmp (status,"FIN") == 0 || read_len <= 0)
		{
			printf ("FIN received, len = %zd, wait for %d secs\n",read_len,interval);
			sleep (interval);
			reset();
		}
		
		sleep (interval);
	}
}