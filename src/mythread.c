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
#include "arr.h"


extern confOpt co;

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
		{
			co.serv_ip = inet_addr(NameToHost_d(e[i].value));
			continue;
		}
		else if(strcmp (e[i].name,"SERVPORT") == 0)
		{
			co.serv_port = (unsigned short)strtol(e[i].value,NULL,10);
			continue;
		}
		else if(strcmp (e[i].name,"INTERVAL") == 0)
		{
			co.interval = (int)strtol(e[i].value,NULL,10);
			continue;
		}
		else if(strcmp (e[i].name,"FRECTIME") == 0)
		{
			co.frectime = (int)strtol(e[i].value,NULL,10);
			continue;
		}
		else if(strcmp (e[i].name,"FRECATPS") == 0)
		{
			co.frecatps = (int)strtol(e[i].value,NULL,10);
			continue;
		}
		else if(strcmp (e[i].name,"CAFILE") == 0)
		{
			strcpy (co.CAfile,e[i].value);
			continue;
		}
		else if(strcmp (e[i].name,"CHECKME") == 0)
		{
			lowerConversion (e[i].value);
			if(strcmp (e[i].value,"true") == 0)
				co.checkMe = true;
			else if(strcmp (e[i].value,"false") == 0)
				co.checkMe = false;
			else
				perr_d (true,LOG_NOTICE,"Unknown configure option %s=%s",e[i].name,e[i].value);
			continue;
		}
		else if(strcmp (e[i].name,"UCERT") == 0)
		{
			strcpy (co.UCert,e[i].value);
			continue;
		}
		else if(strcmp (e[i].name,"UKEY") == 0)
		{
			strcpy (co.UKey,e[i].value);
			continue;
		}
		else perr_d (true,LOG_NOTICE,"Unknown configure option %s=%s",e[i].name,e[i].value);
	}
	if(co.serv_ip == -1)
	{
		perr_d (true,LOG_ERR,"The server IP address could not be read correctly from the configuration file");
		my_exit();
	}
	if(co.serv_port == 0)
	{
		perr_d (true,LOG_ERR,"The server port could not be read correctly from the configuration file");
		my_exit();
	}
	if(co.checkMe == true && (strlen(co.UCert) == 0 || strlen(co.UKey) == 0))
	{
		perr_d (true,LOG_ERR,"User cert or key not specified");
		my_exit();
	}
}

int tryconnect(int led)
{
	errno = 0;
	turn_off_led (led);
	do
	{
		perr_d (true,LOG_INFO,"Trying to connect to the server...");
		serv_fd = connectServ_d (co.serv_ip,co.serv_port);
		if(serv_fd == -1)
		{
			if(co.frecatps-- == 0) return -1;
			perr_d (true,LOG_NOTICE,"Miss Connection, Retry in %d secs",co.frectime);
			sleep (co.frectime);
		}
	}while(serv_fd == -1);
	
	errno = 0;
	turn_on_led (led);
	
	set_fl_d (serv_fd,O_NONBLOCK,false);
	sockNagle_d (serv_fd);
	
	return 0;
}

_Noreturn void * check_monit(void * arg)
{
	errno = 0;
	int temp_counts = 0;
	int while_counts = 0;
	int notice_counts = 5;
	int while_counts_copy = -1;
	int sleep_time = co.interval / 2;
	bool can_be_record = true;
	while (true)
	{
		pthread_mutex_lock (&mutex_monit);
		clearMonit (&mn);
		mn.cpu_temper = read_cpu_temp();
		mn.distance = disMeasure (DISTANCE_T,DISTANCE_E);
		readSensorData (TEMP_HUMI,&mn.env_humidity,&mn.env_temper);
		pthread_mutex_unlock (&mutex_monit);
		
		/**
		 * The distence can not be zero
		 * Identify as invalid data and skip this loop */
		if(mn.distance == 0)
			continue;
		
		
		/**
		 * When the measured distance is less than 30
		 * it is deemed that there is an object approaching */
		if(mn.distance <= 30)
			turn_on_led (LED_GRE);
		else turn_off_led (LED_GRE);
		
		/**
 		 * Environmental exception check rules:
		 * It is recognized as abnormal
		 * if it is detected 5 times in 10 loops
		 * This is to prevent occasional mistakes */
		if(mn.cpu_temper >= MAX_CPU_TEMPER || mn.env_humidity >= MAX_ENV_HUMIDI)
		{
			temp_counts ++;
			if (can_be_record)
			{
				while_counts_copy = while_counts;
				can_be_record = false;
			}
			if(temp_counts >= notice_counts)
			{
				temp_counts = 0;
				turn_on_led (LED_YEL);
				perr_d (true,LOG_WARNING,"The temperature or humidity is too high");
			}
		}
		else turn_off_led (LED_YEL);

		while_counts ++;
		while_counts %= 9;
		if(temp_counts != 0)
		{
			if(while_counts_copy <= while_counts)
			{
				if(while_counts - while_counts_copy >= notice_counts)
				{
					temp_counts = 0;
					can_be_record = true;
				}
			}
			else if (while_counts_copy > while_counts)
			{
				if(while_counts+10 - while_counts >= notice_counts)
				{
					temp_counts = 0;
					can_be_record = true;
				}
			}
		}
		
		sleep (sleep_time);
	}
}

_Noreturn void * sendData(int led)
{
	char status[4];
	ssize_t read_len,write_len;
	int sleep_time;
	while (true)
	{
		sleep_time = co.interval;
		memset (status,0, 4);
		
		// The distance cannot be 0, which means no valid data is read by default
		if(mn.distance == 0)
		{
			sleep (sleep_time);
			continue;
		}
		
		pthread_mutex_lock (&mutex_monit);
		if(mode_strict)
			write_len = SSL_write (ssl_serv_fd,&mn,sizeof(mn));
		else write_len = write (serv_fd,&mn,sizeof(mn));
		pthread_mutex_unlock (&mutex_monit);
		
		printf ("write done. len = %zd\n",write_len);
		
		alarm (10);  // set a timer
		if(mode_strict)
			read_len = SSL_read (ssl_serv_fd,status,4);
		else read_len = read (serv_fd, status,4);
		alarm (0);  // unset timer
		flash_led (led,90);
		
		if(strcmp (status,"FIN") == 0)
		{
			printf ("FIN received, len = %zd, exiting in %d secs\n",read_len,sleep_time);
			sleep (sleep_time);
			my_exit();
		}
//		if(read_len == 0)
//		{
//			sleep_time = co.interval * 2;
//			perr_d(true,LOG_NOTICE,"Server closed, wait for %d secs",sleep_time);
//			// reset();
//		}
		if(read_len == -1)
		{
			sleep_time = co.interval * 5;
			perr_d (true,LOG_ERR,"NetWork Error! Service Will reset in %d secs",sleep_time);
			sleep (sleep_time);
			reset();
		}
		
		sleep (sleep_time);
	}
}