//
// Created by Einc on 2022/4/4.
//

#ifndef __CLIENT_RUN_H_
#define __CLIENT_RUN_H_

#include "head.h"

/** Trying to connect to the server */
extern int tryConnect (int led);

/** [NO RETURN] Continuous monitoring and data acquisition */
_Noreturn extern void *checkMonit (void *arg);

/** [NO RETURN] Continuously send data to the server */
extern _Noreturn void *sendData (void *led);

#endif //__CLIENT_RUN_H_
