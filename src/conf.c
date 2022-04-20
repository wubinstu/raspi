
#include "conf.h"
#include "arr.h"
#include "mylink.h"
#include "filed.h"
#include "msgd.h"

static int conf_logLevel = LOG_WARNING;


void defaultconf(const char * file_path)
{
	setFiledLogLevel (conf_logLevel);
	int rw_fd = newopen_d (file_path);
	if(rw_fd == -1) return;
	char * default_msg[] =
	{
			"\n",
			"# This is the configuration file of project rain\n",
			"# Writing rules: [key = value]\n",
			"# 1.Case insensitive\n",
			"# 2.Ignore spaces,double quotes,tabs\n",
			"# 3.Notes are indicated by pound signs(#)\n",
			"# 4.Incorrect writing may cause the program to not work properly!\n",
			"# Here are some usage examples\n",
			"\n\n\n",
			"# Specify the address of the server\n",
			"# Both domain name and IP address are allowed\n",
			"#ServAddr = 127.0.0.1\n",
			"\n",
			"# Specify the port of the server\n",
			"#ServPort = 1234\n",
			"\n",
			"# Specifies the absolute path of the CA certificate\n",
			"# Used to authenticate the server\n",
			"#CAfile = /home/pi/raspi/keys/ca.crt\n",
			"\n",
			"# set \"true\" when the server wants to verify the client certificate only\n",
			"# If true, \"UCert\" and \"UKey\" must be specified\n",
			"#CheckMe = false\n",
			"\n",
			"# Specifies the absolute path of the user certificate\n",
			"# specified when \"CheckMe = true\"\n",
			"#UCert = /home/pi/raspi/keys/client.crt\n",
			"\n",
			"# Specifies the absolute path of the user private key\n",
			"# specified when \"CheckMe = true\"\n",
			"#UKey = /home/pi/raspi/keys/client.key\n",
			"\n",
			"# Data sampling interval(Seconds)\n",
			"#Interval = 5\n",
			"\n",
			"# Failed reconnection time(Seconds)\n",
			"# Note: this only indicates the time from the failure of the connect function to the next connect request.\n",
			"# Don't forget that the connect itself will block for a period of time(NO BLOCKING)\n",
			"#FRecTime = 42\n",
			"\n",
			"# Failed reconnection attempts\n",
			"#FRecAtps = 20\n",
			"\n"
	};
	for(int i = 0;i < 43;i++)
		write (rw_fd, default_msg[i], strlen(default_msg[i]));

	close (rw_fd);
}

LNode readconf (const char *file_path)
{
	setFiledLogLevel (conf_logLevel);
	FILE * conf_stream = fopen (file_path,"r");

	if (conf_stream == NULL)
	{
		perr_d (true, conf_logLevel, "function fdopen returns NULL when you called readconf");
		return NULL;
	}
	
	LNode L;
	Elem e;
	char line[BUF_SIZE] = {'\0'};
	char aline[BUF_SIZE] = {'\0'};
	char bline[BUF_SIZE] = {'\0'};
	InitLinkList (&L);
	
	while (fgets (line, BUF_SIZE, conf_stream) != NULL)
	{
		rmCharacter (line, '\t');// rm tab
		rmCharacter (line, ' ');// rm space
		if (isEmptyL (line) || isNotes (line))
			continue;
		
		rmNextL (line);
		rmCharacter (line, '"');// rm quotation marks
		if(isContainC (line,'=') && isContainC (line,'#'))
		{
			subString (line,'#',aline,bline);
			subString (aline,'=',e.name,e.value);
		}
		if (isContainC (line, '=') && !isContainC (line,'#'))
			subString (line, '=', e.name, e.value);
		
		upperConversion (e.name);
			
		AddToLinkList (&L, e);
		memset (line, '\0', BUF_SIZE);
	}
	fclose (conf_stream);
	return L;
}

bool checkconf(const char * file_path)
{
	bool flag = true;
	
	int fd = readopen_d (file_path);
	if(fd == -1)
	{
		perr_d (true,conf_logLevel,
				"Can not find configuration file, "
				"The configuration file will be created at %s automatically, "
				"For security reasons, the service has stopped, "
				"Please write your settings and restart the service",CONF_FILE);
		defaultconf (file_path);
		flag = false;
	}
	close (fd);
	return flag;
}

bool checkread(LNode L)
{
	bool flag =  (L -> next != NULL);
	if(flag)
	{
		LNode master = L -> next;
		while (master != NULL)
		{
			if(notASCII (master -> opt -> name) || notASCII (master -> opt -> value))
			{
				flag = false;
				perr_d (true,conf_logLevel,"unknown word in configuration line %s = %s",master -> opt -> name,master -> opt -> value);
			}
			master = master -> next;
		}
	}
	return flag;
}