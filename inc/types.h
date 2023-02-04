
#ifndef __TYPES_H_
#define __TYPES_H_

#include "stdbool.h"

enum ServClnt
{
    server = 0, client = 1
};

enum ClntSSL
{
    client_empty = 0, client_only_ca = 1, client_with_cert_key = 2
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
    KeyValuePair *opt;
    struct LinkNode *next;
} Node, *LNode;

/** For data detection */
typedef struct monitoring
{
    float distance;
    float cpu_temper;
    float env_temper;
    float env_humidity;
} RaspiMonitData;

typedef struct configuration_options
{
    unsigned long serv_ip;
    unsigned short serv_port;
    int interval;
    int frectime;
    int frecatps;
    enum ClntSSL mode_ssl_client;
    char CAfile[256];
    char UCert[256];
    char UKey[256];
} confOptClnt;

/** Fill data */
extern void setElem (KeyValuePair *e, const char *name, const char *value);

/** Free elem */
extern void delElem (KeyValuePair *e);

/** Print elem */
extern void catElem (KeyValuePair e);

/** Print elems */
extern void catElems (KeyValuePair e[], int length);

/** set struct value to zero */
extern void clearMonit (RaspiMonitData *mn);

/**
 * set defalut value to configuration */
extern void default_confOpt (confOptClnt *co);

#endif