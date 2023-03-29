//
// Created by Einc on 2022/4/1.
//

#ifndef __LOAD_LEAN_H_
#define __LOAD_LEAN_H_

#include "head.h"

/** daemon
 * @param cmd program(process) name */
extern void daemonize (const char * cmd);


/** Preprocess runtime arguments */
extern void preRunTimeArgsServ (int argc, const char * argv[]);

extern void preRunTimeArgsClnt (int argc, const char * argv[]);

/** Processing Runtime Parameters */
extern void runTimeArgsServ (int argc, const char * argv[]);

extern void runTimeArgsClnt (int argc, const char * argv[]);

/** Ensure that only one program runs at the same time according to the PID file */
extern void checkPidFileServ (char * pid_file);

extern void checkPidFileClnt (char * pid_file);

/** Read configuration file to global variable */
extern void confToVarServ ();

extern void confToVarClnt ();

/** Cleanup function when program exits */
extern void exitCleanupServ ();

extern void exitCleanupClnt ();

/**
 * When an error occurs in the program
 * it will return to the starting archive point */
extern void resetServ ();

extern void resetClnt ();


/**
 * Returns the bool value about you have root permission and record syslog
 * @return return true if you get root permission
 * @param require_reason log the massage when permission denied */
extern bool checkRootPermission (const char * require_reason);

/** send FIN to server */
extern void sendFINtoServ ();

/** turn down all lights and close server fd */
extern void downLightsCloseServ ();

#endif //__LOAD_LEAN_H_
