
#ifndef __LOG_H_
#define __LOG_H_

#include "stdbool.h"

#define     EMERE              "[  EMERE  ] ";
#define     ALERT              "[  ALERT  ] ";
#define     CRIT               "[  CRITI  ] ";
#define     ERR                "[  ERROR  ] ";
#define     WARNING            "[ WARNING ] ";
#define     NOTICE             "[  NOTIC  ] ";
#define     INFO               "[  INFOS  ] ";
#define     DEBUG              "[  DEBUG  ] ";
#define     UNKNOWN            "[ UNKNOWN ] ";
#define     PREFIX_LEN          12
#define     CACHE_SIZE          50

/** Returns the signal name according to the signal value */
extern char *log_prefix (int logLevel);

/** If the conditions are met, the message is written to the system log,and the device is daemon */
extern void perr_d (bool condition, int logLevel, const char *message, ...);


#endif//__LOG_H_