//
// Created by Einc on 2023/03/21.
//

#ifndef __SELF_THREAD_H_
#define __SELF_THREAD_H_

#include "types.h"

/**
 * to check whether the thread is still alive */
extern bool is_thread_alive (pthread_t tid);

/**
 * create a default thread attribute
 * 1. detach thread
 * 2. process private
 * 3. schedule policy: RR
 * 4. stack guard size 4K */
extern pthread_attr_t default_thread_attr ();

/**
 * create a thread with default attribute */
extern pthread_t create_default_thread (void * (* func) (void *), void * args);

/**
 * create a default mutex attribute
 * 1. process private
 * 2. ERROR CHECK
 * 3. PRIORITY NONE
 * 4. ROBUST */
extern pthread_mutexattr_t default_mutex_attr ();

/**
 * create a mutex with default attribute */
extern pthread_mutex_t * create_default_mutex (pthread_mutex_t * mutex);

/**
 * try to lock a mutex with robust */
extern bool lock_robust_mutex (pthread_mutex_t * mutex);

#endif //__SELF_THREAD_H_
