//
// Created by Einc on 2023/03/21.
//

#ifndef __RUN_SERVER_H_
#define __RUN_SERVER_H_

#include "head.h"
#include "types.h"

extern int readClient (client_info_t * clientInfo, void * buf, int size);

extern int writeClient (client_info_t * clientInfo, void * buf, int size);

extern bool initServerSocket (server_info_t * serverInfo, unsigned short port);

extern void loadSSLServ (server_info_t * serverInfo);

extern void negotiateUUID (client_info_t * clientInfo);

extern void * raspiEventPoll (void * args);

extern void * processRaspiClient (void * args);

extern void * httpEventPoll (void * args);

extern void * processHttpClient (void * args);

#endif //__RUN_SERVER_H_
