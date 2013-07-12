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

#ifndef __PIM_TEST_CPU_HPP_
#define __PIM_TEST_CPU_HPP_

#include "pim_test.hpp"

template <typename TYPE> class CCPU_WAXPBY;
template <typename TYPE> class CCPU_DOTPTOD;
template <typename TYPE> class CPIMCPU_WAXPBY;
template <typename TYPE> class CPIMCPU_DOTPTOD;

/*****************************************************************************/

template <typename TYPE>
	void waxpby( TYPE alpha_,
		               TYPE beta_,
					   TYPE * x_,
					   TYPE * y_,
					   TYPE * w_,
					   size_t length_)
	{
		assert(w_ && x_ && y_);
		for( size_t i = 0; i < length_; i++)
		{
			w_[i] = alpha_ * x_[i] + beta_ * y_[i];
		}
	}

template <typename TYPE>
	TYPE dotprod(
					   TYPE * x_,
					   TYPE * y_,
					   size_t length_)
	{
     TYPE ret = 0;  
	 double tmp_ret = 0;

		assert(x_ && y_);
		for( size_t i = 0; i < length_; i++)
		{
			tmp_ret += (double)x_[i] * (double)y_[i];
		}
		ret = (TYPE)tmp_ret;
		return(ret);
	}


/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CCPUTest : public CPimTest<TYPE>
{
public:
	CCPUTest<TYPE> (const TestConfig *config_ = 0)
		             :  CPimTest<TYPE>(config_)
	{
	    this->platf_nm = "CPU";
		m_thread = 0;

	}

	~CCPUTest<TYPE> ()
	{
      Wait();
	}

static
void* CPUThreadFunc(void *arg)
{
CCPUTest<TYPE> * pCPUTest = (CCPUTest<TYPE> * )arg;
    pCPUTest->ThreadInternal();
	return(NULL);
}


virtual
void PIMLaunch( int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
{
 void *spawn_error = NULL;
    void *pim_args[NUM_PTHREAD_ARGUMENTS];
    size_t arg_size[NUM_PTHREAD_ARGUMENTS];
	pim_f pim_function;

	pim_args[PTHREAD_ARG_THREAD] = &m_thread; // pthread_t
	arg_size[PTHREAD_ARG_THREAD] = sizeof(pthread_t);
	pim_args[PTHREAD_ARG_ATTR] = NULL; // pthread_attr_t
	arg_size[PTHREAD_ARG_ATTR] = sizeof(pthread_attr_t);
	pim_args[PTHREAD_ARG_INPUT] = this; // void * for thread input
	arg_size[PTHREAD_ARG_INPUT] = sizeof(this);

	pim_function.func_ptr = CCPUTest:: CPUThreadFunc;

    this->WaitForEvents( num_preevents, preevent_list );

	spawn_error = pim_spawn(pim_function, pim_args, arg_size, NUM_PTHREAD_ARGUMENTS, this->pim_id, this->platform);

	this->luanchedThreadInc();

	if (spawn_error == NULL)
	{
	     PIM_TESTERROR_MSG("Faild to launch PIM thread\n");
    }

}

virtual
void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
{
    this->WaitForEvents( num_preevents, preevent_list );

	 m_thread = 0;
	 pthread_create(&m_thread,NULL, CCPUTest:: CPUThreadFunc, this);

     this->luanchedThreadInc();
}

virtual
void Wait( void )
{
   if ( m_thread )
   {
       pthread_join(m_thread, NULL);
       m_thread = 0;
       this->sunkThreadInc();
   }


//   this->CPUDeinit();
}



protected:
	pthread_t m_thread;

	virtual int ThreadInternal(void )
	{

		if ( this->m_threadfunc && this->m_call_depth > 0 )
		{
			this->m_threadfunc((void*)&this->m_test_config);
		}
		return(0);
	}
};



template <typename TYPE>
class CPIMCPUTest : public CCPUTest<TYPE>
{
public:
	CPIMCPUTest<TYPE> (const TestConfig *config_ = 0) 
					   :  CCPUTest<TYPE>(config_)
	{
	}

virtual
void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
{
     this->PIMLaunch( num_preevents, preevent_list);
}


};

/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CCPU_WAXPBY : public CCPUTest<TYPE>
{
public:
	 CCPU_WAXPBY<TYPE>(const TestConfig *config_ = 0) : CCPUTest<TYPE> (config_)
	 {
        this->alpha = (TYPE)100.1;
        this->beta = (TYPE)1.1;
		this->test_nm = "waxpby";
	 }

	 CCPU_WAXPBY<TYPE>()
	 {
        this->alpha = (TYPE)100.1;
        this->beta = (TYPE)1.1;
		this->test_nm = "waxpby";
	 }
      TYPE alpha;
      TYPE beta;

     void Init (void )
	 {
	    this->x_length =
     	this->y_length = 
		this->w_length = this->problem_sz;

		CPimTest<TYPE> :: SysMemInit();

        fillWithRand<TYPE>(this->x,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
        fillWithRand<TYPE>(this->y,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
	 }

	 void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
	 {
     int loops = this->m_test_config.tests[this->m_test_id].loops;
        for ( int l = 0; l < loops; l++ )
	    {
			  if ( l == 0 )
			  {
		          CCPUTest<TYPE>::Launch(num_preevents,preevent_list);
			  }
			  else
			  {
		          CCPUTest<TYPE>::Launch(0,0);
			  }
	   	      if ( l < loops - 1 )
		      {
                  this->Wait();
		      }
     	}
	 }


protected:
	virtual int ThreadInternal(void )
	{

		 this->startMsg();

		 waxpby<TYPE>( this->alpha,
		               this->beta,
					   this->x,
					   this->y,
					   this->w,
					   this->problem_sz);
		 return(0);
	}
};

/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CCPU_DOTPTOD : public CCPUTest<TYPE>
{
public:
	CCPU_DOTPTOD<TYPE>(const TestConfig *config_ = 0) : CCPUTest<TYPE> (config_)
	{
		this->test_nm = "dotprod";
	}


     void Init (void )
	 {
	    this->x_length =
     	this->y_length = this->problem_sz;

		CPimTest<TYPE> :: SysMemInit();

        fillWithRand<TYPE>(this->x,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
        fillWithRand<TYPE>(this->y,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
	 }

	 void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0) 
	 {
     int loops = this->m_test_config.tests[this->m_test_id].loops;
        for ( int l = 0; l < loops; l++ )
	    {
			  if ( l == 0 )
			  {
		          CCPUTest<TYPE>::Launch(num_preevents,preevent_list);
			  }
			  else
			  {
		          CCPUTest<TYPE>::Launch(0,0);
			  }
	   	      if ( l < loops - 1 )
		      {
                  this->Wait();
		      }
     	}
	 }

protected:

	virtual int ThreadInternal(void )
	{
		this->startMsg();
		dotprod<TYPE>(this->x, this->y, this->problem_sz);
		return(0);
	}
};


/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CPIMCPU_WAXPBY : public CCPU_WAXPBY<TYPE>
{
public:
	 CPIMCPU_WAXPBY<TYPE>(const TestConfig *config_ = 0) : 
	               CCPU_WAXPBY<TYPE> (config_)
	 {
        this->alpha = (TYPE)100.1;
        this->beta = (TYPE)1.1;
		this->test_nm = "waxpby";
	 }

	 CPIMCPU_WAXPBY<TYPE>()
	 {
        this->alpha = (TYPE)100.1;
        this->beta = (TYPE)1.1;
		this->test_nm = "waxpby";
	 }
 
	 


	 void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
	 {
     int loops = (this->m_test_config ) ? this->m_test_config->tests[this->m_test_config->test_id].loops : 1;
        for ( int l = 0; l < loops; l++ )
	    {
			  if ( l == 0 )
			  {
		           this->PIMLaunch(num_preevents, preevent_list);
			  }
			  else
			  {
		           this->PIMLaunch(0, 0);
			  }
	   	      if ( l < loops - 1 )
		      {
                  this->Wait();
		      }
     	}
	 }



     int Init (uint32_t mem_flags_ = PIM_MEM_DEFAULT_FLAGS )
	 {
	 int ret = 0;
	    this->x_length =
     	this->y_length = 
		this->w_length = this->problem_sz;

		ret = CPimTest<TYPE> :: PIMMemInit(mem_flags_);

     TYPE * px = (TYPE * )pim_map(this->x, this->platform);
     TYPE * py = (TYPE * )pim_map(this->y, this-> platform);

        fillWithRand<TYPE>(px,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
        fillWithRand<TYPE>(py,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);

	    pim_unmap(px);
	    pim_unmap(py);

		return(ret);
	 }


     TYPE alpha;
     TYPE beta;

protected:
	virtual int ThreadInternal(void )
	{
		 this->startMsg();

		 waxpby<TYPE>( this->alpha,
		               this->beta,
					   this->x,
					   this->y,
					   this->w,
					   this->problem_sz);
		 return(0);
	}
};


template <typename TYPE>
class CPIMCPU_DOTPTOD : public CCPU_DOTPTOD<TYPE>
{
public:
	CPIMCPU_DOTPTOD<TYPE>(const TestConfig *config_ = 0) : CCPU_DOTPTOD<TYPE> (config_)
	{

		this->test_nm = "dotprod";
	}

	CPIMCPU_DOTPTOD<TYPE>()
	{
		this->test_nm = "dotprod";
	}

     int Init (uint32_t mem_flags_ = PIM_MEM_DEFAULT_FLAGS )
	 {
	 int ret = 0;
	    this->x_length =
     	this->y_length = this->problem_sz;


		ret = CPimTest<TYPE> :: PIMMemInit(mem_flags_);

     TYPE * px = (TYPE * )pim_map(this->x, this->platform);
     TYPE * py = (TYPE * )pim_map(this->y, this-> platform);

        fillWithRand<TYPE>(px,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
        fillWithRand<TYPE>(py,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);

	    pim_unmap(px);
	    pim_unmap(py);

		return(ret);
	 }


	 void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0) 
	 {
     int loops = (this->m_test_config ) ? this->m_test_config->tests[this->m_test_config->test_id].loops : 1;
        for ( int l = 0; l < loops; l++ )
	    {
			  if ( l == 0 )
			  {
		           this->PIMLaunch(num_preevents, preevent_list);
			  }
			  else
			  {
		           this->PIMLaunch(0, 0);
			  }
	   	      if ( l < loops - 1 )
		      {
                  this->Wait();
		      }
     	}
	 }



protected:

	virtual int ThreadInternal(void )
	{
		this->startMsg();
		dotprod<TYPE>(this->x, this->y, this->problem_sz);
		return(0);
	}
};



#endif  /* __PIM_TEST_HPP_ */
