//
// Created by Einc on 2023/03/21.
//

#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include "types.h"


extern thread_pool_t *
thread_pool_init (unsigned int thread_max_num, unsigned int thread_min_num, unsigned int queue_max_num);

extern void thread_pool_destroy (thread_pool_t * pool);

extern void * thread_pool_process (void * args);

extern void * thread_pool_manage (void * args);

extern bool thread_pool_add_task (thread_pool_t * pool, thread_task_t * task);

#endif //__THREAD_POOL_H_
