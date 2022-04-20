//
// Created by Einc on 2022/4/4.
//

#ifndef __MYTHREAD_H_
#define __MYTHREAD_H_

#include "head.h"

/** Read configuration file to global variable */
extern void conf2var();

/** Trying to connect to the server */
extern int tryconnect(int led);

/** [NO RETURN] Continuous monitoring and data acquisition */
_Noreturn extern void * check_monit(void * arg);

/** [NO RETURN] Continuously send data to the server */
extern _Noreturn void * sendData(int led);

#endif //__MYTHREAD_H_
