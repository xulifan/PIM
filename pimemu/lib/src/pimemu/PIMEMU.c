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
#include "cpu_profile.h"


pthread_key_t pim_thread_info = 0;
static launchThreadStruct ehp_thread;
static pid_t master_thread = 0;

//static 
//launchThreadStruct * pim_thread_getlaunchstruct(void);

/* PIMEMU APIs */


/*****************************************************************************/
/********   date gathering helper functions  *********************************/
void itIsRuntimeCall(int call)
{
launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
     if ( launchStruct )
	 {
         launchStruct->pim_run_time = call;
	 }
}

int isItRuntimeCall(void)
{
int ret = 0;
launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
    if ( launchStruct )
	{
		ret = launchStruct->pim_run_time;
	}
    return(ret);
}

int incAndStopCnt(void)
{
int ret = 0;
launchThreadStruct *launchStruct = pim_thread_getlaunchstruct();
    if (launchStruct)
	{
		  ret = launchStruct->ocl_thread;
          launchStruct->ocl_thread++;
		  if ( launchStruct->ocl_thread == 1 )
		  {
                 OnStopCPUCounters();
		  }
		  
	}
	return(ret);
}

int decAndStartCnt(void)
{
int ret = 0;
launchThreadStruct *launchStruct = pim_thread_getlaunchstruct();

     if ( launchStruct )
	 {
		 ret = launchStruct->ocl_thread;
         launchStruct->ocl_thread--;
         if ( launchStruct->ocl_thread <= 0 )
	     {
		    
		      launchStruct->ocl_thread = 0;
		      OnStartCPUCounters();
	     }
	 }
	 return(ret);
}

int isOclThread(void)
{
int ret = 0;
launchThreadStruct *launchStruct = pim_thread_getlaunchstruct();

     if ( launchStruct )
	 {
		 ret = launchStruct->ocl_thread;
	 }
	 return(ret);
}

long long
pim_thread_incorder (void)
{
long long ret = 0;

  if ( pim_thread_info != 0 && isCapturing() && isThreadCounting() )
  {
launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);   
      if (launchStruct)
      {
	      ret = launchStruct->call_order++;
      }
  }
   return (ret);
}

long long
pim_thread_order (void)
{
long long ret = 0;
  if ( pim_thread_info != 0 )
  {
  launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
      if (launchStruct)
      {
	      ret = launchStruct->call_order;
      }
  }
  return (ret);
}

launchThreadStruct *
pim_thread_getlaunchstruct(void)
{
launchThreadStruct *launchStruct = 0;
   if ( pim_thread_info != 0 )
   {

   launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
   }

   return(launchStruct);
}

int isMasterThread( void )
{
	return(master_thread == GETTID());
}

int isThreadInEMUChain( void )
{
	return(0 != pim_thread_info);

}

int isThreadCounting()
{
int counting = 0;
   if ( pim_thread_info != 0 )
   {
   launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
      if (launchStruct)
      {
	      counting = launchStruct->counting;
      }
   }
   return(counting);
}

void setAboutToLaunchPIMID(uint32_t id)
{
launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);

    if ( launchStruct)
	{
        launchStruct->about_launch_pim_id = id;
	}

}
void setPIMID(uint32_t id)
{
launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
    if ( launchStruct)
	{
       launchStruct->pim_id = id;
	}
}

uint32_t getPIMID(void)
{
launchThreadStruct *launchStruct = (launchThreadStruct *)pthread_getspecific(pim_thread_info);
uint32_t ret = 0;
     if ( launchStruct)
	 {
	     ret = launchStruct->pim_id;
	 }
     return(ret);
}



/**************************************************************************/
/*** INTERNALS ************************************************************/


int OnStopCPUCounters(void)
{

#ifndef WIN32

    if ( isCapturing() && isThreadCounting( ) )
	{
       stop_counters();
	}
#endif
	return(0);
}

int OnStartCPUCounters(void)
{

#ifndef WIN32

    if ( isCapturing() && isThreadCounting( ) )
	{
       start_counters();
	}
#endif
	return(0);
}



int OnCounterStart(launchThreadStruct *launchStruct_)
{
    // start the chain of counting from the master thread

    launchThreadStruct * launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();

    if ( launchStruct && isCapturing() && isThreadCounting( ) )
	{	

    char msg[1024];

    	sprintf(msg, "PIMEMU_BEGIN,\t%d,\t%llu\n",launchStruct->thread_id, launchStruct->call_order);

        pimemuTimelineSendString(msg);
#ifndef WIN32
    // Start counters on main thread
        start_counters();
#endif
	}
	return(0);
}


int OnCounterEnd(launchThreadStruct *launchStruct_)
{

    launchThreadStruct * launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();

    if ( launchStruct && isCapturing() && isThreadCounting( ) )
	{	
    char msg[1024];
#ifndef WIN32
    //stop counting
        stop_counters();

    //print counts, returns 0 if buffer is too small to hold counter information, else returns 1 on success
        if( print_counters( launchStruct->call_order,launchStruct->thread_id, msg, 1024) == 0) {
           fprintf(stderr, " PAPI print_counters, buffer size too small \n");
        }
    
        pimemuCPUCountersSendString(msg);
#endif
	    sprintf(msg, "PIMEMU_END,\t%d,\t%llu\n",launchStruct->thread_id, launchStruct->call_order);
        pimemuTimelineSendString(msg);

	}

	return(0);
}

static
int OnThreadStart(launchThreadStruct *launchStruct_)
{
    launchThreadStruct * launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();
    if ( launchStruct && isCapturing() && isThreadCounting( ) )
	{
    char msg[1024];
    pthread_t th = pthread_self(); 

         sprintf(msg, "PT_START,\t%d,\t%llu,\t%u,\t%llu,\t%u\n",launchStruct->thread_id,
		                                       launchStruct->call_order,
											   launchStruct->parent_id,
											   launchStruct->parent_call_order,
				                               *(unsigned int*)(th));
         pimemuTimelineSendString(msg);

#ifndef WIN32
    // Register thread with PAPI
         if ( (thread_registerPerfCounters()) == 0) {
              fprintf(stderr, "error from papi\n");
         }
    // Add events for profiling
    //start counters
         start_counters();
#endif
	}

	return(0);
}

static
int OnThreadExit(launchThreadStruct *launchStruct_)
{
    launchThreadStruct * launchStruct = 0;

//    pim_thread_incorder ();

    launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();


    if ( launchStruct && isCapturing() && isThreadCounting( ) )
	{
    char msg[1024];

#ifndef WIN32
    //stop counting
          stop_counters();

    //print counts, returns 0 if buffer is too small to hold counter information, else returns 1 on success
          if( print_counters( launchStruct->call_order,launchStruct->thread_id, msg, 1024) == 0) {
               fprintf(stderr, " PAPI print_counters, buffer size too small \n");
          }
    
          pimemuCPUCountersSendString(msg);
    // Unregister thread with PAPI
          if ( (thread_unregisterPerfCounters()) == 0) {
               fprintf(stderr, "error from papi\n");
          }
#endif

          sprintf(msg, "PT_EXIT,\t%d,\t%llu\n",launchStruct->thread_id, launchStruct->call_order);
          pimemuTimelineSendString(msg);    
    
	}

	return(0);
}


int OnThreadLaunch(launchThreadStruct *launchStruct_)
{

    launchThreadStruct * launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();

    if ( launchStruct && isCapturing() && isThreadCounting( ) )
	{
char msg[1024];

          sprintf(msg, "PT_LAUNCH,\t%d,\t%llu,\t%u\n",launchStruct->thread_id, launchStruct->call_order, (launchStruct->child_pid));
          pimemuTimelineSendString(msg);

#ifndef WIN32

 // Counters are stopped before entering this function.
 // They're restarted outside this function as well.
 //print counts, returns 0 if buffer is too small to hold counter information, else returns 1 on success
           if( print_counters( launchStruct->call_order,launchStruct->thread_id, msg, 1024) == 0) {
               fprintf(stderr, " PAPI print_counters, buffer size too small \n");
           }

           pimemuCPUCountersSendString(msg);




           start_counters();

#endif
 
    
	}
   return(0);
}



int OnThreadJoin(launchThreadStruct *launchStruct_)
{

 launchThreadStruct * launchStruct = 0;
 //    pim_thread_incorder ();

     launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();

    if ( launchStruct && isCapturing() && isThreadCounting( ) )
	{
     char msg[1024];

#ifndef WIN32

           stop_counters();
  // Counters are stopped before entering this function.
  // They're restarted outside this function as well.
  //print counts, returns 0 if buffer is too small to hold counter information, else returns 1 on success
          if( print_counters( launchStruct->call_order,launchStruct->thread_id, msg, 1024) == 0) {
              fprintf(stderr, " PAPI print_counters, buffer size too small \n");
          }
          pimemuCPUCountersSendString(msg);
#endif

          sprintf(msg, "PT_JOIN,\t%d,\t%llu,\t%u\n",launchStruct->thread_id, launchStruct->call_order,  (launchStruct->child_pid));
          pimemuTimelineSendString(msg);

	}
  return(0);
}




int OnOclWaitEvent(launchThreadStruct *launchStruct_, const char * outside_msg)
{

    launchThreadStruct * launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();

    if ( launchStruct&& isCapturing() && isThreadCounting( ) )
    {

#ifndef WIN32
        {
            char msg[1024];

//            stop_counters();
            // Counters are stopped before entering this function.
            // They're restarted outside this function as well.
            //print counts, returns 0 if buffer is too small to hold counter information, else returns 1 on success
            if( print_counters( launchStruct->call_order,launchStruct->thread_id, msg, 1024) == 0) {
                fprintf(stderr, " PAPI print_counters, buffer size too small \n");
            }
            pimemuCPUCountersSendString(msg);
        }
#endif
        pimemuTimelineSendString(outside_msg);

#ifndef WIN32

         start_counters();
#endif
    }
    return(0);
}



static
int OnPIMEMUDEstruct(launchThreadStruct *launchStruct_)
{

    launchThreadStruct * launchStruct = (launchStruct_ != NULL) ? launchStruct_ : pim_thread_getlaunchstruct();
    if ( launchStruct && isCapturing() )
	{
	char msg[1024];
 
	    sprintf(msg, "PIMEMU_LAST,\t%d,\t%llu\n",launchStruct->thread_id, launchStruct->call_order);

        pimemuTimelineSendString(msg);
        pimemuDeinitCPUCounterStreaming();  
        pimemuDeinitTimelineStreaming();
   }

	return(0);
}


int is_pimemu_init(void)
{
   return( pim_thread_info != 0 && isEmuMode() );
}

int pimemu_constructor(void)
{
launchThreadStruct *launchStruct = &ehp_thread;
launchThreadStruct *launchTestStruct;

	master_thread = GETTID();

    pthread_key_create(&pim_thread_info, NULL);

    memset(launchStruct, 0, sizeof(launchThreadStruct));

    pthread_setspecific(pim_thread_info, launchStruct);
	launchTestStruct = pim_thread_getlaunchstruct();
	if ( !launchTestStruct )
	{
		return(PIM_BUILTWITHOUT_PTHREAD);
	}

    setPIMID(PIM_EHP_ID);
    setAboutToLaunchPIMID(PIM_INVALID_ID);

	launchStruct->thread_id = GETTID();

	return(PIM_SUCCESS);
}


int pimemu_destructor(void)
{
launchThreadStruct *launchStruct = &ehp_thread;
   

      pim_emu_end_impl();
      OnPIMEMUDEstruct(launchStruct);
  
   return(0);
}


int pim_emu_begin_impl(void)
{
launchThreadStruct *launchStruct = &ehp_thread;

    if ( isMasterThread() )
	{
	    if (pim_thread_order() >  0)
	    {
           pim_thread_incorder ();
	    }


// only on master thread level 
        if ( isCapturing() && isCounting() )
	    {

            pimemuInitTimelineStreaming();
	
#ifndef WIN32
             pimemuInitCPUCounterStreaming();
#endif
			 launchStruct->counting = 1;

             OnCounterStart(launchStruct); 
	    }
	}
	return(0);
}

int pim_emu_end_impl(void)
{
launchThreadStruct *launchStruct = &ehp_thread;

// only on master thread level
    if ( isCapturing() && isCounting() && isMasterThread()  )
	{
//	     clInterceptorCaptureOrphanWaitForOurEvents( );
         pim_thread_incorder ();
         OnCounterEnd(launchStruct);
//  break the chain of counting from the master thread
         launchStruct->counting = 0;
	}

	return(0);
}


void * pimemu_wrapperThreadFunc(void *args)
{
void *ret = 0;
launchThreadStruct *launchStruct = (launchThreadStruct *)args;

launchThreadStruct *threadStruct = (launchThreadStruct *)calloc(1, sizeof(launchThreadStruct));
   pthread_setspecific(pim_thread_info, threadStruct);

   // pass counting through the chain
   threadStruct->thread_id = GETTID();
   threadStruct->counting = launchStruct->counting;
   threadStruct->parent_id = launchStruct->parent_id;
   threadStruct->parent_call_order = launchStruct->call_order;
   threadStruct->pim_id = launchStruct->pim_id;
   threadStruct->about_launch_pim_id = PIM_INVALID_ID;
   threadStruct->ocl_thread = launchStruct->ocl_thread;

   OnThreadStart(threadStruct);

   launchStruct->func(launchStruct->per_call_args);

// bump order
   pim_thread_incorder();
   OnThreadExit(threadStruct);

   free(args);
   free(threadStruct);

   return(ret);
}



