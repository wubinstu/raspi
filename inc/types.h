
#ifndef __TYPES_H_
#define __TYPES_H_

#include "head.h"

/** For configuration file */
typedef struct element
{
	char name[10];
	char value[90];
}Elem;

/** For data detection */
typedef struct monitoring
{
	float distance;
	float cpu_temper;
	float env_temper;
	float env_humidity;
}monit;

typedef struct configuration_options
{
	unsigned long serv_ip;
	unsigned short serv_port;
	int interval;
	int frectime;
	int frecatps;
	bool checkMe;
	char CAfile[256];
	char UCert[256];
	char UKey[256];
}confOpt;

/** Fill data */
extern void setElem(Elem * e,const char * name,const char * value);

/** Free elem */
extern void delElem(Elem * e);

/** Print elem */
extern void catElem(Elem e);

/** Print elems */
extern void catElems(Elem e[],int length);

/** set struct value to zero */
extern void clearMonit(monit * mn);

/**
 * set defalut value to configuration */
extern void default_confOpt(confOpt * co);

#endif