
#include "head.h"
#include "types.h"

void setElem (KeyValuePair * e, const char * name, const char * value)
{
    strcpy (e->name, name);
    strcpy (e->value, value);
}

void delElem (KeyValuePair * e)
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


void clearMonit (RaspiMonitData * mn)
{
    if (mn == NULL)
        return;
    mn->distance = 0;
    mn->cpu_temper = 0;
    mn->env_temper = 0;
    mn->env_humidity = 0;
}

void defaultConfOptClnt (ConfOptClnt * co)
{
    if (co == NULL)
        return;
    co->servIp = inet_addr ("127.0.0.1");
    co->servPort = 9190;
    co->interval = 10;
    co->modeDaemon = true;

    strcpy (co->caFile, "default");
    strcpy (co->pidFile, "default");
}

void defaultConfOptServ (ConfOptServ * co)
{
    if (co == NULL)
        return;
    co->bindIp = INADDR_ANY;
    co->bindPort = 9190;
    co->httpPort = 8080;
    co->sqlPort = 3306;
    co->modeSSL = true;
    co->modeDaemon = true;

    memset (co->caFile, 0, sizeof (co->caFile));
    memset (co->servCert, 0, sizeof (co->servCert));
    memset (co->servKey, 0, sizeof (co->servKey));
    memset (co->pidFile, 0, sizeof (co->pidFile));
    memset (co->sqlHost, 0, sizeof (co->sqlHost));
    memset (co->sqlUser, 0, sizeof (co->sqlUser));
    memset (co->sqlPass, 0, sizeof (co->sqlPass));
    memset (co->sqlName, 0, sizeof (co->sqlName));
}