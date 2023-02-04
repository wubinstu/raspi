//
// Created by Einc on 2023/1/24.
//

#include "global.h"
#include "head.h"


int DEBUGMODE = 1;

enum ServClnt mode_serv_clnt = client;
enum ClntSSL mode_ssl_client = client_empty;

int filed_logLevel = LOG_WARNING;
int rssl_logLevel = LOG_ERR;
int socket_fd_logLevel = LOG_WARNING;

confOptClnt file_client_config;
RaspiMonitData raspi_monit_data;
pthread_mutex_t mutex_monit_data = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_client_data_checker = 0;
pthread_t thread_client_data_sender = 0;

int pid_file_fd = -1;
int serv_fd = -1;
SSL *ssl_serv_fd = NULL;
SSL_CTX *ctx = NULL;

bool mode_strict = false;
bool mode_daemon = false;

jmp_buf jmp_client_rest;


void inline set_serv_clnt (enum ServClnt m)
{
    if (m == server || m == client)
        mode_serv_clnt = m;
}

void inline set_ssl_client (enum ClntSSL m)
{
    if (m == client_empty || m == client_only_ca || m == client_with_cert_key)
        mode_ssl_client = m;
}