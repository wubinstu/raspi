
#include "types.h"

const int ElemSize = sizeof(Elem);

void setElem(Elem * e,const char * name,const char * value)
{
	strcpy(e -> name,name);
	strcpy(e -> value,value);
}

void delElem(Elem * e)
{
	if(e != NULL)
		free(e);
}

void catElem(Elem e)
{
	printf("name = %s, value = %s\n",e.name,e.value);
}

void catElems(Elem e[],int length)
{
	for(int i = 0; i < length;i++)
		printf("name = %s, value = %s\n",e[i].name,e[i].value);
}


void clearMonit(monit * mn)
{
	if(mn == NULL)
		return;
	mn->distance = 0;
	mn->cpu_temper = 0;
	mn->env_temper = 0;
	mn->env_humidity = 0;
}

void default_confOpt(confOpt * co)
{
	if (co == NULL)
		return;
	co->serv_ip = inet_addr ("127.0.0.1");
	co->serv_port = 9190;
	co->interval = 10;
	co->frectime = 42;
	co->frecatps = 7;
	co->checkMe = false;
	bzero (co->CAfile,sizeof (co->CAfile));
	bzero (co->UCert,sizeof (co->UCert));
	bzero (co->UKey,sizeof (co->UKey));
}