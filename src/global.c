//
// Created by Einc on 2023/1/24.
//

#include "global.h"
#include "head.h"


int filed_logLevel = LOG_WARNING;
int rssl_logLevel = LOG_ERR;
int socket_fd_logLevel = LOG_WARNING;

ConfOptClnt config_client;
ConfOptServ config_server;
RaspiMonitData raspi_monit_data;

server_info_t raspi_connect_server;
server_info_t server_accept_raspi;
server_info_t server_accept_http;

sql_pool_t * sql_pool_accept_raspi = NULL;
thread_pool_t * thread_pool_accept_raspi = NULL;
thread_pool_t * thread_pool_accept_http = NULL;

hash_map_t * hash_map_raspi = NULL;

int pid_file_fd = -1;

jmp_buf jmp_client_rest;
jmp_buf jmp_server_rest;