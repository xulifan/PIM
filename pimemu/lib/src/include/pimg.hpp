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

#ifndef _PIMG_HPP_
#define _PIMG_HPP_

#ifdef _WIN32
#define _SCL_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <time.h>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <functional>

using namespace std;

#if __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
//#include <GLUT/glut.h>
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>


typedef unsigned char BYTE;

#else

#define _VARIADIC_MAX 10
#include <CL/cl.hpp>

#endif

#include "pim.h"
#include "pim-private.h"
#include "PIMEMU.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif




template<typename T> struct TypeTraits {};

template<>
struct TypeTraits<float> {


  static const char* name() {return "float";}
  static const char* print_format() {return "%f";}

};

template<>
struct TypeTraits<double> {

  static const char* name() {return "double";}
  static const char* print_format() {return "%f";}
};

template<>
struct TypeTraits<int> {

  static const char* name() {return "int";}
  static const char* print_format() {return "%d";}
};

template<>
struct TypeTraits<long int> {

  static const char* name() {return "long int";}
  static const char* print_format() {return "%ld";}

};

typedef struct _pim_gpuarg
{
	void * arg_ptr;
	size_t arg_sz;
} pim_gpuarg;

class helpFactory;

class CPIM_GPU {
public:




	CPIM_GPU( void);
	virtual ~CPIM_GPU( void);

	int pimgInit( void );

 
    cl_kernel compileKernel(const char *kernelname_,
	                                const char *filename_ = NULL,
									const char *options_ = NULL);

    cl::Buffer * allocBuffer(
                            ::size_t size_,
                            void* host_ptr_ = NULL,
							cl_mem_flags flags_ = 0
						    );
    cl_int releaseBuffer( cl::Buffer * buffer_);


    cl_int launchKernelAsync(const char *kernelname_,
	                    const void** args_, 
                        const size_t* arg_sizes_, 
                        size_t nargs_);

	cl_int launchKernel2(const char *kernelname_,
	                   const char *filename_,
					   const char *options_,
					   int dim_,
					   size_t global_dim_[3],
					   size_t local_dim_[3],
					   VECTOR_CLASS<pim_gpuarg> & args_);

    void * mapBuffer(cl::Buffer * buffer_, cl_map_flags map_flags_ = (CL_MAP_READ | CL_MAP_WRITE));

    cl_int unmapBuffer( cl::Buffer * buffer_, void * mapped_ptr_);


	inline void setPimGpuFactory( helpFactory *  PIMGPUFactory_)
	{
		m_PIMGPUFactory = PIMGPUFactory_;
	}

	inline helpFactory *getPimGpuFactory( void )
	{
		return(m_PIMGPUFactory);
	}

	inline void setID( int index_)
	{
		m_index = index_;
	}

	inline int getID( void)
	{
		return(m_index);
	}

	cl::CommandQueue getQ( void );

protected:
	pthread_rwlock_t m_comp_rwlock;
	helpFactory *m_PIMGPUFactory;
	cl_int getDeviceList( cl_device_type type,
                          VECTOR_CLASS<cl::Device> & devices
						);
	cl::CommandQueue m_launch_q;



    int m_index;

};

#define _MAX_NUM_OF_PIMGPUS 128
class helpFactory {
public:
typedef map<cl_command_queue, cl_event > queue_to_event_map;
typedef map<string, cl::Program > program_cache_map;
typedef map<string, cl_kernel > kernel_cache_map;

   helpFactory( void);
   ~helpFactory( void);
   cl_int getDeviceList( cl_device_type type_,
                          VECTOR_CLASS<cl::Device>& devices_
						);
   CPIM_GPU * gpufactory( void);
   inline const cl::Device & getDevice( void )
   {
// TODO:
 // select device
	   return m_gpu_devices.at(m_default_dev);
   }

   inline const cl::Context & getContext( void )
   {

	   return m_ctxt;
   }

/******************************************************************************/
/***************   run-time helpers ******************************************/
   void addPimClEvent( cl_event event_ );
   int removePimClEvent( cl_event event_ );
   int isPimClEvent( cl_event event_ );
 
   cl_event * addPimImplicitClEvent(cl_command_queue q_, cl_event event_ );
   cl_event removePimImplicitClEvent(cl_command_queue q_);
   cl_event *getPimImplicitClEvent(cl_command_queue q_);
   void clearPimImplicitClEvent( void );
   void getPimImplicitClEvenArray(int *array_sz_, cl_event ** array_);

   void addPimClQue( cl_command_queue que_ );
   void removePimClQue( cl_command_queue que_ );
   int getPimClQues( cl_command_queue **que_);


   void stackTimeLineMsg( const char * msg);
   void popTimeLineMsg(  FILE * msg_file);
   void sendStackedTimeLineMsg( FILE * msg_file);
   cl::CommandQueue getSingleQ( void );
   cl_kernel compileKernel(const char *kernel_, const char *filename_, const char *options_);

protected:
   CPIM_GPU * sPIMGPUs;
   int active_counter;
	VECTOR_CLASS<cl::Device> m_gpu_devices;
	cl::Context m_ctxt;
	int m_default_dev;
    pthread_rwlock_t m_event_rwlock;
    VECTOR_CLASS<cl_event> m_global_outsranding_events;
	VECTOR_CLASS<cl_event> :: iterator findPimClEvent(cl_event event_);
	queue_to_event_map m_implicit_events;
	VECTOR_CLASS<cl_command_queue> m_myqueues; 
// FIXME!!!
// stack of messages.
	VECTOR_CLASS<string> m_event_message_q; 

	cl::CommandQueue m_single_launch_q;

	program_cache_map m_prog_map;
	kernel_cache_map m_kern_map;



};


CPIM_GPU * gpuFactory( void);
helpFactory * getHelpFactory( void );

#endif // PIMG_HPP
