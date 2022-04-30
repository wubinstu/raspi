//
// Created by Einc on 2022/4/1.
//

#ifndef __MYDAEMON_H
#define __MYDAEMON_H

#include "head.h"

/** daemon */
extern void daemonize (const char *cmd);

/** Cleanup function when program exits */
extern void my_exit();

/**
 * When an error occurs in the program
 * it will return to the starting archive point
 */
extern void reset();

/** Ensure that only one program runs at the same time according to the PID file */
extern void check_running();

/**
 * Returns the bool value about you have root permission and record syslog */
extern bool check_permission(const char * msg);

/** Register signal processing function */
extern void sig_reg();

/**
 * Processing Runtime Parameters */
extern void dealWithArgs(int argc,const char * argv[]);

#endif //__MYDAEMON_H
