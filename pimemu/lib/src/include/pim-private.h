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


#ifndef __LIB_PIM_PRIVATE_H_
#define __LIB_PIM_PRIVATE_H_
/******************************************************************************/

#ifndef WIN32

#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>

#else

// wthreads
#include "wpthread.h"

#endif

#include <sys/types.h>
#include <fcntl.h>

#include <stdio.h>
#include <assert.h>
#include <CL/cl.h>

#include "pim.h"
#include "pthread_cpu_emulate.h"
#include "opencl_gpu_emulate.h"
#include "PIMEMU.h"
// ocl interceptor
#include "pimemu_clinterceptor.h"
// pthread interceptor
#include "pimemu_pthreadinterceptor.h"



#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
/*************  global model defaults ***********************************************/

#define __EMULATE_PERF_MODE__  1
#define __EMULATE_FUNC_MODE__  0
#define __PIMEMUENV__     "PIMEMUENV"
#define __DAFUTLT_MODE__  __EMULATE_FUNC_MODE__


/************ Custom data structures used throughout the emulator ************/
// Tiny toy structures describing our system.

// ISAs
#define ISA_X86_64          1
#define ISA_ARM             2

// Pipeline Ordering
#define ORDERING_INORDER    1
#define ORDERING_OOO        2

typedef struct {
	uint32_t id;
    uint32_t freq;
	uint32_t width;
	uint32_t num_hwthreads;
    uint8_t isa;
    uint8_t ordering;
} cpu_configuration_t;

// ISAs
#define ISA_SI              1

typedef struct {
	uint32_t id;
    uint32_t freq; 
	uint32_t cus;
    uint32_t width;
    uint16_t num_instr_perclck;
    uint8_t isa;
	gpu_handle_t gpu_handle;
} gpu_configuration_t;

typedef struct {
    uint32_t id;
    uint32_t size_in_mb;
    uint32_t off_stack_bw; // expressed as whole int percentqge of bw
    uint32_t between_pim_bw; // expressed as a whle integer percentrage of bw
    uint32_t freq;              // in mhz
    uint32_t num_channels;      
    uint32_t channel_width;     // default 32
    uint32_t bits_percycle;     // DDR2 == 2
    uint32_t latency;           // in nanosec
    uint32_t off_stack_latency; // % increase in latency
    uint32_t utilization;       // in %
} dram_configuration_t;

typedef struct {
    uint32_t num_cpu_cores;
    cpu_configuration_t *cpus;
    uint32_t num_gpu_cores;
    gpu_configuration_t *gpus;
} ehp_configuration_t;

typedef struct {
	uint32_t id;
    uint32_t num_cpu_cores;
    cpu_configuration_t *cpus;
    uint32_t num_gpu_cores;
    gpu_configuration_t *gpus;
    uint32_t num_dram_stacks;
    dram_configuration_t *dram;
    uint64_t ns_to_launch_to_pim;
    uint64_t ns_to_complete_from_pim;
} pim_configuration_t;

typedef struct {
    ehp_configuration_t ehp;
    uint32_t num_pim_cores;
    pim_configuration_t *pims;
} pim_emu_configuration_t;

typedef struct pim_mem_object {
    uintptr_t address_of_object;
	gpu_handle_t gpu_handle;
	uint32_t flags;
    void *mapped_pointer;
	void *allocated_pointer;
    int currently_mapped;
    pim_platform_type type;
    struct pim_mem_object *next; // Dead simple linked list
} pim_mem_object_t;


/************** Global variables shared throughout the emulator **************/



typedef  int (* MEM_OBJ_CRITERIA_F)(pim_mem_object_t *, void * search_val);

/******************** Internal Emulator Helper Functions *********************/

#ifdef WIN32
int pim_constructor ( void );
int pim_destructor ( void );
#endif

int isEmuInitialized(void);

/******************************************************************************/
/************** exported internally to pimAModel ******************************/
const pim_emu_configuration_t *getEmuArchitectureConfig(void );

/* const char * getEmuArchitectureConfigFile(void)
 * Returns:
 *      const char *
 *          Returns the location of the emulator cofiguration .xml file.
 *          the location has to be defined through PIMEMUENV environment variable.
 */
const char * getEmuArchitectureConfigFile(void);

/* const char * getEmuTempFilesLocation(void)
 * Returns:
 *      const char *
 *          Returns the location of temprary files used for dat gatherging and event capturing.
 *          the location has to be defined through PIMEMUENV environment variable.
 */
const char * getEmuTempFilesLocation(void);


void get_value_from_dram(uint32_t number_of_pims, uint32_t *output,
        pim_device_info param_name);

void get_free_dram_space(uint32_t number_of_pims, uint64_t *output);

void get_value_from_pim(uint32_t device, uint32_t *output,
        pim_device_info param_name);

void get_value_from_pim_cpu(uint32_t device, uint32_t cpu, void *param_value,
                            size_t *param_value_size_ret,
                            pim_device_info param_name);
void get_value_from_pim_gpu(uint32_t device, uint32_t gpu, void *param_value,
	                        size_t *param_value_size_ret,
                            pim_device_info param_name);
void get_value_from_pim_dram(uint32_t device, uint32_t dram, void *param_value,
	                        size_t *param_value_size_ret,
                            pim_device_info param_name);

int insert_memobj(pim_mem_object_t *to_insert);
int remove_memobj(pim_mem_object_t *to_remove);
pim_mem_object_t *find_memobj(void *address);
pim_mem_object_t *find_memobj_bycrit(MEM_OBJ_CRITERIA_F crit_f, void *serach_val);




/******************************************************************************/
/************* emulator data gathering ****************************************/
int isCapturing(void);
int isEmuMode(void);
int isCounting( void );
int isCountingMode( void );

int pimemuSendString(FILE * file_, const char* string_);
int pimemuTimelineSendString(const char* string_);
int pimemuCPUCountersSendString(const char* string_);
int pimemuInitCPUCounterStreaming( void );
int pimemuDeinitCPUCounterStreaming(void);
int pimemuInitTimelineStreaming(void);
int pimemuDeinitTimelineStreaming(void);



#ifdef __cplusplus
}
#endif

#endif  /* __LIB_PIM_PRIVATE_H_ */
