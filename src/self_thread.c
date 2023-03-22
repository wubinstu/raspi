//
// Created by Einc on 2023/03/21.
//

#include "log.h"
#include "global.h"
#include "self_thread.h"

bool is_thread_alive (pthread_t tid)
{
    int ret = pthread_kill (tid, 0); // 发送 0 信号
    if (ret == 0)
        return true;
    else if (ret == ESRCH)
        return false;
    else
    {
        perr (true, LOG_WARNING,
              "function is_thread_alive return false in other reason");
        return false;
    }
}

pthread_attr_t default_thread_attr ()
{
    pthread_attr_t attr;
    pthread_attr_init (& attr);
    int errno_save = errno;

    // 设置线程分离, 线程自生自灭, 不期望获得返回值
    if ((errno = pthread_attr_setdetachstate (& attr, PTHREAD_CREATE_DETACHED)) != 0)
        perr (true, LOG_WARNING,
              "function default_thread_attr can not set detach state");

    // 设置线程间竞争资源(CPU等)的级别
    if ((errno = pthread_attr_setscope (& attr, PTHREAD_PROCESS_PRIVATE)) != 0)
        perr (true, LOG_WARNING,
              "function default_thread_attr can not set scope");

    // 设定新线程的调度策略
    if ((errno = pthread_attr_setschedpolicy (& attr, SCHED_RR)) != 0)
        perr (true, LOG_WARNING,
              "function default_thread_attr can not set schedule policy");

    // 设置线程栈警戒区
    if ((errno = pthread_attr_setguardsize (& attr, PAGE_4K)) != 0)
        perr (true, LOG_WARNING,
              "function default_thread_attr can not set guard size");

    errno = errno_save;
    return attr;
}

pthread_t create_default_thread (void * (* func) (void *), void * args)
{
    int errno_save = errno;
    pthread_t thread;
    pthread_attr_t attr = default_thread_attr ();
    if ((errno = pthread_create (& thread, & attr, func, args)) != 0)
    {
        perr (true, LOG_WARNING,
              "function create_default_thread failed");
        return -1;
    }

    errno = errno_save;
    return thread;
}

pthread_mutexattr_t default_mutex_attr ()
{
    int errno_save = errno;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init (& attr);

    // 设置互斥量可见范围
    if ((errno = pthread_mutexattr_setpshared (& attr, PTHREAD_PROCESS_PRIVATE)) != 0)
        perr (true, LOG_WARNING,
              "function default_mutex_attr can not set PROCESS_PRIVATE");

    // 设置互斥锁误检查(同线程重复加锁,解锁未锁定的锁,解锁其他线程锁定的锁都会出错)
    if ((errno = pthread_mutexattr_settype (& attr, PTHREAD_MUTEX_ERRORCHECK)) != 0)
        perr (true, LOG_WARNING,
              "function default_mutex_attr can not set type to ERRORCHECK");

    // 线程的优先级和调度不会受到互斥量拥有权的影响
    if ((errno = pthread_mutexattr_setprotocol (& attr, PTHREAD_PRIO_NONE)) != 0)
        perr (true, LOG_WARNING,
              "function default_mutex_attr can not set protocol to NONE");

    if ((errno = pthread_mutexattr_setrobust (& attr, PTHREAD_MUTEX_ROBUST)) != 0)
        perr (true, LOG_WARNING,
              "function default_mutex_attr can not set robust");

    errno = errno_save;
    return attr;
}

pthread_mutex_t * create_default_mutex (pthread_mutex_t * mutex)
{
    int errno_save = errno;
    pthread_mutexattr_t attr = default_mutex_attr ();
    if ((errno = pthread_mutex_init (mutex, & attr)) != 0)
    {
        perr (true, LOG_WARNING,
              "function create_default_mutex failed");
        errno = errno_save;
        return NULL;
    }
    errno = errno_save;
    return mutex;
}

bool lock_robust_mutex (pthread_mutex_t * mutex)
{
    int rtn = pthread_mutex_lock (mutex);
    if (rtn == 0)
        return true;
    else if (rtn == EDEADLK)
    {
        perr (true, LOG_WARNING,
              "function lock_robust_mutex failed, already get lock");
        return false;
    } else if (rtn == EOWNERDEAD)
    {
        rtn = pthread_mutex_consistent (mutex);
        if (rtn == 0)
        {
            if (pthread_mutex_unlock (mutex) != 0)
            {
                perr (true, LOG_WARNING,
                      "function lock_robust_mutex failed, can NOT unlock mutex after consistent");
                return false;
            }
            if (pthread_mutex_lock (mutex) != 0)
            {
                perr (true, LOG_WARNING,
                      "function lock_robust_mutex failed, can NOT lock mutex after consistent");
                return false;
            }
            return true;
        }
        if (rtn == EINVAL)
        {
            perr (true, LOG_WARNING,
                  "function lock_robust_mutex failed, mutex is NOT robust");
            return false;
        }

        pthread_mutexattr_t attr = default_mutex_attr ();
        pthread_mutex_destroy (mutex);
        pthread_mutex_init (mutex, & attr);
        pthread_mutex_lock (mutex);
        return true;
    } else
    {
        perr (true, LOG_WARNING,
              "function lock_robust_mutex failed in other reason");
        return false;
    }
}