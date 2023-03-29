//
// Created by Einc on 2023/03/21.
//

#ifndef __SQL_POOL_H_
#define __SQL_POOL_H_

#include "types.h"

extern sql_pool_t * sql_pool_init (const char * host, unsigned short port,
                                   const char * user, const char * pass, const char * db,
                                   unsigned int conn_max, unsigned int conn_min);


extern void sql_pool_destroy (sql_pool_t * pool);

extern sql_node_t * sql_pool_conn_fetch (sql_pool_t * pool);

extern bool sql_pool_conn_release (sql_pool_t * pool, sql_node_t ** sql_node);

extern bool sql_pool_conn_add (sql_pool_t * pool, unsigned int add_num);

extern bool sql_pool_conn_del (sql_pool_t * pool, unsigned int del_num);

extern void * sql_pool_manage (void * args);

#endif //__SQL_POOL_H_
