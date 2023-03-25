//
// Created by Einc on 2023/03/21.
//

#ifndef __RUN_SERVER_H_
#define __RUN_SERVER_H_

#include "head.h"
#include "types.h"

extern bool initServerSocket ();

extern void loadSSLServ ();

extern void eventPoll (server_info_t serverInfo);

extern void * processRaspiClient (void * args);

extern void * processHttpClient (void * args);

#endif //__RUN_SERVER_H_
