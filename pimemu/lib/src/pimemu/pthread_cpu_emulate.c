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

#include "pthread_cpu_emulate.h"

#if defined(WIN32)

//#include <stdlib.h>
//#include <windows.h>
size_t cache_line_size() {
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);

    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);
    return line_size;
}

#elif defined(linux)

#include <stdio.h>
size_t cache_line_size() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int i = 0;
    if (p) {
        fscanf(p, "%d", &i);
        fclose(p);
    }
    return i;
}

#else
#error Unrecognized platform
#endif


#ifdef WIN32
size_t getpagesize(void)
{
  SYSTEM_INFO siSysInfo;
  // Copy the hardware information to the SYSTEM_INFO structure. 
      GetSystemInfo(&siSysInfo); 
      return(siSysInfo.dwPageSize); 
}

#endif


void *pim_cpu_malloc(size_t size, pim_device_id device, pim_acc_flags acc_flags)
{
    // Dead simple beginning
    return malloc(size);
}

void *pim_cpu_malloc_aligned(size_t size, size_t alignment, pim_device_id device, pim_acc_flags acc_flags, void ** actual_ptr)
{
void * ret = NULL;
long long iptr; 
 
    size_t aligned_size = size + alignment;
	*actual_ptr = malloc(aligned_size);
	iptr = (long long)(*actual_ptr);
	iptr = (( iptr + alignment - 1) / alignment);
	iptr *= alignment;
	ret = (void*)iptr;
    return (ret);
}

int spawn_cpu_thread(pim_f to_launch, void **args, size_t *arg_sizes,
        size_t nargs, pim_device_id device)
{
    int ret;
    pthread_t *thread;
    pthread_attr_t *attr;
    void *(*start_routine)(void *);
    void *thread_arg;
    
    // pthread_create has 4 arguments, but one of them is the
    // function pointer, which is passed to this function througnh the
    // pim_f variable "to_launch".
    if (nargs != NUM_PTHREAD_ARGUMENTS ||
        arg_sizes[PTHREAD_ARG_THREAD] != sizeof(pthread_t) ||
        arg_sizes[PTHREAD_ARG_ATTR] != sizeof(pthread_attr_t) ||
        arg_sizes[PTHREAD_ARG_INPUT] != sizeof(void *)) {
        return PIM_INVALID_PARAMETER;
    }

    thread = ((pthread_t *)args[PTHREAD_ARG_THREAD]);
    attr = ((pthread_attr_t *)args[PTHREAD_ARG_ATTR]);
    thread_arg = args[PTHREAD_ARG_INPUT];
    start_routine = to_launch.func_ptr;

	if( isEmuMode() )
    {
    long long launch_order;
	launchThreadStruct *currlaunchStruct;
	char msg[1024];


		   setAboutToLaunchPIMID(device);

		   ret = pthread_create(thread, attr, start_routine, thread_arg);

		   launch_order= pim_thread_order(); 
		   currlaunchStruct = pim_thread_getlaunchstruct();		   

		  if ( isCapturing() && isThreadCounting() )
		  {
               sprintf(msg, "PT_LAUNCH_PIM,\t%u,\t%llu,\t%u\n",
			       	     currlaunchStruct->thread_id, launch_order, device);

		        stackTimeLineMsg(msg);
		  }

	
	}
	else
	{
			   ret = pthread_create(thread, attr, start_routine, thread_arg);
	}
 

	return(ret);
}
