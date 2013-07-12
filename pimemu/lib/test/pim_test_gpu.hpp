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

#ifndef __PIM_TEST_EHP_GPU_HPP_
#define __PIM_TEST_EHP_GPU_HPP_

#include "pim_test.hpp"

/*****************************************************************************/
template <typename TYPE> class CGPUTest;
template <typename TYPE> class CGPU_WAXPBY;
template <typename TYPE> class CPIMGPU_WAXPBY;
template <typename TYPE> class CGPU_DOTPROD;
template <typename TYPE> class CPIMGPU_DOTPROD;




/*******************************************************************
   Helper routines
*******************************************************************/
// copy an OpenCl buffer to and from a file.
cl_int uploadBufferToHD( const char * file_nm_, const char * buf_nm_);
cl_int downloadBufferFromHD( const char * file_nm_, const char * buf_nm_, cl_mem_flags flags_ = 0);

// copy an OpenCl buffer to and from a host-based vector.

template <typename VECTOR_TYPE>
int copyFromHostToDev(cl::Buffer & dst_, const vector<VECTOR_TYPE> & src_)
{
int ret = 0;

    ret = cl::copy(src_.begin(), src_.end(), dst_ ); 
	return(ret);
}

template <typename VECTOR_TYPE>
int copyFromDevToHost(vector<VECTOR_TYPE> & dst_, cl::Buffer & src_)
{
int ret = 0;
    ret = cl::copy(src_, dst_.begin(), dst_.end()); 
	return(ret);
}


cl_int initOcl( cl_context *context_, cl_device_id *device_  );
cl_int getDevicePropoery(void *queue_, cl_device_id device_, int property_id_, int property_ln_);
cl_int createOclQueue(cl_command_queue *queue_, cl_context context_, cl_device_id device_);
cl_int compileOclKernels(cl_program *program_, cl_context context_, cl_device_id device_, const char *file_name_, const char *options_);
cl_int getKernel(cl_kernel *kernel_,cl_program program_, const char *kernel_name_);


/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CGPUTest : public CPimTest<TYPE>
{
public:
	static cl_context EHP_context;
	static cl_context PIM_context;
    static cl_program EHP_programs[1];
    static cl_program PIM_programs[1];
	static cl_device_id EHP_devices[_MAX_DEVICES_];
	static cl_device_id PIM_devices[16][_MAX_DEVICES_];

	CGPUTest<TYPE> (const TestConfig *config_ = 0) 
					:  CPimTest<TYPE>(config_)
	{
//	cl_int cl_err;
	    this->platf_nm = "GPU";
	    m_context = 0;
		m_devices = 0;

		this->platform = PIM_PLATFORM_OPENCL_GPU;

		m_queue = 0;

	}

	~CGPUTest<TYPE> ( void )
	{

		Wait();

	   if ( m_queue )
	   {
		   clReleaseCommandQueue(m_queue);
		   m_queue = 0;
	   }

#if 0
// MUTEX
	   if ( m_context && (*m_context) )
	   {
		   clReleaseContext(*m_context);
		   *m_context = 0;
	   }
#endif


	}

    void GPUMemInit(uint32_t mem_flags_= 0)
    {
	    if ( !this->cl_x  && this->x_length > 0 )
	    {
		cl_int cl_err;

			this->cl_x = clCreateBuffer(*m_context   /* context */,
                                  mem_flags_ /* flags */,
                                  this->x_length * sizeof(TYPE)      /* size */,
                                  this->x_sysptr /* host_ptr */,
                                  &cl_err /* errcode_ret */);
			ASSERT_CL_RETURN(cl_err);
	    }

	    if ( !this->cl_y  && this->y_length > 0 )
	    {
		cl_int cl_err;

			this->cl_y = clCreateBuffer(*m_context   /* context */,
                                  mem_flags_ /* flags */,
                                  this->y_length * sizeof(TYPE)      /* size */,
                                  this->y_sysptr /* host_ptr */,
                                  &cl_err /* errcode_ret */);
			ASSERT_CL_RETURN(cl_err);
	    }

	    if ( !this->cl_z  && this->z_length > 0 )
	    {
		cl_int cl_err;

			this->cl_z = clCreateBuffer(*m_context   /* context */,
                                  mem_flags_ /* flags */,
                                  this->z_length * sizeof(TYPE)      /* size */,
                                  this->z_sysptr /* host_ptr */,
                                  &cl_err /* errcode_ret */);
			ASSERT_CL_RETURN(cl_err);
	    }

	    if ( !this->cl_w  && this->w_length > 0 )
	    {
		cl_int cl_err;

			this->cl_w = clCreateBuffer(*m_context   /* context */,
                                  mem_flags_ /* flags */,
                                  this->w_length * sizeof(TYPE)      /* size */,
                                  this->w_sysptr /* host_ptr */,
                                  &cl_err /* errcode_ret */);
			ASSERT_CL_RETURN(cl_err);
	    }


   }

	void Init(uint32_t mem_flags_ = PIM_MEM_DEFAULT_FLAGS)
	{
	cl_uint cl_err = CL_SUCCESS;
       uint32_t id = pim_get_id();


		m_devices = ((uint32_t)PIM_HOST_ID == id)? EHP_devices: PIM_devices[0];
        m_context = ((uint32_t)PIM_HOST_ID == id)? &EHP_context: &PIM_context;
// critical section
	    pthread_rwlock_wrlock(&this->m_ocl_rwlock);


		if ( m_context[0] == 0 )
		{
			cl_err = initOcl( m_context, m_devices  );
		}
	      pthread_rwlock_unlock(&this->m_ocl_rwlock);
// end of critical section
// critical section

		m_program = ((uint32_t)PIM_HOST_ID == id)? EHP_programs: PIM_programs;

	    pthread_rwlock_wrlock(&this->m_ocl_rwlock);

		if ( m_program[0] == 0)
		{
        char * dir = fix_directory(this->m_kernel_loc);
	         assert(dir);
        char * prog = concat_directory_and_name(dir, this->m_kernel_file);
	         assert(prog);
		     cl_err = compileOclKernels(m_program, *m_context, m_devices[0], prog, this->m_kernel_opt);
		     free(dir);
		     free(prog);
		}
	    pthread_rwlock_unlock(&this->m_ocl_rwlock);
// end of critical section


		cl_err = createOclQueue(&m_queue, *m_context, m_devices[0]);

	}

     void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
     {
		    
		    this->luanchedThreadInc();

		 	if ( num_preevents > 0 && preevent_list )
			{
				for( int i = 0; i < num_preevents; i++)
				{

    					clReleaseEvent(preevent_list[i]->getPostEvent());
	
					    preevent_list[i]->getPostEvent() = 0;
					
				}
			}



	 }

     void PIMLaunch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
     {
		 CGPUTest<TYPE> :: Launch(num_preevents, preevent_list);
	 }

   void Wait(void)
   {   
		if ( this->m_postevent != 0 )
		{

        cl_int cl_err = clWaitForEvents(1, &this->getPostEvent());
            ASSERT_CL_RETURN(cl_err);
		    clReleaseEvent(this->m_postevent);

		    this->m_postevent = 0;

            this->sunkThreadInc();
		}
// free GPU memory

//	    this->GPUDeinit();
// free PIM memory
//		this->CPUDeinit();
   }

protected:

	cl_device_id * m_devices;

	cl_context *m_context;
	cl_program *m_program;
	cl_command_queue m_queue;



};

template <typename T>
cl_device_id CGPUTest<T> :: EHP_devices[_MAX_DEVICES_] = {0};
template <typename T>
cl_device_id CGPUTest<T> :: PIM_devices[16][_MAX_DEVICES_] = {{0}};

template <typename T>
cl_context CGPUTest<T> :: EHP_context = 0;
template <typename T>
cl_context CGPUTest<T> :: PIM_context = 0;
template <typename T>
cl_program CGPUTest<T> :: EHP_programs[1] = {0};
template <typename T>
cl_program CGPUTest<T> :: PIM_programs[1] = {0};


/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CGPU_WAXPBY : public CGPUTest<TYPE>
{
public:
	 CGPU_WAXPBY<TYPE>(const TestConfig *config_ = 0)
					   : CGPUTest<TYPE> (config_)
	 {
        this->alpha = 0;
        this->beta = 0;;
		this->test_nm = "waxpby";
	 }

     void Init (uint32_t mem_flags_ = 0)
	 {
	 cl_int cl_err = CL_SUCCESS;
	     this->x_length =
     	 this->y_length = 
		 this->w_length = this->problem_sz;
		 CGPUTest<TYPE> :: Init();
		 CGPUTest<TYPE> :: GPUMemInit(mem_flags_);

//		 do
		 {
         this->x_mapptr = (float*)clEnqueueMapBuffer(this->m_queue,
                                            this->cl_x,
                                            CL_TRUE, 
                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                            0,
                                            this->x_length * sizeof(TYPE),
                                            0,
                                            NULL,
                                            NULL,
                                            &cl_err);
		 }
//		 while(cl_err != 0 );

	     ASSERT_CL_RETURN(cl_err);

//		 do
		 {
         this->y_mapptr = (float*)clEnqueueMapBuffer(this->m_queue,
                                            this->cl_y,
                                            CL_TRUE, 
                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                            0,
                                            this->y_length * sizeof(TYPE),
                                            0,
                                            NULL,
                                            NULL,
                                            &cl_err);
		 } 
//		 while(cl_err != 0 );



	      ASSERT_CL_RETURN(cl_err);

          fillWithRand<TYPE>(this->x_mapptr,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
          fillWithRand<TYPE>(this->y_mapptr,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);


          cl_err = clEnqueueUnmapMemObject(this->m_queue,
                                          this->cl_x,
                                          this->x_mapptr,
                                          0,
                                          NULL,
                                          NULL);
	      ASSERT_CL_RETURN(cl_err);

          cl_err = clEnqueueUnmapMemObject(this->m_queue,
                                          this->cl_y,
                                          this->y_mapptr,
                                          0,
                                          NULL,
                                          NULL);
	      ASSERT_CL_RETURN(cl_err);


	 }

     void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
     {
	 cl_int cl_err = CL_SUCCESS;
	 cl_kernel kernel;
	 int nargs;
	 int len = (int)this->problem_sz;
	 int my_numevents;
	 cl_event *preevents = 0;
	 CPimTest<TYPE> ** my_preevent_list = 0;

	        this->SortOutEvents( &my_numevents, &my_preevent_list, num_preevents, preevent_list  );

	        if ( my_numevents > 0 )
			{
				preevents = new cl_event[my_numevents];
				assert(preevents);
				for( int i = 0; i < num_preevents; i++)
				{

					  preevents[i] = my_preevent_list[i]->getPostEvent();
				}
			}

	      getKernel(&kernel,this->m_program[0], this->test_nm);
		  nargs = 0;
// critical section
// setarg is not thread-safe
//	      pthread_rwlock_wrlock(&this->m_ocl_rwlock);

	      cl_err=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(int),
                        &len);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(TYPE),
                        &alpha);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(TYPE),
                        &beta);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(cl_mem),
                        &this->cl_x);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(cl_mem),
                        &this->cl_y);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(cl_mem),
                        &this->cl_w);
 
//	      pthread_rwlock_unlock(&this->m_ocl_rwlock);
// end of critical section

	      ASSERT_CL_RETURN(cl_err);

  



	 size_t ndrl[3] = {64, 1, 1};
	 size_t ndrg[3] = {((this->problem_sz + ndrl[0] - 1)/ndrl[0]) * ndrl[0], 1, 1};

     int loops = this->m_test_config.tests[this->m_test_id].loops;
         for(int i = 0; i < loops; i++)
	     {
	     int loop_num = (i == 0) ? my_numevents : 0;
		 cl_event *loop_preevents = (i == 0) ? preevents : 0;
		 cl_event * postevent = (i == loops - 1) ? &this->m_postevent : 0; 
              this->startMsg();

			   if ( i != loops - 1)
			   {
			       this->luanchedThreadInc();
			   }

	           cl_err = clEnqueueNDRangeKernel(this->m_queue,
                                          kernel,
                                          1,
                                          NULL,
                                          ndrg,
                                          ndrl,
                                          loop_num,
                                          loop_preevents,
                                          postevent);
		       ASSERT_CL_RETURN(cl_err);

			   if ( i != loops - 1)
			   {
				    this->sunkThreadInc();
			   }
		 }


		 if ( preevents )
		 {
			 delete [] preevents;
		 }

		 CGPUTest<TYPE>::Launch(my_numevents, preevent_list);
		 if (my_preevent_list)
		 {
			 delete[] my_preevent_list;
		 }

	 }

public:
     TYPE alpha;
     TYPE beta;

};


template <typename TYPE>
class CPIMGPU_WAXPBY : public CGPU_WAXPBY<TYPE>
{
public:
	 CPIMGPU_WAXPBY<TYPE>(const TestConfig *config_ = 0)
					   : CGPU_WAXPBY<TYPE>(config_)
	 {

	 }

 

     void Init (uint32_t mem_flags_ = PIM_MEM_DEFAULT_FLAGS )
	 {
	 cl_int cl_err = CL_SUCCESS;
	     this->x_length =
     	 this->y_length = 
		 this->w_length = this->problem_sz;

		 CPimTest<TYPE> :: PIMMemInit(mem_flags_);


     TYPE * px = (TYPE * )pim_map(this->x, this->platform);
     TYPE * py = (TYPE * )pim_map(this->y, this-> platform);

        fillWithRand<TYPE>(px,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
        fillWithRand<TYPE>(py,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);

	    pim_unmap(px);
	    pim_unmap(py);

	 }

     void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
     {
	 cl_int cl_err = CL_SUCCESS;
	 int len = (int)this->problem_sz;
     char* ocl_source;
     const void * args[1024];
     size_t sizes[1024];
     size_t nargs = 0;
     int dim = 1;
     size_t lcl[3] = {64,1,1};
	 size_t glbl[3] = {0,1,1}; 
	 pim_f gpu_kernel;
	 int my_numevents;
	 cl_event *preevents = 0;
	 CPimTest<TYPE> ** my_preevent_list = 0;

	        this->SortOutEvents( &my_numevents, &my_preevent_list, num_preevents, preevent_list  );

	        if ( my_numevents > 0 )
			{
				preevents = new cl_event[my_numevents];
				assert(preevents);
				for( int i = 0; i < num_preevents; i++)
				{

					  preevents[i] = my_preevent_list[i]->getPostEvent();
				}
			}

     char * dir = fix_directory(this->m_kernel_loc);
	         assert(dir);
             ocl_source = concat_directory_and_name(dir, this->m_kernel_file);
	         assert(ocl_source);

			 gpu_kernel.func_name = (void*)this->test_nm;

		     nargs = 0;


             args[nargs] =  ocl_source;
	         sizes[nargs++] = sizeof(char*);

             args[nargs] =  this->m_kernel_opt;
	         sizes[nargs++] = sizeof(char*);
// kernel launch arguments
             args[nargs] =  &dim;
	         sizes[nargs++] = sizeof(int);

	         glbl[0] = ((this->problem_sz / lcl[0]) + 1) * lcl[0];

             args[nargs] =  glbl;
	         sizes[nargs++] = sizeof(size_t) * dim;

             args[nargs] =  lcl;
	         sizes[nargs++] = sizeof(size_t) * dim;

// event arguments
// pre-events
             args[nargs] =  &my_numevents;
	         sizes[nargs++] = sizeof(int);

// pre-events
             args[nargs] =  preevents;
	         sizes[nargs++] = sizeof(cl_event*);

// post-events
             args[nargs] =  &this->m_postevent;
	         sizes[nargs++] = sizeof(cl_event*);

// kernel arguments
             args[nargs] = &len;
	         sizes[nargs++] = sizeof(int);

             args[nargs] = &this->alpha;
	         sizes[nargs++] = sizeof(TYPE);

             args[nargs] = &this->beta;
	         sizes[nargs++] = sizeof(TYPE);

			args[nargs] = this->x;
	        sizes[nargs++] = sizeof(void*);
			args[nargs] = this->y;
	        sizes[nargs++] = sizeof(void*);
			args[nargs] = this->w;
	        sizes[nargs++] = sizeof(void*);

        int loops = this->m_test_config.tests[this->m_test_id].loops;
           for(int i = 0; i < loops; i++)
	       {
	       int loop_num = (i == 0) ? my_numevents : 0;
// we cannot yet get pim ID frominside GPU
// sending it beforehand
                this->startMsg(this->pim_id);

// event arguments
// pre-events
                args[OPENCL_ARG_NUMPREEVENTS] =  &loop_num;
// pre-events
                args[OPENCL_ARG_PREEVENTS] =  (i == 0) ? preevents : 0;

		 // post-events
               args[OPENCL_ARG_POSTEVENT] =  (i == loops - 1) ? &this->m_postevent : 0;

			   if ( i != loops - 1)
			   {
			       this->luanchedThreadInc();
			   }

         void *spawn_error = NULL;
	           spawn_error = pim_spawn(gpu_kernel, (void**)args, sizes, nargs, this->pim_id, this->platform);
	           if (spawn_error == NULL)
	           {
	                PIM_TESTERROR_MSG("Faild to launch PIM thread\n");
		       }

			   if ( i != loops - 1)
			   {
				    this->sunkThreadInc();
			   }

	     }
		 if ( preevents )
		 {
			 delete [] preevents;
		 }
		 
		 CGPUTest<TYPE>::PIMLaunch(my_numevents, preevent_list);
		 if (my_preevent_list)
		 {
			 delete[] my_preevent_list;
		 }
	 }

public:


};


/****************************************************************************/
/****************************************************************************/

template <typename TYPE>
class CGPU_DOTPROD : public CGPUTest<TYPE>
{
public:
	CGPU_DOTPROD<TYPE>(const TestConfig *config_ = 0)
					   : CGPUTest<TYPE> (config_)
	{
		this->test_nm = "dotprod";
	}


     void Init (uint32_t mem_flags_ = 0 )
	 {
	 cl_int cl_err = CL_SUCCESS;
	    this->x_length =
     	this->y_length = this->problem_sz;


		CGPUTest<TYPE> :: Init(mem_flags_);

    int nmb_CUs;
	    getDevicePropoery(&nmb_CUs, this->m_devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, 4);
		this->z_length = nmb_CUs * 16 * 64;

		CGPUTest<TYPE> :: GPUMemInit(mem_flags_);


//		 do 
		 {
         this->x_mapptr = (float*)clEnqueueMapBuffer(this->m_queue,
                                            this->cl_x,
                                            CL_TRUE, 
                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                            0,
                                            this->x_length * sizeof(TYPE),
                                            0,
                                            NULL,
                                            NULL,
                                            &cl_err);
		 }
//		 while(cl_err != -0 );

	     ASSERT_CL_RETURN(cl_err);

//		 do
		 {
         this->y_mapptr = (float*)clEnqueueMapBuffer(this->m_queue,
                                            this->cl_y,
                                            CL_TRUE, 
                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                            0,
                                            this->y_length * sizeof(TYPE),
                                            0,
                                            NULL,
                                            NULL,
                                            &cl_err);
		 }
//		 while( cl_err != 0 );


	      ASSERT_CL_RETURN(cl_err);

          fillWithRand<TYPE>(this->x_mapptr,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
          fillWithRand<TYPE>(this->y_mapptr,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);


          cl_err = clEnqueueUnmapMemObject(this->m_queue,
                                          this->cl_x,
                                          this->x_mapptr,
                                          0,
                                          NULL,
                                          NULL);
	      ASSERT_CL_RETURN(cl_err);

          cl_err = clEnqueueUnmapMemObject(this->m_queue,
                                          this->cl_y,
                                          this->y_mapptr,
                                          0,
                                          NULL,
                                          NULL);
	      ASSERT_CL_RETURN(cl_err);


	 }

     void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
     {
	 cl_int cl_err = CL_SUCCESS;
	 cl_kernel kernel;
	 int nargs;
	 int my_numevents;
	 cl_event *preevents = 0;
	 CPimTest<TYPE> ** my_preevent_list = 0;

	        this->SortOutEvents( &my_numevents, &my_preevent_list, num_preevents, preevent_list  );

	        if ( my_numevents > 0 )
			{
				preevents = new cl_event[my_numevents];
				assert(preevents);
				for( int i = 0; i < num_preevents; i++)
				{

					  preevents[i] = my_preevent_list[i]->getPostEvent();
				}
			}

	      getKernel(&kernel,this->m_program[0], this->test_nm);
		  nargs = 0;

// critical section
// setarg is not thread-safe
//	      pthread_rwlock_wrlock(&this->m_ocl_rwlock);

	      cl_err=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(int),
                        &this->problem_sz);
	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(cl_mem),
                        &this->cl_x);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(cl_mem),
                        &this->cl_y);

	      cl_err|=clSetKernelArg(kernel,
                        nargs++,
                        sizeof(cl_mem),
                        &this->cl_z);

//	      pthread_rwlock_unlock(&this->m_ocl_rwlock);
// end of critical section

          ASSERT_CL_RETURN(cl_err);


	 size_t ndrl[3] = {64, 1, 1};
	 size_t ndrg[3] = {((this->z_length + ndrl[0] - 1)/ndrl[0]) * ndrl[0], 1, 1};
     int loops = this->m_test_config.tests[this->m_test_id].loops;

         for(int i = 0; i < loops; i++)
	     {
	     int loop_num = (i == 0) ? my_numevents : 0;
		 cl_event *loop_preevents = (i == 0) ? preevents : 0;
		 cl_event * postevent = (i == loops - 1) ? &this->m_postevent : 0; 
              this->startMsg();

			   if ( i != loops - 1)
			   {
			       this->luanchedThreadInc();
			   }

	          cl_err = clEnqueueNDRangeKernel(this->m_queue,
                                          kernel,
                                          1,
                                          NULL,
                                          ndrg,
                                          ndrl,
                                          loop_num,
                                          loop_preevents,
                                          postevent);
		      ASSERT_CL_RETURN(cl_err);

			   if ( i != loops - 1)
			   {
				    this->sunkThreadInc();
			   }

		 }


		 if ( preevents )
		 {
			 delete [] preevents;
		 }

		 CGPUTest<TYPE>::Launch(my_numevents, preevent_list);

		 if (my_preevent_list)
		 {
			 delete[] my_preevent_list;
		 }

	 }

public:


};



template <typename TYPE>
class CPIMGPU_DOTPROD : public CGPU_DOTPROD<TYPE>
{
public:
	 CPIMGPU_DOTPROD<TYPE>(const TestConfig *config_ = 0)
					   : CGPU_DOTPROD<TYPE> (config_)
	{
	}

     void Init (uint32_t mem_flags_ = PIM_MEM_DEFAULT_FLAGS )
	 {
	 cl_int cl_err = CL_SUCCESS;
	    this->x_length =
     	this->y_length = this->problem_sz;
		int nmb_CUs;
		size_t ret_size;
			pim_get_device_info(this->pim_id, PIM_GPU_NUM_CUS, sizeof(int), &nmb_CUs, &ret_size);

		this->z_length = nmb_CUs * 16 * 64;
		CPimTest<TYPE> :: PIMMemInit(mem_flags_);



     TYPE * px = (TYPE * )pim_map(this->x, this->platform);
     TYPE * py = (TYPE * )pim_map(this->y, this-> platform);

        fillWithRand<TYPE>(px,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);
        fillWithRand<TYPE>(py,this->problem_sz,1, (TYPE)0.1, (TYPE)1000.);

	    pim_unmap(px);
	    pim_unmap(py);
	 }

     void Launch(int num_preevents = 0, CPimTest<TYPE>** preevent_list = 0)
     {

	 cl_int cl_err = CL_SUCCESS;
	 int len = (int)this->problem_sz;
     char* ocl_source;
     const void * args[1024];
     size_t sizes[1024];
     size_t nargs = 0;
     int dim = 1;
	 size_t ndrl[3] = {64, 1, 1};
	 size_t ndrg[3] = {((this->z_length + ndrl[0] - 1)/ndrl[0]) * ndrl[0], 1, 1};
	 pim_f gpu_kernel;
	 int my_numevents;
	 cl_event * preevents = 0;
	 CPimTest<TYPE> ** my_preevent_list = 0;

	        this->SortOutEvents( &my_numevents, &my_preevent_list, num_preevents, preevent_list  );

	        if ( my_numevents > 0 )
			{
				preevents = new cl_event[my_numevents];
				assert(preevents);
				for( int i = 0; i < num_preevents; i++)
				{

					  preevents[i] = my_preevent_list[i]->getPostEvent();
				}
			}

     char * dir = fix_directory(this->m_kernel_loc);
	         assert(dir);
             ocl_source = concat_directory_and_name(dir, this->m_kernel_file);
	         assert(ocl_source);

			 gpu_kernel.func_name = (void*)this->test_nm;

		     nargs = 0;
             args[nargs] =  ocl_source;
	         sizes[nargs++] = sizeof(char*);

             args[nargs] =  this->m_kernel_opt;
	         sizes[nargs++] = sizeof(char*);
// kernel launch arguments
             args[nargs] =  &dim;
	         sizes[nargs++] = sizeof(int);

             args[nargs] =  ndrg;
	         sizes[nargs++] = sizeof(size_t) * dim;

             args[nargs] =  ndrl;
	         sizes[nargs++] = sizeof(size_t) * dim;

// event arguments
// pre-events
             args[nargs] =  &my_numevents;
	         sizes[nargs++] = sizeof(int);

// pre-events
             args[nargs] =  preevents;
	         sizes[nargs++] = sizeof(cl_event*);

// post-events
             args[nargs] =  &this->m_postevent;
	         sizes[nargs++] = sizeof(cl_event*);

// kernel arguments
             args[nargs] = &len;
	         sizes[nargs++] = sizeof(int);

			args[nargs] = this->x;
	        sizes[nargs++] = sizeof(void*);
			args[nargs] = this->y;
	        sizes[nargs++] = sizeof(void*);
			args[nargs] = this->z;
	        sizes[nargs++] = sizeof(void*);
     int loops = this->m_test_config.tests[this->m_test_id].loops;

           for(int i = 0; i < loops; i++)
	       {
	       int loop_num = (i == 0) ? my_numevents : 0;
// we cannot yet get pim ID frominside GPU
// sending it beforehand
                this->startMsg(this->pim_id);

// event arguments
// pre-events
                args[OPENCL_ARG_NUMPREEVENTS] =  &loop_num;
// pre-events
                args[OPENCL_ARG_PREEVENTS] =  (i == 0) ? preevents : 0;

		 // post-events
               args[OPENCL_ARG_POSTEVENT] =  (i == loops - 1) ? &this->m_postevent : 0;

			   if ( i != loops - 1)
			   {
			       this->luanchedThreadInc();
			   }

         void *spawn_error = NULL;
	           spawn_error = pim_spawn(gpu_kernel, (void**)args, sizes, nargs, this->pim_id, this->platform);
	           if (spawn_error == NULL)
	           {
	                PIM_TESTERROR_MSG("Faild to launch PIM thread\n");
		       }


			   if ( i != loops - 1)
			   {
				    this->sunkThreadInc();
			   }
	     }

		 if ( preevents )
		 {
			 delete [] preevents;
		 }

		 CGPUTest<TYPE>::Launch(my_numevents, preevent_list);

		 if (my_preevent_list)
		 {
			 delete[] my_preevent_list;
		 }

	 }

public:


};


#endif  /* __PIM_TEST_HPP_ */
