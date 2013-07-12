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

#ifndef _PIM_SIM_GPU_H_
#define _PIM_SIM_GPU_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifndef PIM_SUCCESS
#define PIM_SUCCESS 0
#endif


typedef void * PIM_GPU_ID;
typedef void * PIM_GPU_MEM;
typedef void * gpu_handle_t;
typedef void * gpu_mem_handle_t;



/***********************************************************************
  returns instantiated and intialized PIM GPU device handle. 
  
***********************************************************************/
int 
pim_get_gpu_id(	PIM_GPU_ID* device );


/***********************************************************************

Arguments:

  PIM_GPU_ID gpu_dev - PIM gpu device handle.
  size_t size        - size of teh buffer.
TO DO :: pass system pointer
  cl_mem_flags flags - CL memory flags.

  returns instantiated and intialized PIM GPU memory handle. 
  
***********************************************************************/

PIM_GPU_MEM pim_gpu_mem_alloc(PIM_GPU_ID gpu_dev, size_t size, unsigned int flags);

/***********************************************************************

Arguments:

  PIM_GPU_ID gpu_dev - PIM gpu device handle.
  PIM_GPU_MEM gpu_mem_handle - PIM GPU memory handle. 

TO DO :: pass mem map flags

  returns host pointer to the GPU buffer

***********************************************************************/

void * pim_gpu_mem_map(PIM_GPU_ID gpu_dev, PIM_GPU_MEM gpu_mem_handle, unsigned int flags_);


/***********************************************************************

Arguments:

  PIM_GPU_ID gpu_dev - PIM gpu device handle.
  void * mem_mapped - host pointer to the GPU buffer. 

***********************************************************************/

int pim_gpu_mem_unmap(PIM_GPU_ID gpu_dev, PIM_GPU_MEM gpu_mem_handle_, void * mem_mapped_ );


/***********************************************************************

Arguments:

  PIM_GPU_ID gpu_dev      - PIM gpu device handle.
  void *kernel            - pointer to a 0-terminated string of the kernel name.
  const void** args       - array of pointers on the kernel arguments (see notes).
  const size_t* arg_sizes - array of argument sizes.
  size_t nargs            - number of kernel arguments.
Notes:
  the arguments are divided into 3 classes: compilitation, launch and execution arguments.

  comlilation arguments.
  args[0] - pointer on the 0-terminated string  of the source file where the kernel resides. length = arg_sizes[0].
  args[1] - pointer on the 0-termimated strings  of the CL-compiler options.
            (options are applied to all kerrnels residing in the same source file. onnce appiled to the file they stick). length = arg_sizes[1].

  launch arguments.
  args[2] - dimensions, a number of dimensions (not more than 3) a global working domain has been defined with. size = arg_sizes[2] = 4.
  args[3] - pointer on the array of values, type size_t, defining a length of the each global working domain dimension,
            defined by the previous argument. size = arg_sizes[3] = sizeof(size_t) * args[2].
  args[4] - pointer on the array of values, type size_t, defining a length of the each local working domain dimension (or work group),
            defined by the args[2]. size = arg_sizes[4] = sizeof(size_t) * args[2].

  execution arguments.
   starting from args[5]/nargs[5] it's an array of nargs-5 pointers and sizes of the kernal execution arguments, staring from 

***********************************************************************/

int pim_gpu_spawn_inparallel_sync(PIM_GPU_ID gpu_dev,
	                              void *kernel,
                                  const void** args, 
                                  const size_t* arg_sizes, 
                                  size_t nargs);

/***********************************************************************

Arguments:

  PIM_GPU_ID gpu_dev - PIM gpu device handle.
  PIM_GPU_MEM gpu_mem_handle - PIM GPU memory handle. 

**********************************************************************/
int pim_gpu_mem_free(PIM_GPU_ID gpu_dev, PIM_GPU_MEM gpu_mem_handle);
                

/***********************************************************************

Arguments:

  PIM_GPU_ID gpu_dev - PIM gpu device handle.
**********************************************************************/
int pim_gpu_relase(PIM_GPU_ID gpu_dev);

/*********************************************************************/
/***************** PIM Factory **************************************/
int buildPIMFactory(void);

int removePIMFactory(void);

/******************** global event tracking interface ****************/
// tracks only "our" events
void addPimClEvent( cl_event event_ );
int removePimClEvent( cl_event event_ );
int isPimClEvent( cl_event event_ );

// tracks "implicit" events
cl_event* addPimImplicitClEvent(cl_command_queue q_, cl_event event_ );
cl_event removePimImplicitClEvent(cl_command_queue q_ );
cl_event *getPimImplicitClEvent(cl_command_queue q_ );
void clearPimImplicitClEvent( void );
void getPimImplicitClEvenArray(int *array_sz_, cl_event ** array_);


// track our ques
void addPimClQue( cl_command_queue que_ );
void removePimClQue( cl_command_queue que_ );
int getPimClQues( cl_command_queue **que_);

// stacked messages
void stackTimeLineMsg( const char * msg);
void sendStackedTimeLineMsg( FILE * msg_file);


#ifdef __cplusplus
}
#endif

#endif // _PIM_SIM_GPU_H_
