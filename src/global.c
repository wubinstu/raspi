//
// Created by Einc on 2023/1/24.
//

#include "global.h"
#include "head.h"


enum ServClnt mode_serv_clnt = client;
enum ClntSSL mode_ssl_client = client_ssl_empty;
enum ServSSL mode_ssl_server = server_ssl_enable;

int filed_logLevel = LOG_WARNING;
int rssl_logLevel = LOG_ERR;
int socket_fd_logLevel = LOG_WARNING;

ConfOptClnt config_client;
ConfOptServ config_server;
RaspiMonitData raspi_monit_data;

int pid_file_fd = -1;
int serv_fd = -1;
SSL * ssl_serv_fd = NULL;
SSL_CTX * ctx_client_to_server = NULL;

bool mode_strict = false;

jmp_buf jmp_client_rest;
jmp_buf jmp_server_rest;


void inline setServClnt (enum ServClnt m)
{
    if (m == server || m == client)
        mode_serv_clnt = m;
}

void inline set_ssl_client (enum ClntSSL m)
{
    if (m == client_ssl_disable || m == client_ssl_empty || m == client_ssl_only_ca)
        mode_ssl_client = m;
}