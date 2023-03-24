//
// Created by Einc on 2023/01/29.
//

#ifndef __RSIGNAL_H_
#define __RSIGNAL_H_


/** TODO
 * Register signal processing function
 * SIGINT -> exitCleanupClnt()
 * SIGTERM -> myexit()
 * SIGQUIT -> myexit()
 * SIGHUP -> resetClnt()
 * SIGALARM -> resetClnt() */
extern void sigRegisterClnt ();

/// TODO
extern void sigRegisterServ ();

#endif //__RSIGNAL_H_