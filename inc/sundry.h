
#ifndef __SUNDRY_H_
#define __SUNDRY_H_

#include "head.h"

/** draw a process bar (m/100) */
extern void procbar(int m);

/** draw a process bar (m/100) */
extern void procbar_f(float m,bool state);

/** check whether debug mode */
extern bool isDebugger();

#endif