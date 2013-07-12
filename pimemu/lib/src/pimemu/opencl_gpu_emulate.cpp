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

#ifdef WIN32 
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include "pimg.hpp"
#include "pim.h"
#include "opencl_gpu_emulate.h"
#include "psg_shared.h"

int 
pim_get_gpu_id(	PIM_GPU_ID* device_ )
{
int ret = -1;
   
   if ( device_ )
   {
   CPIM_GPU * temp = gpuFactory();

        *device_ = 0;

        if ( temp ) 
        {
             temp->pimgInit();
			 *device_ = (PIM_GPU_ID)temp;
			 ret = PIM_SUCCESS;
	    }
   }

   return(ret);
}

/*

#define PIM_MEM_PIM_READ                            (1<<0)
#define PIM_MEM_PIM_WRITE                           (1<<1)
#define PIM_MEM_PIM_RW                              (PIM_MEM_PIM_READ|PIM_MEM_PIM_WRITE)
#define PIM_MEM_HOST_READ                           (1<<2)
#define PIM_MEM_HOST_WRITE                          (1<<3)
#define PIM_MEM_HOST_RW                             (PIM_MEM_HOST_READ|PIM_MEM_HOST_WRITE)
#define PIM_MEM_HOST_NOACCESS                       (1<<4)
#define PIM_MEM_PEER_READ                           (1<<5)
#define PIM_MEM_PEER_WRITE                          (1<<6)
#define PIM_MEM_PEER_RW                             (PIM_MEM_PEER_READ|PIM_MEM_PEER_WRITE)
#define PIM_MEM_PEER_NOACCESS                       (1<<7)
#define PIM_MEM_DEFAULT_FLAGS                       (PIM_MEM_PIM_RW|PIM_MEM_HOST_RW|PIM_MEM_PEER_RW)



#define CL_MEM_READ_WRITE                           (1 << 0)
#define CL_MEM_WRITE_ONLY                           (1 << 1)
#define CL_MEM_READ_ONLY                            (1 << 2)
#define CL_MEM_USE_HOST_PTR                         (1 << 3)
#define CL_MEM_ALLOC_HOST_PTR                       (1 << 4)
#define CL_MEM_COPY_HOST_PTR                        (1 << 5)
// reserved                                         (1 << 6)    
#define CL_MEM_HOST_WRITE_ONLY                      (1 << 7)
#define CL_MEM_HOST_READ_ONLY                       (1 << 8)
#define CL_MEM_HOST_NO_ACCESS                       (1 << 9)


 cl_map_flags - bitfield

#define CL_MAP_READ                                 (1 << 0)
#define CL_MAP_WRITE                                (1 << 1)
#define CL_MAP_WRITE_INVALIDATE_REGION              (1 << 2)


*/

PIM_GPU_MEM pim_gpu_mem_alloc(PIM_GPU_ID gpu_dev_, size_t size_, unsigned int flags_)
{
PIM_GPU_MEM ret = NULL;
CPIM_GPU * temp = (CPIM_GPU * )gpu_dev_;
cl_mem_flags mem_flags = 0;
    switch ( flags_ & 0x3)
	{
    case PIM_MEM_PIM_READ:
	     mem_flags |= CL_MEM_READ_ONLY;
		 break;
	case  PIM_MEM_PIM_WRITE:
	     mem_flags |= CL_MEM_WRITE_ONLY;
	     break;
	case  PIM_MEM_PIM_RW:
	default:
	      mem_flags |= CL_MEM_READ_WRITE;
		  break;
	}

cl::Buffer *tbuf = temp->allocBuffer(size_, NULL, mem_flags);
    ret = (PIM_GPU_MEM)tbuf;
    return(ret);
}

void * pim_gpu_mem_map(PIM_GPU_ID gpu_dev_, PIM_GPU_MEM gpu_mem_handle_, unsigned int flags_)
{
void * ret = NULL;
CPIM_GPU * temp = (CPIM_GPU * )gpu_dev_;
cl_map_flags map_flags = 0;
    switch ( flags_ & 0xc)
	{
    case PIM_MEM_HOST_READ:
	     map_flags |= CL_MAP_READ;
		 break;
	case PIM_MEM_HOST_WRITE:
	     map_flags |= CL_MAP_WRITE_INVALIDATE_REGION;
	     break;
	case  PIM_MEM_HOST_RW:
	default:
	      map_flags |= CL_MAP_READ | CL_MAP_WRITE;
		  break;
	}

   ret = temp->mapBuffer((cl::Buffer *)gpu_mem_handle_, map_flags);
   return(ret);
}

int pim_gpu_mem_unmap(PIM_GPU_ID gpu_dev_, PIM_GPU_MEM gpu_mem_handle_, void * mem_mapped_ )
{
int ret = PIM_SUCCESS;
CPIM_GPU * temp = (CPIM_GPU * )gpu_dev_;
 //temp->unmapBuffer((cl::Buffer*)gpu_mem_handle_, mem_mapped_);
 ret =    temp->unmapBuffer((cl::Buffer*)gpu_mem_handle_, mem_mapped_);
return(ret);
}

int pim_gpu_mem_free(PIM_GPU_ID gpu_dev_, PIM_GPU_MEM gpu_mem_handle_)
{
int ret = PIM_SUCCESS;
CPIM_GPU * temp = (CPIM_GPU * )gpu_dev_;
   temp->releaseBuffer((cl::Buffer *)gpu_mem_handle_);
   return(ret);
}
                
int pim_gpu_relase(PIM_GPU_ID gpu_dev_)
{
int ret = PIM_SUCCESS;
CPIM_GPU * temp = (CPIM_GPU * )gpu_dev_;
   delete temp;
   return(ret);
}


int pim_gpu_spawn_inparallel_sync(PIM_GPU_ID gpu_dev_,
	                              void *kernel_,
                                  const void** args_, 
                                  const size_t* arg_sizes_, 
                                  size_t nargs_)
{
int ret = PIM_SUCCESS;


CPIM_GPU * temp = (CPIM_GPU * )gpu_dev_;


   ret = temp->launchKernelAsync((const char*)kernel_,
                            args_, 
                            arg_sizes_, 
                            nargs_);
   return(ret);
}



/*********************************************************************/
/******************** global event tracking interface ****************/


// tracks only "our" events
void addPimClEvent( cl_event event_ )
{
	getHelpFactory()->addPimClEvent(event_);
}

int removePimClEvent( cl_event event_ )
{
	return(getHelpFactory()->removePimClEvent(event_));
}

int isPimClEvent( cl_event event_ )
{
	return(getHelpFactory()->isPimClEvent(event_));
}

// tracks "implicit" events

cl_event * addPimImplicitClEvent(cl_command_queue q_, cl_event event_ )
{
	return(getHelpFactory()->addPimImplicitClEvent(q_, event_));
}

cl_event removePimImplicitClEvent(cl_command_queue q_ )
{
	return(getHelpFactory()->removePimImplicitClEvent(q_));
}

void clearPimImplicitClEvent( void )
{
	getHelpFactory()->clearPimImplicitClEvent();
}

void getPimImplicitClEvenArray(int *array_sz_, cl_event ** array_)
{
	return(getHelpFactory()->getPimImplicitClEvenArray(array_sz_, array_));
}

cl_event *getPimImplicitClEvent(cl_command_queue q_)
{
	return(getHelpFactory()->getPimImplicitClEvent(q_));
}

// track our ques

void addPimClQue( cl_command_queue que_ )
{
	getHelpFactory()->addPimClQue(que_ );
}

void removePimClQue( cl_command_queue que_ )
{
	getHelpFactory()->removePimClQue( que_ );
}

int getPimClQues( cl_command_queue **que_)
{
	return(getHelpFactory()->getPimClQues(que_));
}


// stacked messages
void stackTimeLineMsg( const char * msg)
{
	getHelpFactory()->stackTimeLineMsg(msg);
}

void sendStackedTimeLineMsg( FILE * msg_file)
{
	getHelpFactory()->sendStackedTimeLineMsg(msg_file);
}

