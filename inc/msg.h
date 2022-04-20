
#ifndef __MSG_H_
#define __MSG_H_

#include "head.h"

/** print a stderr message */
extern void errors(const char * message);

/** print a stderr message */
extern void warnings(const char * message);

/** print a stdout message */
extern void success(const char * message,...);

/** If the conditions are met, the message is written to stderr
 * and if 'isEmerge' is true program will stop */
extern void perr(bool condition,bool isEmerge,const char * msg);
//isEmerge == 1, error message
//isEmerge == 0, warning message

#endif