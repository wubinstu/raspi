//
// Created by Einc on 2023/04/07.
//

#ifndef __SSL_POOL_
#define __SSL_POOL_

#include "global.h"

extern ssl_pool_t * ssl_pool_init (unsigned int ssl_max, unsigned int ssl_min, SSL_CTX * ctx);

extern void ssl_pool_destroy (ssl_pool_t * pool);

extern ssl_node_t * ssl_pool_node_fetch (ssl_pool_t * pool);

extern bool ssl_pool_node_release (ssl_pool_t * pool, ssl_node_t ** ssl_node);

extern bool ssl_pool_node_add (ssl_pool_t * pool, unsigned int add_num);

extern bool ssl_pool_node_del (ssl_pool_t * pool, unsigned int del_num);

extern void * ssl_pool_manage (void * args);

#endif //__SSL_POOL_
