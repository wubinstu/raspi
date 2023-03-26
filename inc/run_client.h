//
// Created by Einc on 2022/4/4.
//

#ifndef __RUN_CLIENT_H_
#define __RUN_CLIENT_H_

#include "head.h"
#include "uuid/uuid.h"


extern int readServer (void * buf, int size);

extern int writeServer (void * buf, int size);

/** Trying to connect to the server
 * exponential backoff:[1,MAXSLEEP(128)]secs
 * set server socket to no blocking and enable nagle */
extern int tryConnect (int led);

extern void loadSSLClnt ();

extern bool fileUUID (uuid_t uu, bool true_for_load_false_for_record);

extern void negotiateUUID ();

/** [NO RETURN] Continuous monitoring and data acquisition */
extern void checkMonit ();

/** [NO RETURN] Continuously send data to the server */
extern void sendData (int led);

#endif //__RUN_CLIENT_H_
