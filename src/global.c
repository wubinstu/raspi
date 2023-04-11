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
ssl_pool_t * ssl_pool_accept_raspi = NULL;
ssl_pool_t * ssl_pool_accept_http = NULL;

hash_table_client_t * hash_table_raspi = NULL;
hash_table_client_t * hash_table_http = NULL;
hash_table_info_t * hash_table_info_raspi_http = NULL;

pthread_t thread_id_server_accept_raspi = 0;
pthread_t thread_id_server_accept_http = 0;

struct epoll_event event_server_raspi[SERVER_EPOLL_SIZE];
struct epoll_event event_server_http[SERVER_EPOLL_SIZE];

int pid_file_fd = -1;
int web_html_fd = -1;
long web_html_size = -1;
char * web_html_buf = NULL;

jmp_buf jmp_client_rest;
jmp_buf jmp_server_rest;