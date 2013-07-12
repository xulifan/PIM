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

#ifndef __PIM_EMU_H_
#define __PIM_EMU_H_

#include <CL/cl.h>

#ifdef WIN32
#define GETTID() gettid()
#else
#define GETTID() syscall(SYS_gettid)
#endif


#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/






/**************************************************************************/
/*** INTERNALS ************************************************************/



typedef void * (*PIM_LAUNCH_THREAD_FUNC)(void*);


typedef struct {
    pid_t thread_id;
	pid_t parent_id;
	uint32_t pim_id;
	uint32_t about_launch_pim_id;
    uint32_t child_pid;
	PIM_LAUNCH_THREAD_FUNC func;
	void * per_call_args; 
	long long call_order;
    long long parent_call_order;
	int counting;
	int pim_run_time; // pthread create is called from PIM RUN-TIME
	int ocl_thread; // pthread create is a special OCL thread

} launchThreadStruct;


int OnStopCPUCounters(void);
int OnStartCPUCounters(void);
void itIsRuntimeCall(int call);
int isItRuntimeCall(void);
int incAndStopCnt(void);
int decAndStartCnt(void);
int isOclThread(void);
int OnCounterStart(launchThreadStruct *launchStruct_);
int OnCounterEnd(launchThreadStruct *launchStruct_);
int OnThreadLaunch(launchThreadStruct *launchStruct_);
int OnThreadJoin(launchThreadStruct *launchStruct_);
int OnOclWaitEvent(launchThreadStruct *launchStruct_, const char * msg);
void * pimemu_wrapperThreadFunc(void *args);


/**************************************************************************/
/********** specific emulation ********************************************/
#define THREAD_EHP_OCL       0
#define THREAD_PIM_OCL       1
#define THREAD_PIMLCL_OCL    2

typedef struct {
    cl_command_queue command_queue;
    cl_kernel        kernel;
    cl_uint          work_dim;
    const size_t *   global_work_offset;
    const size_t *   global_work_size;
    const size_t *   local_work_size;
    cl_uint          num_events_in_wait_list;
    const cl_event * event_wait_list;
    cl_event *       event;  
} launchOclKernelStruct;




/*****************************************************************************/
/********   date gathering helper functions  *********************************/

int pimemu_constructor(void);
int pimemu_destructor(void);
int isMasterThread(
);int isThreadInEMUChain( void );
int pim_emu_begin_impl(void);
int pim_emu_end_impl(void); 





/**********************************************************************************/
/*** passing pim ID through the threda chain *************************************/

void setAboutToLaunchPIMID(uint32_t id);
void setPIMID(uint32_t id);
uint32_t getPIMID(void);

long long
pim_thread_incorder (void);

long long
pim_thread_order (void);

int isThreadCounting();

launchThreadStruct *pim_thread_getlaunchstruct(void);


#ifdef __cplusplus
}
#endif

#endif  /* __PIM_EMU_H_ */

