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

#ifndef __PIM_TEST_HPP_
#define __PIM_TEST_HPP_

#include <stdlib.h>
#include <errno.h>
#define __STDC_FORMAT_MACROS


#ifdef WIN32
#include <stdint.h>
#include "pim_win.h"
#include "wpthread.h"
#else
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <inttypes.h>
#endif


#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <streambuf>
#include <list>
#include <map>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <functional>
#include <vector>  
#include <deque>
#include <stack>

#include <CL/cl.hpp>
#include "pim-private.h"

#ifdef WIN32

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#endif

#define _BASE_TYPE_ float

#ifdef _PIM_DEBUG_

#define PIM_TEST_MSG(msg)               \
{                                         \
   if (msg != NULL)                       \
   { \
      cout <<  "PIM TEST: " << msg << endl;    \
   }\
}

#else
#define PIM_TEST_MSG(msg)
#endif

#define PIM_TESTERROR_MSG(msg)            \
{                                         \
   if (msg != NULL)                       \
   { \
      cout <<  "PIM TEST ERROR: " << __func__ << " " << __LINE__ << " " << msg << endl;    \
   }\
}

#define _MAX_PLATFORMS_ 16
#define _MAX_DEVICES_   128
#define _DEFAULT_KERNEL_LOC_ "../../test/"
#define _DEFAULT_KERNEL_FILE_ "pim_test.cl"

using namespace std;

char *concat_directory_and_name(const char *directory, const char *name);

char* fix_directory(char *input);

char* find_directory(char *input);

const char *getOpenCLErrorString(cl_int err);

typedef void * (*testThreadFunc)(void *);

typedef struct {
   uint32_t num_pims;
   pim_device_id* list_of_pims;
   int host_cpu_cores;
   int32_t host_gpu_CUs;
   int32_t * pim_cpu_cores;
   int32_t * pim_gpu_cores;
   int32_t * pim_gpu_num_cores;
} ExSclConfig;

typedef struct {
   size_t problem_sz;
   int loops;
   int depth;
   int num_tests;
   testThreadFunc test_seqs;
   const char * test_nm;
   const char * kernel_loc;
   const char * kernel_file;
   const char * kernel_opions;
} RunableTestConfig;


typedef struct {
   int pim_id;
   int test_id;
   int num_tests;
   RunableTestConfig *tests;
   ExSclConfig modelConfig;
} TestConfig;


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

/****************************************************************************************/
// Fill a host vector with random values

template <typename TYPE> 
int fillWithRand(
         TYPE * buffer_, 
         size_t width_,
         size_t height_,
         const TYPE rangeMin_,
         const TYPE rangeMax_,
         unsigned int seed_ = 0)
{

    // set seed
    if(!seed_)
        seed_ = (unsigned int)time(NULL);

    srand(seed_);

    // set the range
    double range = double(rangeMax_ - rangeMin_) + 1.0; 

    /* random initialisation of input */
    for(size_t i = 0; i < height_; i++)
        for(size_t j = 0; j < width_; j++)
        {
         size_t index = i*width_ + j;
            buffer_[index] = rangeMin_ + (TYPE)(range*rand()/(RAND_MAX + 1.0)); 
        }

    return CL_SUCCESS;
}


template <typename TYPE> 
int fillWithRand(
         vector<TYPE> &vec_, 
         size_t width_,
         size_t height_,
         const TYPE rangeMin_,
         const TYPE rangeMax_,
         unsigned int seed_ = 0)
{

int ret = 0;

    if(vec_.empty())
    {
        cout << "Cannot fill vector." << std::endl;
        return EXIT_FAILURE;
    }

	ret = fillWithRand(
                  &vec_[0], 
                  width_,
                  height_,
                  rangeMin_,
                  rangeMax_,
                  seed_);

    return (ret);
}

// Fill a host-buffer with zero values

template <typename TYPE>
    void fillWithZero(TYPE * buffer_, ::size_t len_)
	{
	    memset(buffer_, 0, sizeof(TYPE)*len_); 
	}

// Fill a vector with zero values
template <typename TYPE>
    void fillWithZero(std::vector<TYPE> & vector_, ::size_t len_)
	{
    size_t actual_lel = (len_ < vector_.size()) ? len_ : vector_.size(); 
	    memset(&vector_[0], 0, sizeof(TYPE)*actual_lel); 
	}


/************************************************************************************/
/************************************************************************************/
template <typename TYPE> class CPimTest;

template<typename TYPE>
class CPimTest
{
public:
CPimTest(const TestConfig *config_ = 0)
{
// init ocl lock 
 //   pthread_rwlock_init(&m_ocl_rwlock, NULL);
	m_test_config = *config_;
// SIDE EFFECT !!!!

    m_test_id = m_test_config.test_id;

	problem_sz = m_test_config.tests[m_test_id].problem_sz;
    m_call_depth = m_test_config.tests[m_test_id].depth;
	m_threadfunc = m_test_config.tests[m_test_id].test_seqs;
	pim_id = m_test_config.pim_id;


	 size_t loc_len = (!m_test_config.tests[m_test_id].kernel_loc) ? strlen(getEmuTempFilesLocation())
		    : strlen(m_test_config.tests[m_test_id].kernel_loc);
	    m_kernel_loc = (char*)malloc(loc_len+1);
		assert(m_kernel_loc);
		strcpy(m_kernel_loc, (!m_test_config.tests[m_test_id].kernel_loc) ? getEmuTempFilesLocation()
			: m_test_config.tests[m_test_id].kernel_loc);

	 size_t file_len = (!m_test_config.tests[m_test_id].kernel_file) ? strlen(_DEFAULT_KERNEL_FILE_)
		    : strlen(m_test_config.tests[m_test_id].kernel_file);
	    m_kernel_file = (char*)malloc(file_len+1);
		assert(m_kernel_file);
		strcpy(m_kernel_file, (!m_test_config.tests[m_test_id].kernel_file) ? _DEFAULT_KERNEL_FILE_
			: m_test_config.tests[m_test_id].kernel_file);

	 size_t opt_len = (!m_test_config.tests[m_test_id].kernel_opions) ? 32 :
		  strlen(m_test_config.tests[m_test_id].kernel_opions) + 32;
	    m_kernel_opt = (char*)malloc(opt_len+1);
		assert(m_kernel_opt);
		m_kernel_opt[0] = 0;
     char def_options[33];
	 TypeTraits<TYPE> f;
	    sprintf(def_options, " -D BASE_TYPE=%s ",f.name()); 
		if ( m_test_config.tests[m_test_id].kernel_opions) 
		{
			strcpy(m_kernel_opt, m_test_config.tests[m_test_id].kernel_opions );
		}
		strcat(m_kernel_opt, def_options);

	x = 0;
	y = 0;
	z = 0;
	w = 0;

	cl_x = 
	cl_y =
	cl_z = 
	cl_w = 0;

	 x_sysptr =
	 y_sysptr =
	 z_sysptr =
	 w_sysptr =
	 x_mapptr =
	 y_mapptr =
	 z_mapptr =
	 w_mapptr = 0;


	x_length =
	y_length =
	z_length =
	w_length = 0;
	platform = PIM_PLATFORM_PTHREAD_CPU;
	pim_x =
	pim_y =
	pim_z =
	pim_w = 0;

	m_postevent = 0;


}


virtual
~CPimTest()
{
    
	CPUDeinit();

    GPUDeinit();

	   if (m_kernel_opt)
	   {
		   free(m_kernel_opt);
		   m_kernel_opt = 0;
	   }

	   if ( m_kernel_file )
	   {
		   free(m_kernel_file);
		   m_kernel_file = 0;
	   }

	   if ( m_kernel_loc )
	   {
		   free(m_kernel_loc);
		   m_kernel_loc = 0;
	   }


}

void CPUDeinit( )
{
	if ( x  && x_length > 0 )
	{
		if (pim_x)
		{
			pim_free(x);
		}
		else
		{
		    free(x);
		}
		x = 0;
	}
	if ( y  && y_length > 0 )
	{
		if (pim_y)
		{
			pim_free(y);
		}
		else
		{
		   free(y);
		}
		y = 0;
	}
	if ( z  && z_length > 0 )
	{
		if (pim_z)
		{
			pim_free(z);
		}
		else
		{
    		free(z);
		}
		z = 0;
	}
	if ( w  && w_length > 0 )
	{
		if (pim_w)
		{
			pim_free(w);
		}
		else
		{
    		free(w);
		}
		w = 0;
	}

}

void GPUDeinit( void )
{
	if ( cl_x  && x_length > 0 )
	{
		clReleaseMemObject(cl_x);

		cl_x = 0;
	}

	if ( cl_y  && y_length > 0 )
	{
		clReleaseMemObject(cl_y);

		cl_y = 0;
	}
	if ( cl_z  && z_length > 0 )
	{
		clReleaseMemObject(cl_z);

		cl_z = 0;
	}
	if ( cl_w  && w_length > 0 )
	{
		clReleaseMemObject(cl_w);

		cl_w = 0;
	}

}


virtual void Launch( int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0, int loops = 1, int depth = 0, void* (*testFunc) = 0 )
{

};
virtual void PIMLaunch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0, int loops = 1, int depth = 0, void* (*testFunc) = 0 )
{
	
};

void SortOutEvents( int * my_num_preevents, CPimTest<TYPE>*** my_preevent_list, int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0  )
{
uint32_t id = pim_get_id();
int meEHP = ((uint32_t)PIM_HOST_ID == id);
int meCPU = !strcmp(this->platf_nm, "CPU");

    if ( my_num_preevents )
	{
		  *my_num_preevents = 0;
	}

    if ( num_preevents > 0 && preevent_list && my_num_preevents  && my_preevent_list)
	{
		  *my_preevent_list = new CPimTest<TYPE>*[1024];
		  assert(*my_preevent_list);

          for(int i = 0; i < num_preevents; i++)
	      {

		   int otherEHP = ((uint32_t)PIM_HOST_ID == preevent_list[i]->pim_id);
		   int otherCPU = !strcmp(preevent_list[i]->platf_nm, "CPU");

			   if ( meCPU || otherCPU ||
				    (!meCPU && !otherCPU && ((meEHP && !otherEHP) || (!meEHP && otherEHP)))
			      )
			   { 
        // the CPU or GPU but from diff context

				    preevent_list[i]->Wait();
		  	   }
			   else
		       {
				    (*my_preevent_list)[(*my_num_preevents)] = preevent_list[i];
					(*my_num_preevents)++;
		       }

		      
	     }
	}
}

virtual void Init( void )
{

}

virtual void Wait( void )
{

}

virtual void WaitForEvents( int num_preevents, CPimTest<TYPE> ** prevtests = 0)
{

	if ( num_preevents > 0 && prevtests)
	{
		for( int i = 0; i < num_preevents; i++)
		{
			prevtests[i]->Wait();
		}

	}
}

inline
cl_event & getPostEvent( void )
{  
    return(m_postevent);
}


void luanchedThreadInc(void)
{
#ifdef WIN32
   InterlockedAdd64(&s_threads_launched_sofar, 1);
#else
// critical section
    pthread_rwlock_wrlock(&m_ocl_rwlock);
	s_threads_launched_sofar++;
	pthread_rwlock_unlock(&m_ocl_rwlock);
#endif
}

static
long long luanchedThreadsTotal(void)
{
   return(s_threads_launched_sofar);
}


void sunkThreadInc(void)
{
#ifdef WIN32
   InterlockedAdd64(&s_threads_sunk_sofar, 1);
#else
    pthread_rwlock_wrlock(&m_ocl_rwlock);
	s_threads_sunk_sofar++;
	pthread_rwlock_unlock(&m_ocl_rwlock);
#endif
}

static
long long sunkThreadsTotal(void)
{
   return(s_threads_sunk_sofar);
}




public:

	const char* test_nm;
	const char* platf_nm;
    int pim_id;
	uint32_t platform;

	 cl_event m_postevent;
	 char *m_kernel_loc;
	 char *m_kernel_file;
	 char *m_kernel_opt;

	static pthread_rwlock_t m_ocl_rwlock;
// global thread statistics
	static long long s_threads_launched_sofar;
	static long long s_threads_sunk_sofar;

    TestConfig m_test_config;
	int m_test_id;
    int m_call_depth;
	testThreadFunc m_threadfunc;
	size_t problem_sz;
	 
	  TYPE * x;
	  TYPE * y;
	  TYPE * z;
	  TYPE * w;
	  cl_mem cl_x;
	  cl_mem cl_y;
	  cl_mem cl_z;
	  cl_mem cl_w;
	  TYPE * x_sysptr;
	  TYPE * y_sysptr;
	  TYPE * z_sysptr;
	  TYPE * w_sysptr;
	  TYPE * x_mapptr;
	  TYPE * y_mapptr;
	  TYPE * z_mapptr;
	  TYPE * w_mapptr;
	  size_t x_length;
	  size_t y_length;
	  size_t z_length;
	  size_t w_length;
	
	  int pim_x;
	  int pim_y;
	  int pim_z;
	  int pim_w;




;

protected:

void startMsg( uint32_t ext_id = PIM_INVALID_ID )
{
char msg[1024];
uint32_t id =  ( ext_id == PIM_INVALID_ID ) ? pim_get_id() : ext_id;
char tmp_buf[32];
  sprintf(tmp_buf, "PIM%d", id);
  sprintf(msg, "Platform: %s, Device: %s, Kernel: %s\n", platf_nm, (id==(uint32_t)PIM_HOST_ID)?"EHP": tmp_buf, test_nm);
  PIM_TEST_MSG(msg);

}


void SysMemInit( void)
{


	if ( !x && x_length > 0 )
	{
		x = (TYPE*)malloc(x_length*sizeof(TYPE));
		assert(x);
	}
	if ( !y && y_length > 0 )
	{
		y = (TYPE*)malloc(y_length*sizeof(TYPE));
		assert(y);
	}
	if ( !z && z_length > 0 )
	{
		z = (TYPE*)malloc(z_length*sizeof(TYPE));
		assert(z);
	}
	if ( !w && w_length > 0 )
	{
		w = (TYPE*)malloc(w_length*sizeof(TYPE));
		assert(w);
	}
}


int PIMMemInit (uint32_t mem_flags_ = PIM_MEM_DEFAULT_FLAGS )
{
uint32_t mem_flags;
	if ( !x && x_length > 0 )
	{
		mem_flags = mem_flags_ |  PIM_MEM_ALIGN_CACHELINE;
		x = (TYPE*)pim_malloc(sizeof(TYPE) * x_length, pim_id, mem_flags, platform);
		assert(x);
        pim_x = 1;
	}
	if ( !y && y_length > 0 )
	{
		mem_flags = mem_flags_ |  PIM_MEM_ALIGN_WORD;
		y = (TYPE*)pim_malloc(sizeof(TYPE) * y_length, pim_id, mem_flags, platform);
		assert(y);
        pim_y = 1;
	}
	if ( !z && z_length > 0 )
	{
		mem_flags = mem_flags_;
		z = (TYPE*)pim_malloc(sizeof(TYPE) * z_length, pim_id, mem_flags, platform);
		assert(z);
        pim_z = 1;
	}
	if ( !w && w_length > 0 )
	{
		mem_flags = mem_flags_ |  PIM_MEM_ALIGN_PAGE;
		w = (TYPE*)pim_malloc(sizeof(TYPE) * w_length, pim_id, mem_flags, platform);
		assert(w);
        pim_w = 1;
	}

	return(0);
}

};


template<class T> pthread_rwlock_t  CPimTest<T> :: m_ocl_rwlock;
template<class T> long long CPimTest<T> :: s_threads_launched_sofar = 0;
template<class T> long long CPimTest<T> :: s_threads_sunk_sofar = 0;


#endif  /* __PIM_TEST_HPP_ */
