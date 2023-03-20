
#ifndef __TYPES_H_
#define __TYPES_H_

#include "stdbool.h"

enum ServClnt
{
    server = 0, client = 1
};

enum ClntSSL
{
    client_ssl_disable = -1,
    client_ssl_empty = 0,
    client_ssl_only_ca = 1
};

enum ServSSL
{
    server_ssl_disable = -1,
    server_ssl_enable = 1
};

/** For configuration file */
typedef struct element
{
    char name[10];
    char value[90];
} KeyValuePair;

/**
 * Define the structure of linked list nodes
 * The use of pointer type data fields will
 * make the linked list more widely applicable */
typedef struct LinkNode
{
    KeyValuePair * opt;
    struct LinkNode * next;
} Node, * LNode;

/** For data detection */
typedef struct monitoring
{
    float distance;
    float cpu_temper;
    float env_temper;
    float env_humidity;
} RaspiMonitData;

typedef struct configuration_options_client
{
    unsigned long servIp;
    unsigned short servPort;
    int interval;

    char caFile[256];
    char pidFile[256];
    bool modeDaemon;
} ConfOptClnt;

typedef struct configuration_options_server
{
    unsigned long bindIp;
    unsigned short bindPort;


    bool modeDaemon;
    bool modeSSL;
    char caFile[256];
    char servCert[256];
    char servKey[256];
    char pidFile[256];
} ConfOptServ;

/** Fill data */
extern void setElem (KeyValuePair * e, const char * name, const char * value);

/** Free elem */
extern void delElem (KeyValuePair * e);

/** Print elem */
extern void catElem (KeyValuePair e);

/** Print elems */
extern void catElems (KeyValuePair e[], int length);

/** set struct value to zero */
extern void clearMonit (RaspiMonitData * mn);

/**
 * set default value to configuration */
extern void defaultConfOptClnt (ConfOptClnt * co);

extern void defaultConfOptServ (ConfOptServ * co);

#endif