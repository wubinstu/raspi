//
// Created by Einc on 2022/4/1.
//

#ifndef __LOAD_LEAN_H_
#define __LOAD_LEAN_H_

#include "head.h"

/** daemon
 * @param cmd program(process) name */
extern void daemonize (const char *cmd);

/**
 * Processing Runtime Parameters */
extern void dealWithArgsServ (int argc, const char *argv[]);

extern void dealWithArgsClnt (int argc, const char *argv[]);

/**
 * Returns the bool value about you have root permission and record syslog
 * @return return true if you get root permission
 * @param require_reason log the massage when permission denied */
extern bool check_permission (const char *require_reason);

/** Ensure that only one program runs at the same time according to the PID file */
extern void checkPidFile ();

/** Read configuration file to global variable */
extern void conf2var ();

/** Cleanup function when program exits */
extern void exitCleanupClnt ();

/**
 * When an error occurs in the program
 * it will return to the starting archive point */
extern void resetClnt ();

#endif //__LOAD_LEAN_H_
