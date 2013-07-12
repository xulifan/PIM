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

#include <memory.h>
#include "pim-private.h"
#include "pimemu_pthreadinterceptor.h"

#define PIM_PTHREADICTR_MSG(msg)               \
{                                         \
   if (msg != NULL)                       \
   { \
      fprintf(stderr, "PIM PTHREADINTERCEPTOR: %s (%d): %s\n", __func__, __LINE__, msg);    \
   }\
}

#define PTHREAD_INTERCEPTOR_FUNCTION_NAME(_FUNCNAME_) piminterceptor_##_FUNCNAME_
#define DECLARE_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
	static piminterceptor_pthread_##_FUNCNAME_ PTHREAD_INTERCEPTOR_FUNCTION_NAME(_FUNCNAME_) = NULL;

#ifdef WIN32
#define GET_INTERCEPTOR_FUNCTION_ADDRESS(_FUNCNAME_) \
{ \
	PTHREAD_INTERCEPTOR_FUNCTION_NAME(_FUNCNAME_) = (piminterceptor_pthread_##_FUNCNAME_)w_pthread_##_FUNCNAME_; \
}
#else
#define GET_INTERCEPTOR_FUNCTION_ADDRESS(_FUNCNAME_) \
{ \
  char *error; \
  char func_name[128];\
    sprintf(func_name,"pthread_%s",#_FUNCNAME_); \
	PTHREAD_INTERCEPTOR_FUNCTION_NAME(_FUNCNAME_) = (piminterceptor_pthread_##_FUNCNAME_)dlsym(pthreadDllHandle, func_name); \
	 error = dlerror(); \
    if (error != NULL) { \
       PIM_PTHREADICTR_MSG("No function"); \
	} \
}

#endif

#define GET_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
piminterceptor_pthread##_FUNCNAME_ get##_FUNCNAME_( void) \
{ \
	return	(PTHREAD_INTERCEPTOR_FUNCTION_NAME(_FUNCNAME_)); \
}

DECLARE_INTERCEPTOR_FUNCTION(attr_destroy);
DECLARE_INTERCEPTOR_FUNCTION(attr_getdetachstate);
DECLARE_INTERCEPTOR_FUNCTION(attr_getguardsize);
DECLARE_INTERCEPTOR_FUNCTION(attr_getinheritsched);
DECLARE_INTERCEPTOR_FUNCTION(attr_getschedparam);
DECLARE_INTERCEPTOR_FUNCTION(attr_getschedpolicy);
DECLARE_INTERCEPTOR_FUNCTION(attr_getscope);
DECLARE_INTERCEPTOR_FUNCTION(attr_getstackaddr);
DECLARE_INTERCEPTOR_FUNCTION(attr_getstacksize);
DECLARE_INTERCEPTOR_FUNCTION(attr_init);
DECLARE_INTERCEPTOR_FUNCTION(attr_setdetachstate);
DECLARE_INTERCEPTOR_FUNCTION(attr_setguardsize);
DECLARE_INTERCEPTOR_FUNCTION(attr_setinheritsched);
DECLARE_INTERCEPTOR_FUNCTION(attr_setschedparam);
DECLARE_INTERCEPTOR_FUNCTION(attr_setschedpolicy);
DECLARE_INTERCEPTOR_FUNCTION(attr_setscope);
DECLARE_INTERCEPTOR_FUNCTION(attr_setstackaddr);
DECLARE_INTERCEPTOR_FUNCTION(attr_setstacksize);
DECLARE_INTERCEPTOR_FUNCTION(cancel);
//DECLARE_INTERCEPTOR_FUNCTION(cleanup_push);
//DECLARE_INTERCEPTOR_FUNCTION(cleanup_pop);
DECLARE_INTERCEPTOR_FUNCTION(cond_broadcast);
DECLARE_INTERCEPTOR_FUNCTION(cond_destroy);
DECLARE_INTERCEPTOR_FUNCTION(cond_init);
DECLARE_INTERCEPTOR_FUNCTION(cond_signal);
DECLARE_INTERCEPTOR_FUNCTION(cond_timedwait);
DECLARE_INTERCEPTOR_FUNCTION(cond_wait);
DECLARE_INTERCEPTOR_FUNCTION(condattr_destroy);
DECLARE_INTERCEPTOR_FUNCTION(condattr_getpshared);
DECLARE_INTERCEPTOR_FUNCTION(condattr_init);
DECLARE_INTERCEPTOR_FUNCTION(condattr_setpshared);
DECLARE_INTERCEPTOR_FUNCTION(create);
DECLARE_INTERCEPTOR_FUNCTION(detach);
DECLARE_INTERCEPTOR_FUNCTION(equal);
DECLARE_INTERCEPTOR_FUNCTION(exit);
DECLARE_INTERCEPTOR_FUNCTION(getconcurrency);
DECLARE_INTERCEPTOR_FUNCTION(getschedparam);
#ifndef WIN32
DECLARE_INTERCEPTOR_FUNCTION(getspecific);
#endif
DECLARE_INTERCEPTOR_FUNCTION(join);
#ifndef WIN32
DECLARE_INTERCEPTOR_FUNCTION(key_create);
DECLARE_INTERCEPTOR_FUNCTION(key_delete);
#endif
DECLARE_INTERCEPTOR_FUNCTION(mutex_destroy);
DECLARE_INTERCEPTOR_FUNCTION(mutex_getprioceiling);
DECLARE_INTERCEPTOR_FUNCTION(mutex_init);
DECLARE_INTERCEPTOR_FUNCTION(mutex_lock);
DECLARE_INTERCEPTOR_FUNCTION(mutex_setprioceiling);
DECLARE_INTERCEPTOR_FUNCTION(mutex_trylock);
DECLARE_INTERCEPTOR_FUNCTION(mutex_unlock);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_destroy);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_getprioceiling);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_getprotocol);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_getpshared);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_gettype);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_init);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_setprioceiling);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_setprotocol);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_setpshared);
DECLARE_INTERCEPTOR_FUNCTION(mutexattr_settype);
DECLARE_INTERCEPTOR_FUNCTION(once);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_destroy);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_init);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_tryrdlock);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_trywrlock);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_rdlock);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_unlock);
DECLARE_INTERCEPTOR_FUNCTION(rwlock_wrlock);
DECLARE_INTERCEPTOR_FUNCTION(rwlockattr_destroy);
DECLARE_INTERCEPTOR_FUNCTION(rwlockattr_getpshared);
DECLARE_INTERCEPTOR_FUNCTION(rwlockattr_init);
DECLARE_INTERCEPTOR_FUNCTION(rwlockattr_setpshared);
DECLARE_INTERCEPTOR_FUNCTION(self);
DECLARE_INTERCEPTOR_FUNCTION(setcancelstate);
DECLARE_INTERCEPTOR_FUNCTION(setcanceltype);
DECLARE_INTERCEPTOR_FUNCTION(setconcurrency);
DECLARE_INTERCEPTOR_FUNCTION(setschedparam);
#ifndef WIN32
DECLARE_INTERCEPTOR_FUNCTION(setspecific);
#endif
DECLARE_INTERCEPTOR_FUNCTION(testcancel);


/*****************************************************************************/
/************ internals *****************************************************/
//GET_INTERCEPTOR_FUNCTION_ADDRESS

static
int pim_pthreadGET_INTERCEPTOR_FUNCTION_ADDRESSes( void* pthreadDllHandle )
{
int ret = 0;
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_destroy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getdetachstate);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getguardsize);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getinheritsched);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getschedparam);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getschedpolicy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getscope);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getstackaddr);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_getstacksize);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_init);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setdetachstate);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setguardsize);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setinheritsched);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setschedparam);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setschedpolicy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setscope);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setstackaddr);
GET_INTERCEPTOR_FUNCTION_ADDRESS(attr_setstacksize);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cancel);
//GET_INTERCEPTOR_FUNCTION_ADDRESS(cleanup_push);
//GET_INTERCEPTOR_FUNCTION_ADDRESS(cleanup_pop);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cond_broadcast);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cond_destroy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cond_init);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cond_signal);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cond_timedwait);
GET_INTERCEPTOR_FUNCTION_ADDRESS(cond_wait);
GET_INTERCEPTOR_FUNCTION_ADDRESS(condattr_destroy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(condattr_getpshared);
GET_INTERCEPTOR_FUNCTION_ADDRESS(condattr_init);
GET_INTERCEPTOR_FUNCTION_ADDRESS(condattr_setpshared);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(create);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(detach);
GET_INTERCEPTOR_FUNCTION_ADDRESS(equal);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(exit);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(getconcurrency);
GET_INTERCEPTOR_FUNCTION_ADDRESS(getschedparam);
GET_INTERCEPTOR_FUNCTION_ADDRESS(getspecific)
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(join);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(key_create);
GET_INTERCEPTOR_FUNCTION_ADDRESS(key_delete);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_destroy);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_getprioceiling);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_init);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_lock);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_setprioceiling);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_trylock);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutex_unlock);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_destroy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_getprioceiling);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_getprotocol);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_getpshared);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_gettype);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_init);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_setprioceiling);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_setprotocol);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_setpshared);
GET_INTERCEPTOR_FUNCTION_ADDRESS(mutexattr_settype);
GET_INTERCEPTOR_FUNCTION_ADDRESS(once);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_destroy);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_init);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_tryrdlock);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_trywrlock);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_rdlock);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_unlock);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlock_wrlock);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlockattr_destroy);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlockattr_getpshared);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlockattr_init);
GET_INTERCEPTOR_FUNCTION_ADDRESS(rwlockattr_setpshared);
#endif
GET_INTERCEPTOR_FUNCTION_ADDRESS(self);
#ifndef WIN32
GET_INTERCEPTOR_FUNCTION_ADDRESS(setcancelstate);
GET_INTERCEPTOR_FUNCTION_ADDRESS(setcanceltype);
GET_INTERCEPTOR_FUNCTION_ADDRESS(setconcurrency);
GET_INTERCEPTOR_FUNCTION_ADDRESS(setschedparam);
GET_INTERCEPTOR_FUNCTION_ADDRESS(setspecific);
GET_INTERCEPTOR_FUNCTION_ADDRESS(testcancel);
#endif
   return(ret);
}



static
int dummy = 0; 

static
void *
pthreadDllHandle = NULL; 


int pim_pthreadinterceptorInit( void )
{
int ret = 0;
 
   if ( NULL == pthreadDllHandle )
   {

#ifdef WIN32
	   pthreadDllHandle = &dummy;
#else
   //Load the dll and keep the handle to it

        pthreadDllHandle = dlopen("libpthread.so.0", RTLD_LAZY);
#endif
   // If the handle is valid, try to get the function address. 
        if (NULL != pthreadDllHandle) 
        { 
	        PIM_PTHREADICTR_MSG("Loaded pthread lib");
      //Get pointers to OCL functions with GET_INTERCEPTOR_FUNCTION_ADDRESS MACRO:
	        ret = pim_pthreadGET_INTERCEPTOR_FUNCTION_ADDRESSes(pthreadDllHandle);
         }
         else
         {
	        ret = -100;
	        PIM_PTHREADICTR_MSG("Cannot load pthread lib");
         }

   }

   return(ret);
}


int   pthread_attr_destroy(pthread_attr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}

}

int   pthread_attr_getdetachstate(const pthread_attr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getdetachstate))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getdetachstate)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_getguardsize(const pthread_attr_t *a, size_t *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getguardsize))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getguardsize)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
	
}

int   pthread_attr_getinheritsched(const pthread_attr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getinheritsched))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getinheritsched)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}


int   pthread_attr_getschedparam(const pthread_attr_t *a,
          struct sched_param *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getschedparam))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getschedparam)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_getschedpolicy(const pthread_attr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getschedpolicy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getschedpolicy)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_attr_getscope(const pthread_attr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getscope))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getscope)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_getstackaddr(const pthread_attr_t *a, void **b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getstackaddr))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getstackaddr)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_getstacksize(const pthread_attr_t *a, size_t *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getstacksize))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_getstacksize)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_attr_init(pthread_attr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_init))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_init)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
	
}

int   pthread_attr_setdetachstate(pthread_attr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setdetachstate))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setdetachstate)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_attr_setguardsize(pthread_attr_t *a, size_t b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setguardsize))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setguardsize)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_setinheritsched(pthread_attr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setinheritsched))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setinheritsched)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_setschedparam(pthread_attr_t *a,
          const struct sched_param *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setschedparam))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setschedparam)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_attr_setschedpolicy(pthread_attr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setschedpolicy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setschedpolicy)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_attr_setscope(pthread_attr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setscope))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setscope)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_attr_setstackaddr(pthread_attr_t *a, void *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setstackaddr))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setstackaddr)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}			
}

int   pthread_attr_setstacksize(pthread_attr_t *a, size_t b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setstacksize))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(attr_setstacksize)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_cancel(pthread_t a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cancel))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cancel)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}

}


/************************ MACRO *************************************/
#if 0
void  pthread_cleanup_push(void (*routine)(void *),
                                 void *arg)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cleanup_push))
	{
		PTHREAD_INTERCEPTOR_FUNCTION_NAME(cleanup_push)(routine,arg);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}		
}

void  pthread_cleanup_pop(int a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cleanup_pop))
	{
		PTHREAD_INTERCEPTOR_FUNCTION_NAME(cleanup_pop)(a);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}		
	
}

#endif





int   pthread_cond_broadcast(pthread_cond_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_broadcast))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_broadcast)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_cond_destroy(pthread_cond_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_cond_init(pthread_cond_t *a, const pthread_condattr_t *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_init))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_init)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}			
}

int   pthread_cond_signal(pthread_cond_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_signal))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_signal)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_cond_timedwait(pthread_cond_t *a, 
          pthread_mutex_t *b, const struct timespec *c)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_timedwait))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_timedwait)(a,b,c));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}			
}

int   pthread_cond_wait(pthread_cond_t *a, pthread_mutex_t *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_wait))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(cond_wait)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_condattr_destroy(pthread_condattr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_condattr_getpshared(const pthread_condattr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_getpshared))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_getpshared)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}		
}

int   pthread_condattr_init(pthread_condattr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_init))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_init)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_condattr_setpshared(pthread_condattr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_setpshared))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(condattr_setpshared)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}


int
pthread_create (pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(create) )
	{
	   if ( isEmuMode()  ) 
	   {

       launchThreadStruct * currThread = pim_thread_getlaunchstruct();
       launchThreadStruct *pass_ident = (launchThreadStruct *)calloc(1, sizeof(launchThreadStruct));

	        if ( currThread->ocl_thread == 0 )
			{
// stop here
                OnStopCPUCounters();
			}

// bump order
			pim_thread_incorder();

	        pass_ident->func = start_routine;
	        pass_ident->per_call_args = arg;
// pass counting through the chain
			pass_ident->counting = isThreadCounting();
			pass_ident->parent_id =  GETTID();
			pass_ident->call_order = pim_thread_order(); 
			pass_ident->pim_id = (currThread->about_launch_pim_id != PIM_INVALID_ID) ?
	                               currThread->about_launch_pim_id : currThread->pim_id;;
			pass_ident->ocl_thread = isOclThread();
			setAboutToLaunchPIMID(PIM_INVALID_ID);

	        PTHREAD_INTERCEPTOR_FUNCTION_NAME(create)(thread, attr, pimemu_wrapperThreadFunc, pass_ident);
	        currThread->child_pid = *(uint32_t*)(*thread);


			OnThreadLaunch(currThread);
	   }
	   else
	   {
	        PTHREAD_INTERCEPTOR_FUNCTION_NAME(create) (thread, attr,start_routine, arg);
	   }
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_detach(pthread_t a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(detach))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(detach)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

int   pthread_equal(pthread_t a, pthread_t b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(equal))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(equal)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}	
}

void
pthread_exit (void *retval)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(exit) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(exit) (retval);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}

}

int   pthread_getconcurrency(void)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(getconcurrency))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(getconcurrency)());
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}

}

int   pthread_getschedparam(pthread_t a, int *b, struct sched_param *c)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(getschedparam))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(getschedparam)(a,b,c));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

#ifndef WIN32

void *pthread_getspecific(pthread_key_t key)
{
void * ret = 0;
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(getspecific) )
	{
		ret = PTHREAD_INTERCEPTOR_FUNCTION_NAME(getspecific)(key);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
	return(ret);
}

#endif

int
pthread_join (pthread_t th_, void **thread_return)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(join) )
	{
    uint32_t child_th = *(unsigned int*)th_;
        if (isEmuMode())
    	{

         launchThreadStruct * currThread = pim_thread_getlaunchstruct();

            currThread->child_pid = child_th ;

// bump order
			pim_thread_incorder();

	        OnThreadJoin(currThread);
	    

	        PTHREAD_INTERCEPTOR_FUNCTION_NAME(join) (th_,thread_return);
            
	
             OnStartCPUCounters();
			
		}
		else
		{

			 PTHREAD_INTERCEPTOR_FUNCTION_NAME(join) (th_,thread_return);
		}

	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

#ifndef WIN32

int   pthread_key_create(pthread_key_t * key, void (*func)(void *))
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(key_create))
	{
		PTHREAD_INTERCEPTOR_FUNCTION_NAME(key_create)(key, func);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_key_delete(pthread_key_t  key)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(key_delete))
	{
		PTHREAD_INTERCEPTOR_FUNCTION_NAME(key_delete)(key);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

#endif



/*
    Mutex
*/

int   pthread_mutex_destroy(pthread_mutex_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutex_getprioceiling(const pthread_mutex_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_getprioceiling))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_getprioceiling)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int
pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *mutex_attr)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_init) )
	{
   	     PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_init) (mutex, mutex_attr);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}



int
pthread_mutex_lock (pthread_mutex_t *mutex)
{

	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_lock) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_lock) (mutex);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_mutex_setprioceiling(pthread_mutex_t *a, int b, int *c)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_setprioceiling))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_setprioceiling)(a,b,c));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutex_trylock(pthread_mutex_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_trylock))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_trylock)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_unlock) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutex_unlock) (mutex);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_mutexattr_destroy(pthread_mutexattr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *a,
          int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_getprioceiling))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_getprioceiling)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_getprotocol(const pthread_mutexattr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_getprotocol))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_getprotocol)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_getpshared(const pthread_mutexattr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_getpshared))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_getpshared)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_gettype(const pthread_mutexattr_t *a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_gettype))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_gettype)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_init(pthread_mutexattr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_init))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_init)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_setprioceiling(pthread_mutexattr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_setprioceiling))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_setprioceiling)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_setprotocol(pthread_mutexattr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_setprotocol))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_setprotocol)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_setpshared(pthread_mutexattr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_setpshared))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_setpshared)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_mutexattr_settype(pthread_mutexattr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_settype))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(mutexattr_settype)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_once(pthread_once_t *a, void (*b)(void))
{

/*********************************************************************/
/****  LOAD PTHREAD LIBRARY HERE BEFORE THE FIRST ONCE ***************/
	if ( !PTHREAD_INTERCEPTOR_FUNCTION_NAME(once) )
	{
	    pim_pthreadinterceptorInit();
		pim_cliterceptorInit();
	}
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(once))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(once)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_rwlock_destroy(pthread_rwlock_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int  pthread_rwlock_init(pthread_rwlock_t *lock,
                               const pthread_rwlockattr_t *attr)
{

    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_init) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_init)(lock, attr);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_rwlock_tryrdlock(pthread_rwlock_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_tryrdlock))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_tryrdlock)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_rwlock_trywrlock(pthread_rwlock_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_trywrlock))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_trywrlock)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}


int   pthread_rwlock_rdlock(pthread_rwlock_t *lock)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_rdlock) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_rdlock)(lock);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_rwlock_unlock(pthread_rwlock_t *lock)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_unlock) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_unlock)(lock);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_rwlock_wrlock(pthread_rwlock_t *lock)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_wrlock) )
	{
	    PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlock_wrlock)(lock);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}

int   pthread_rwlockattr_destroy(pthread_rwlockattr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_destroy))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_destroy)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *a,
                                    int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_getpshared))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_getpshared)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_rwlockattr_init(pthread_rwlockattr_t *a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_init))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_init)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_rwlockattr_setpshared(pthread_rwlockattr_t *a, int b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_setpshared))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(rwlockattr_setpshared)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}


pthread_t pthread_self(void)
{

pthread_t ret = 0;

    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(self) )
	{
	     ret = PTHREAD_INTERCEPTOR_FUNCTION_NAME(self)();
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}

   return(ret);
}

int   pthread_setcancelstate(int a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(setcancelstate))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(setcancelstate)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_setcanceltype(int a, int *b)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(setcanceltype))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(setcanceltype)(a,b));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_setconcurrency(int a)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(setconcurrency))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(setconcurrency)(a));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}

int   pthread_setschedparam(pthread_t a, int b,
          const struct sched_param *c)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(setschedparam))
	{
		return(PTHREAD_INTERCEPTOR_FUNCTION_NAME(setschedparam)(a,b,c));
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
		return 0;	
	}
}




#ifndef WIN32
int   pthread_setspecific(pthread_key_t key, const void *data)
{
    if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(setspecific) )
	{
		PTHREAD_INTERCEPTOR_FUNCTION_NAME(setspecific)(key, data);
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
    return 0;
}
#endif

void  pthread_testcancel(void)
{
	if ( PTHREAD_INTERCEPTOR_FUNCTION_NAME(testcancel))
	{
		PTHREAD_INTERCEPTOR_FUNCTION_NAME(testcancel)();
	}
	else
	{
	    PIM_PTHREADICTR_MSG("NOT FOUND!");
	}
}

