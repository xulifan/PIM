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

#include <stdlib.h>
#include <stdio.h>
#ifdef EMULATE_PTHREADS
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include <string.h>
#include "cpu_profile.h"

#define NUM_EVENTS 4 
static __thread int retval,i;
static __thread int EventSet=PAPI_NULL;
static __thread int events[NUM_EVENTS];
static __thread long long values[NUM_EVENTS];
static __thread long long elapsed_us, elapsed_cyc;
static __thread int events_added=0;

static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log=NULL;
static int write_log=0;
static char *logname=NULL;

static int added_events_to_eventset(){
	return events_added;
}

static void add_perfEvents() {
  retval=0;
  
  if ( (retval=PAPI_create_eventset( &EventSet ))!=PAPI_OK){
        exit(-1);
  }

  if( (retval=PAPI_event_name_to_code("RETIRED_INSTRUCTIONS:u:c=1:i=0",&events[0])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 1");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("RETIRED_INSTRUCTIONS:u:c=0",&events[1])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 2");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("CPU_CLK_UNHALTED:u",&events[2])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 3");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("L2_CACHE_MISS:ALL:u",&events[3])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 4");
        exit(-1);
  }

  if ( (retval=PAPI_add_events(EventSet, events, NUM_EVENTS))!=PAPI_OK){
        PAPI_perror("PAPI_add_events in pthread");
        exit(-1);
  }
	events_added=1;

}

static void cleanup_perfEvents() {
    retval = PAPI_cleanup_eventset(EventSet);
    if ( retval != PAPI_OK ) {
        PAPI_perror("PAPI_cleanup_eventset");
        exit(-1);
    }
    retval = PAPI_destroy_eventset(&EventSet);
    if ( retval != PAPI_OK) {
        PAPI_perror("PAPI_destroy_eventset");
        exit(-1);
    }
}

static int fill_buffer(char *dst, int len, int index, char *src, int count) {

	if(index + count < len ) {
  		sprintf(dst+index, "%s",src);
		index+=count;
		return index;
		
	}
	else {
		fprintf(stderr,"Buffer length of %d is not big enough to store counter data \n", len);
	}
	return -1;
}

	
int  print_counters(int exit_point, int id, char *buff, int len) {

  	pthread_mutex_lock(&print_mutex);

  	if(write_log) { 
  		fprintf(log, "%d, %d, %lld, %lld", exit_point, id , elapsed_us, elapsed_cyc);
   
  		for(i=0;i<NUM_EVENTS;i++) {
        
    			fprintf(log,", %lld", values[i]);
  		}
  		fprintf(log,"\n");
   	}
	if(buff) {
		if(len<1) {
			fprintf(stderr,"print_counters() was called with char buffer of illegal length %d\n",len);
  			pthread_mutex_unlock(&print_mutex);
			return 0;
			
		}
		char tmp[512];
		int count=0;
		int index=0;

		memset(tmp,'\0',512);
		memset(buff,'\0',len);

  		count=snprintf(tmp,512, "%d, %d, %lld, %lld", exit_point, id , elapsed_us, elapsed_cyc);
		if( (index=fill_buffer(buff, len, index, tmp,count)) == -1) {
  			pthread_mutex_unlock(&print_mutex);
			return 0;
		}
		
 
  		for(i=0;i<NUM_EVENTS;i++) {
	    		count=snprintf(tmp, 512,", %lld", values[i]);
			if( (index=fill_buffer(buff, len, index, tmp,count)) == -1) {
  				pthread_mutex_unlock(&print_mutex);
				return 0;
			}
		}
	    	count=snprintf(tmp, 512," \n");
		if( (index=fill_buffer(buff, len, index, tmp,count)) == -1) {
  			pthread_mutex_unlock(&print_mutex);
			return 0;
		}
	}
	

  	pthread_mutex_unlock(&print_mutex);
	return 1;
}


void start_counters() {
  retval=0;
	
	if(!added_events_to_eventset()) {
		add_perfEvents();
	}
  
  if ( (retval = PAPI_start(EventSet)) != PAPI_OK ){
        PAPI_perror("PAPI_start");
        exit(-1);
  }
  elapsed_us = PAPI_get_virt_usec(  );
  elapsed_cyc = PAPI_get_virt_cyc(  );

  return;
}

void stop_counters() {
  retval=0;

  elapsed_us = PAPI_get_virt_usec(  ) - elapsed_us;
  elapsed_cyc = PAPI_get_virt_cyc(  ) - elapsed_cyc;

  if( (retval = PAPI_stop(EventSet, values) ) != PAPI_OK ){
        PAPI_perror("PAPI_stop");
        exit(-1);
    }
    return;
}

int thread_registerPerfCounters()
{
  retval=0;

  retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
    PAPI_perror("PAPI_register_thread");
    return(0);
  }
  return (1);
}

int thread_unregisterPerfCounters(){
  retval=0;
  if (added_events_to_eventset()) {
      cleanup_perfEvents();
  }
  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK ) {
    PAPI_perror("PAPI_unregister_thread");
    return(0);
  }
  return (1);
}



int init_perfCounters(char *name) {

  retval=0;
  //*** Init Papi Library and register it to support pthreads **********
  if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ){
    PAPI_perror("PAPI_library_init");
    return(0);
  }
  if ( ( retval =PAPI_thread_init( ( unsigned long ( * )( void ) ) ( pthread_self ) ) ) != PAPI_OK ) {
    PAPI_perror("PAPI_thread_init");
    return(0);
  }
 
   
  if(!name) {
    //fprintf(stderr, "Log file name not specified printing to stdout \n");
    //log=stdout;
    write_log=0;	
    logname=NULL;

  }
  else {
    write_log=1;
    int len=strlen(name);
    logname=(char *)malloc(len*sizeof(char));
    strcpy(logname, name);

    if ( (log= (FILE *) fopen(name,"w")) == NULL) {
      fprintf(stderr,"Could not open file %s for writing \n",name);
      return(0);
    }
  }
  
 
  return (1);
}

int print_counterheader(char *buff, int len) {
    if(write_log) {
        fprintf(log,"Exec_Order, ThreadId, elapsed_us, elapsed_uc, COMMIT_CYC, INSTRUCTIONS, CLOCK_CYC, L2_CACHE_MISS\n");
    }
    if(buff) {
        if(len<1) {
            fprintf(stderr,"print_counters() was called with char buffer of illegal length %d\n",len);
            return 0;
        }

        char tmp[512];
        int count=0;
        int index=0;
        count=snprintf(tmp,512,"Exec_Order, ThreadId, elapsed_us, elapsed_uc, COMMIT_CYC, INSTRUCTIONS, CLOCK_CYC, L2_CACHE_MISS\n");
        if( (index=fill_buffer(buff, len, index, tmp, count)) == -1) {
            return 0;	
        }
    }
    return 1;
}

void shutdown_perfCounters() {
    PAPI_shutdown();

    if(write_log) {	
    	fclose(log);
    }	
    //    pthread_mutex_destroy(&print_mutex);
    PRINT_DEBUG("Success!\n");

}
