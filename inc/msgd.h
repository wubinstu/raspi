
#ifndef __MSGD_H_
#define __MSGD_H_

#include "head.h"


// static const char *     EMERE       =       "[  EMERE  ] ";
// static const char *     ALERT       =       "[  ALERT  ] ";
// static const char *     CRIT        =       "[  CRITI  ] ";
// static const char *     ERR         =       "[  ERROR  ] ";
// static const char *     WARNING     =       "[ WARNING ] ";
// static const char *     NOTICE      =       "[  NOTIC  ] ";
// static const char *     INFO        =       "[  INFOS  ] ";
// static const char *     DEBUG       =       "[  DEBUG  ] ";
// static const char *     UNKNOWN     =       "[ UNKNOWN ] ";

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
extern char * log_prefix(int logLevel);

/** If the conditions are met, the message is written to the system log,and the device is daemon */
extern void perr_d(bool condition,int logLevel,const char * message,...);


#endif