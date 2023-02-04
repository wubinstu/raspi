
#include "head.h"
#include "types.h"

void setElem (KeyValuePair *e, const char *name, const char *value)
{
    strcpy (e->name, name);
    strcpy (e->value, value);
}

void delElem (KeyValuePair *e)
{
    if (e != NULL)
        free (e);
}

void catElem (KeyValuePair e)
{
    printf ("name = %s, value = %s\n", e.name, e.value);
}

void catElems (KeyValuePair e[], int length)
{
    for (int i = 0; i < length; i++)
        printf ("name = %s, value = %s\n", e[i].name, e[i].value);
}


void clearMonit (RaspiMonitData *mn)
{
    if (mn == NULL)
        return;
    mn->distance = 0;
    mn->cpu_temper = 0;
    mn->env_temper = 0;
    mn->env_humidity = 0;
}

void default_confOpt (confOptClnt *co)
{
    if (co == NULL)
        return;
    co->serv_ip = inet_addr ("127.0.0.1");
    co->serv_port = 9190;
    co->interval = 10;
    co->frectime = 42;
    co->frecatps = 7;
    co->mode_ssl_client = 0;
    bzero (co->CAfile, sizeof (co->CAfile));
    bzero (co->UCert, sizeof (co->UCert));
    bzero (co->UKey, sizeof (co->UKey));
}