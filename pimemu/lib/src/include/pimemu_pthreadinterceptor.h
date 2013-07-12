/*******************************************************************************
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 *
 * RESTRICTED RIGHTS NOTICE (DEC 2007)
 * (a)     This computer software is submitted with restricted rights under
 *     Government Contract No. DE-AC52-8MA27344 and subcontract B600716. It
 *     may not be used, reproduced, or disclosed by the Government except as
 *     provided in paragraph (b) of this notice or as otherwise expressly
 *     stated in the contract.
 *
 * (b)     This computer software may be -
 *     (1) Used or copied for use with the computer(s) for which it was
 *         acquired, including use at any Government installation to which
 *         the computer(s) may be transferred;
 *     (2) Used or copied for use with a backup computer if any computer for
 *         which it was acquired is inoperative;
 *     (3) Reproduced for safekeeping (archives) or backup purposes;
 *     (4) Modified, adapted, or combined with other computer software,
 *         provided that the modified, adapted, or combined portions of the
 *         derivative software incorporating any of the delivered, restricted
 *         computer software shall be subject to the same restricted rights;
 *     (5) Disclosed to and reproduced for use by support service contractors
 *         or their subcontractors in accordance with paragraphs (b)(1)
 *         through (4) of this notice; and
 *     (6) Used or copied for use with a replacement computer.
 *
 * (c)     Notwithstanding the foregoing, if this computer software is
 *         copyrighted computer software, it is licensed to the Government with
 *         the minimum rights set forth in paragraph (b) of this notice.
 *
 * (d)     Any other rights or limitations regarding the use, duplication, or
 *     disclosure of this computer software are to be expressly stated in, or
 *     incorporated in, the contract.
 *
 * (e)     This notice shall be marked on any reproduction of this computer
 *     software, in whole or in part.
 ******************************************************************************/

#ifndef __PIMEMU_PTHREADINTERCEPTOR_H
#define __PIMEMU_PTHREADINTERCEPTOR_H


#include <sys/types.h>
#ifndef WIN32
#include <dlfcn.h>
#else
#include "wpthread.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int  (*piminterceptor_pthread_attr_destroy)(pthread_attr_t *);
typedef int  (*piminterceptor_pthread_attr_getdetachstate)(const pthread_attr_t *, int *);
typedef int  (*piminterceptor_pthread_attr_getguardsize)(const pthread_attr_t *, size_t *);
typedef int  (*piminterceptor_pthread_attr_getinheritsched)(const pthread_attr_t *, int *);
typedef int  (*piminterceptor_pthread_attr_getschedparam)(const pthread_attr_t *,
          struct sched_param *);
typedef int  (*piminterceptor_pthread_attr_getschedpolicy)(const pthread_attr_t *, int *);
typedef int  (*piminterceptor_pthread_attr_getscope)(const pthread_attr_t *, int *);
typedef int  (*piminterceptor_pthread_attr_getstackaddr)(const pthread_attr_t *, void **);
typedef int  (*piminterceptor_pthread_attr_getstacksize)(const pthread_attr_t *, size_t *);
typedef int  (*piminterceptor_pthread_attr_init)(pthread_attr_t *);
typedef int  (*piminterceptor_pthread_attr_setdetachstate)(pthread_attr_t *, int);
typedef int  (*piminterceptor_pthread_attr_setguardsize)(pthread_attr_t *, size_t);
typedef int  (*piminterceptor_pthread_attr_setinheritsched)(pthread_attr_t *, int);
typedef int  (*piminterceptor_pthread_attr_setschedparam)(pthread_attr_t *,
          const struct sched_param *);
typedef int  (*piminterceptor_pthread_attr_setschedpolicy)(pthread_attr_t *, int);
typedef int  (*piminterceptor_pthread_attr_setscope)(pthread_attr_t *, int);
typedef int  (*piminterceptor_pthread_attr_setstackaddr)(pthread_attr_t *, void *);
typedef int  (*piminterceptor_pthread_attr_setstacksize)(pthread_attr_t *, size_t);
typedef int  (*piminterceptor_pthread_cancel)(pthread_t);

/* MACRO

typedef void  (*piminterceptor_pthread_cleanup_push)(void (*routine)(void *),
                                 void *arg);
typedef void  (*piminterceptor_pthread_cleanup_pop)(int);
*/

typedef int  (*piminterceptor_pthread_cond_broadcast)(pthread_cond_t *);
typedef int  (*piminterceptor_pthread_cond_destroy)(pthread_cond_t *);
typedef int  (*piminterceptor_pthread_cond_init)(pthread_cond_t *, const pthread_condattr_t *);
typedef int  (*piminterceptor_pthread_cond_signal)(pthread_cond_t *);
typedef int  (*piminterceptor_pthread_cond_timedwait)(pthread_cond_t *, 
          pthread_mutex_t *, const struct timespec *);
typedef int  (*piminterceptor_pthread_cond_wait)(pthread_cond_t *, pthread_mutex_t *);
typedef int  (*piminterceptor_pthread_condattr_destroy)(pthread_condattr_t *);
typedef int  (*piminterceptor_pthread_condattr_getpshared)(const pthread_condattr_t *, int *);
typedef int  (*piminterceptor_pthread_condattr_init)(pthread_condattr_t *);
typedef int  (*piminterceptor_pthread_condattr_setpshared)(pthread_condattr_t *, int);
typedef int  (*piminterceptor_pthread_create)(pthread_t *, const pthread_attr_t *,
          void *(*)(void *), void *);
typedef int  (*piminterceptor_pthread_detach)(pthread_t);
typedef int  (*piminterceptor_pthread_equal)(pthread_t, pthread_t);
typedef void  (*piminterceptor_pthread_exit)(void *);
typedef int  (*piminterceptor_pthread_getconcurrency)(void);
typedef int  (*piminterceptor_pthread_getschedparam)(pthread_t, int *, struct sched_param *);
#ifndef WIN32
typedef void * (*piminterceptor_pthread_getspecific)(pthread_key_t);
#endif
typedef int  (*piminterceptor_pthread_join)(pthread_t, void **);
#ifndef WIN32
typedef int  (*piminterceptor_pthread_key_create)(pthread_key_t *, void (*)(void *));
typedef int  (*piminterceptor_pthread_key_delete)(pthread_key_t);
#endif
typedef int  (*piminterceptor_pthread_mutex_destroy)(pthread_mutex_t *);
typedef int  (*piminterceptor_pthread_mutex_getprioceiling)(const pthread_mutex_t *, int *);
typedef int  (*piminterceptor_pthread_mutex_init)(pthread_mutex_t *, const pthread_mutexattr_t *);
typedef int  (*piminterceptor_pthread_mutex_lock)(pthread_mutex_t *);
typedef int  (*piminterceptor_pthread_mutex_setprioceiling)(pthread_mutex_t *, int, int *);
typedef int  (*piminterceptor_pthread_mutex_trylock)(pthread_mutex_t *);
typedef int  (*piminterceptor_pthread_mutex_unlock)(pthread_mutex_t *);
typedef int  (*piminterceptor_pthread_mutexattr_destroy)(pthread_mutexattr_t *);
typedef int  (*piminterceptor_pthread_mutexattr_getprioceiling)(const pthread_mutexattr_t *,
          int *);
typedef int  (*piminterceptor_pthread_mutexattr_getprotocol)(const pthread_mutexattr_t *, int *);
typedef int  (*piminterceptor_pthread_mutexattr_getpshared)(const pthread_mutexattr_t *, int *);
typedef int  (*piminterceptor_pthread_mutexattr_gettype)(const pthread_mutexattr_t *, int *);
typedef int  (*piminterceptor_pthread_mutexattr_init)(pthread_mutexattr_t *);
typedef int  (*piminterceptor_pthread_mutexattr_setprioceiling)(pthread_mutexattr_t *, int);
typedef int  (*piminterceptor_pthread_mutexattr_setprotocol)(pthread_mutexattr_t *, int);
typedef int  (*piminterceptor_pthread_mutexattr_setpshared)(pthread_mutexattr_t *, int);
typedef int  (*piminterceptor_pthread_mutexattr_settype)(pthread_mutexattr_t *, int);
typedef int  (*piminterceptor_pthread_once)(pthread_once_t *, void (*)(void));
typedef int  (*piminterceptor_pthread_rwlock_destroy)(pthread_rwlock_t *);
typedef int  (*piminterceptor_pthread_rwlock_init)(pthread_rwlock_t *,
                const pthread_rwlockattr_t *);
typedef int  (*piminterceptor_pthread_rwlock_rdlock)(pthread_rwlock_t *);
typedef int  (*piminterceptor_pthread_rwlock_tryrdlock)(pthread_rwlock_t *);
typedef int  (*piminterceptor_pthread_rwlock_trywrlock)(pthread_rwlock_t *);
typedef int  (*piminterceptor_pthread_rwlock_unlock)(pthread_rwlock_t *);
typedef int  (*piminterceptor_pthread_rwlock_wrlock)(pthread_rwlock_t *);
typedef int  (*piminterceptor_pthread_rwlockattr_destroy)(pthread_rwlockattr_t *);
typedef int  (*piminterceptor_pthread_rwlockattr_getpshared)(const pthread_rwlockattr_t *,
          int *);
typedef int  (*piminterceptor_pthread_rwlockattr_init)(pthread_rwlockattr_t *);
typedef int  (*piminterceptor_pthread_rwlockattr_setpshared)(pthread_rwlockattr_t *, int);
typedef pthread_t (*piminterceptor_pthread_self)(void);
typedef int  (*piminterceptor_pthread_setcancelstate)(int, int *);
typedef int  (*piminterceptor_pthread_setcanceltype)(int, int *);
typedef int  (*piminterceptor_pthread_setconcurrency)(int);
typedef int  (*piminterceptor_pthread_setschedparam)(pthread_t, int ,
          const struct sched_param *);
#ifndef WIN32
typedef int  (*piminterceptor_pthread_setspecific)(pthread_key_t, const void *);
#endif
typedef void  (*piminterceptor_pthread_testcancel)(void);




/*********************************************************************************
 ******************** interface *************************************************/

int pim_pthreadinterceptorInit( void );


#ifdef __cplusplus
}
#endif

#endif  /* __OPENCL_CL_H */

