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

#include "pimg.hpp"
#include "opencl_gpu_emulate.h"
#include "psg_shared.h"
#include "pimemu_clinterceptor.h"

static int convertToString(const char *filename, std::string& s);
static string extractProgName( const char *filename_);



static helpFactory * myFactory = 0;

#ifdef WIN32
class WinConstrDestructHelperlper
{
public:
	WinConstrDestructHelperlper()
	{
	    pim_constructor ();
	}

	~WinConstrDestructHelperlper()
	{
		  pim_destructor ( );

	}
};

static WinConstrDestructHelperlper sWinHelper;

#endif

 
/***********************************************************************************************

***********************************************************************************************/
helpFactory :: helpFactory( void)
   {


	   pthread_rwlock_init(&m_event_rwlock, NULL);	
       m_default_dev = 0;
	   sPIMGPUs = 0;
	   active_counter = 0;
       m_ctxt = 0;
       m_gpu_devices.clear();
	   m_global_outsranding_events.clear();
	   m_implicit_events.clear();
	   m_myqueues.clear();

	   m_event_message_q.clear();

   m_prog_map.clear();
   m_kern_map.clear();



   }
helpFactory :: ~helpFactory( void)
   {

// pim_destructor

//	   printf("our_events %lld, implicit events %lld\n", m_myqueues.size(), m_implicit_events.size());

	   if ( sPIMGPUs ) 
	   {
//		   cout << "before deleting\n";
		   delete [] sPIMGPUs;
//		   cout << "after deleting\n";
		   sPIMGPUs = 0;
		   active_counter = 0;
	   }


   }

cl_int helpFactory :: getDeviceList( cl_device_type type_,
                          VECTOR_CLASS<cl::Device>& devices_
						)
{
cl_int ret = CL_SUCCESS;
	cl::Platform platform = cl::Platform::getDefault();
	if(strcmp(platform.getInfo<CL_PLATFORM_VENDOR>().c_str(), "Advanced Micro Devices, Inc."))
	{
		std::cout<<"PIM OpenCl: Default platform should be Advanced Micro Devices, Inc. to run this sample\n"<<std::endl;
		exit(0);
	}

	ret = platform.getDevices(type_, &devices_);

	cout << "PIM OpenCl: Found following " << "GPU" << " devices:\n";

VECTOR_CLASS<cl::Device>:: iterator j;
int i;
	for (j = (devices_).begin (), i = 0; j != (devices_).end (); j++, i++ )
	{
	 string dev_name;
		   (*j).getInfo<string>(CL_DEVICE_NAME, &dev_name);
		   cout << i << " " << dev_name.c_str() << "\n";

	}

	return(ret);

}


cl::CommandQueue helpFactory :: getSingleQ( void )
{
	if ( !m_single_launch_q() )
	{
cl_int err = CL_SUCCESS;
cl::CommandQueue tmp(getContext(), getDevice(), 0, &err);
	 ASSERT_CL_RETURN(err);
	  m_single_launch_q = tmp;
	}
	return(m_single_launch_q);
}
CPIM_GPU * helpFactory :: gpufactory( void)
   {
   CPIM_GPU newPIMGPU;
   cl_device_type type = CL_DEVICE_TYPE_GPU;
   cl_int err = CL_SUCCESS;

	   pthread_rwlock_wrlock(&m_event_rwlock);

       if ( sPIMGPUs == NULL )
       {
	   
           getDeviceList( type, m_gpu_devices);

// common context for all devices found
           cl::Context tmp_c(m_gpu_devices,NULL, NULL, NULL, &err);
	       ASSERT_CL_RETURN(err);
	       m_ctxt = tmp_c;
	       sPIMGPUs = new CPIM_GPU[_MAX_NUM_OF_PIMGPUS];
       }

       CPIM_GPU * ret = &sPIMGPUs[active_counter];
	   ret->setID(active_counter);
	   ret->setPimGpuFactory(this);

	   active_counter++;

	   pthread_rwlock_unlock(&m_event_rwlock);
	   return(ret);
   }


cl_kernel helpFactory :: compileKernel(const char *kernel_, const char *filename_, const char *options_)
{

cl_kernel ret;
cl_int status = 0;

  

      assert(kernel_ && filename_);

 
    // create a CL program using the kernel source
string kernel_nm(kernel_); 
string prog_nm = filename_;
string opt = (options_) ? options_ : "";

      assert(prog_nm.length() > 0 );

string last_opt;
program_cache_map :: iterator p;
kernel_cache_map :: iterator k;
string prog_tag = prog_nm + "." + opt;
string  kern_tag = prog_tag + "." + kernel_nm;
cl_int err;


  

      p = m_prog_map.find(prog_tag);

	 ret = 0;
// prog already compiled ?
	 if ( p != m_prog_map.end()  )
	 {
		 k = m_kern_map.find( kern_tag );
		 if ( k != m_kern_map.end() )
		 {
     		 ret = k->second;
		 }
		 else
		 {
  	        ret = clCreateKernel(p->second(), kernel_, &err);
            ASSERT_CL_RETURN(err);
	        m_kern_map.insert(make_pair(kern_tag, ret));
		 }
	 }

	 else
	 {
     

// otherwise compile
    std::string sourceStr;

         status = convertToString(filename_, sourceStr);
         if (status != CL_SUCCESS) 
         {
             std::cout << "PIM OpenCl: Failed to open OCL source file " << filename_ << std::endl;
			 
         }
		 else
		 {
         
    // create program through .cl file
        cl::Program new_prog(getContext(), std::string(sourceStr), false, &status);
            ASSERT_CL_RETURN(status);
	        status= new_prog.build(options_);
            ASSERT_CL_RETURN(status);
//	cout << "Built program: " << filename_ << ",\n";
	        m_prog_map.insert(make_pair(prog_tag, new_prog));

  	        ret = clCreateKernel(new_prog(), kernel_, &err);
            ASSERT_CL_RETURN(err);
	        m_kern_map.insert(make_pair(kern_tag, ret));
		 }
	 }



	return(ret);
}




void helpFactory :: addPimClEvent( cl_event event_ )
{
	pthread_rwlock_wrlock(&m_event_rwlock);
	m_global_outsranding_events.push_back(event_);
	pthread_rwlock_unlock(&m_event_rwlock);
}


int helpFactory :: removePimClEvent( cl_event event_ )
{
VECTOR_CLASS<cl_event> :: iterator r;
int ret = 0;

	pthread_rwlock_wrlock(&m_event_rwlock);
	if ( (r = findPimClEvent(event_)) !=  m_global_outsranding_events.end() )
	{

//		clReleaseEvent(*r);
		m_global_outsranding_events.erase(r);
		ret = 1;

	}
    pthread_rwlock_unlock(&m_event_rwlock);
	return(ret);
	
}

VECTOR_CLASS<cl_event> :: iterator helpFactory :: findPimClEvent(cl_event event_)
{
VECTOR_CLASS<cl_event> :: iterator r;

    for (r = m_global_outsranding_events.begin(); r != m_global_outsranding_events.end(); r++)
	{
		if ( *r == event_ )
		{ 
			break;
		}
	}
    return(r);
}

int helpFactory :: isPimClEvent( cl_event event_ )
{
VECTOR_CLASS<cl_event> :: iterator r;
int ret = 0;
   pthread_rwlock_wrlock(&m_event_rwlock);
   ret = ( (r = findPimClEvent(event_)) !=  m_global_outsranding_events.end() );
   pthread_rwlock_unlock(&m_event_rwlock);

   return(ret);
}

cl_event*  helpFactory :: addPimImplicitClEvent(cl_command_queue q_, cl_event event_ )
{
cl_event * ret = 0;
	 pthread_rwlock_wrlock(&m_event_rwlock);
     m_implicit_events.insert(make_pair(q_, event_));
	 ret = &m_implicit_events.find(q_)->second;
	 pthread_rwlock_unlock(&m_event_rwlock);
	 return(ret);
}

cl_event helpFactory :: removePimImplicitClEvent(cl_command_queue q_)
{
cl_event ret = 0;
	pthread_rwlock_wrlock(&m_event_rwlock);
	queue_to_event_map :: iterator q;
	if ( (q = m_implicit_events.find(q_)) != m_implicit_events.end() )
	{
		ret = q->second;
		m_implicit_events.erase(q_);

	}
	pthread_rwlock_unlock(&m_event_rwlock);
	return(ret);
}

void helpFactory :: clearPimImplicitClEvent( void )
{
	pthread_rwlock_wrlock(&m_event_rwlock);
	m_implicit_events.clear();
	pthread_rwlock_unlock(&m_event_rwlock);
}

cl_event *helpFactory :: getPimImplicitClEvent(cl_command_queue q_)
{
cl_event *ret = NULL;
  pthread_rwlock_wrlock(&m_event_rwlock);
   if ( m_implicit_events.find(q_) != m_implicit_events.end() ) 
   {
	    ret = &(m_implicit_events.find(q_)->second);
   }
   pthread_rwlock_unlock(&m_event_rwlock);
   return(ret);
}

void helpFactory :: getPimImplicitClEvenArray(int *array_sz_, cl_event ** array_)
{
queue_to_event_map:: iterator m;
int i;
    
    if (array_sz_)
	{
		*array_sz_ = 0;
	}

	pthread_rwlock_wrlock(&m_event_rwlock);
	if ( !m_implicit_events.empty() && array_sz_ && array_)
	{
	   *array_ = ( cl_event *) malloc(m_implicit_events.size() * sizeof(cl_event));
	   assert(*array_);
       for( i = 0, m = m_implicit_events.begin(); m != m_implicit_events.end(); m++)
       {
		   (*array_)[i] = (*m).second;
       }
	}

	m_implicit_events.clear();
	pthread_rwlock_unlock(&m_event_rwlock);
}

void helpFactory :: addPimClQue( cl_command_queue que_ )
{
	pthread_rwlock_wrlock(&m_event_rwlock);
	m_myqueues.push_back(que_);
	pthread_rwlock_unlock(&m_event_rwlock);
}

void helpFactory :: removePimClQue( cl_command_queue que_ )
{
	pthread_rwlock_wrlock(&m_event_rwlock);
VECTOR_CLASS<cl_command_queue> :: iterator q;
    for (q = m_myqueues.begin(); q != m_myqueues.end(); q++)
	{
		if ( *q == que_ )
		{ 
			m_myqueues.erase(q);
			break;
		}
	}
	pthread_rwlock_unlock(&m_event_rwlock);
}

int helpFactory :: getPimClQues( cl_command_queue **que_)
{
int ret = 0;
    if ( que_ ) 
	{
		*que_= 0;
		if (!m_myqueues.empty())
		{
             ret = (int)m_myqueues.size();

             *que_ = &m_myqueues[0];
		}
	}
    return(ret);
}





/******************************************************************************************/
void helpFactory :: stackTimeLineMsg( const char * msg)
{
string tmp(msg);
	pthread_rwlock_wrlock(&m_event_rwlock);
	m_event_message_q.push_back(tmp);
	pthread_rwlock_unlock(&m_event_rwlock);

}
void helpFactory :: popTimeLineMsg( FILE * msg_file)
{
	pimemuSendString(msg_file, m_event_message_q.back().c_str());


	m_event_message_q.pop_back();


}

void helpFactory :: sendStackedTimeLineMsg( FILE * msg_file)
{
	pthread_rwlock_wrlock(&m_event_rwlock);
	while (!m_event_message_q.empty())
	{
		popTimeLineMsg( msg_file);
	}
	pthread_rwlock_unlock(&m_event_rwlock);
}



/******************************************************************************************/


int buildPIMFactory(void)
{
	if ( !myFactory )
	{
		myFactory = new helpFactory;
		assert(myFactory);
	}
	return(myFactory != NULL);
}

int removePIMFactory(void)
{
    if ( myFactory )
    {
	    delete myFactory;
	    myFactory = 0;
    }
	return(0);
}


helpFactory * getHelpFactory( void )
{
	buildPIMFactory();
	return(myFactory);
}

CPIM_GPU * gpuFactory( void)
{
CPIM_GPU * ret = 0;
   if (buildPIMFactory())
   {
       ret = myFactory->gpufactory();
   }
   return (ret);
}

/***********************************************************************************************

***********************************************************************************************/


CPIM_GPU :: CPIM_GPU( void)
{




}

CPIM_GPU :: ~CPIM_GPU( void)
{

}

int CPIM_GPU :: pimgInit( void )
{
int ret = 0;
cl_int err = CL_SUCCESS;

// init kernel cache lock 
     pthread_rwlock_init(&m_comp_rwlock, NULL);


    m_launch_q =  getQ( );

    return(ret);
}

cl::CommandQueue CPIM_GPU :: getQ( void )
{
#if 0
cl_int err = CL_SUCCESS;
cl::CommandQueue tmp(getPimGpuFactory()->getContext(), getPimGpuFactory()->getDevice(), 0, &err);
	 ASSERT_CL_RETURN(err);
    return(tmp);
#else
	return(getPimGpuFactory()->getSingleQ( ));
#endif
}


// launch kernel on the default device
cl_int CPIM_GPU :: 	launchKernelAsync(const char *kernelname_,
	                    const void** args_, 
                        const size_t* arg_sizes_, 
                        size_t nargs_)
{
cl_int ret = CL_SUCCESS;
int nmb_arg;
const char *kern_file = (const char *)args_[OPENCL_ARG_SOURCE];
const char *kern_opt = (const char *)args_[OPENCL_ARG_OPTIONS];
cl_event * post_event = (cl_event*)args_[OPENCL_ARG_POSTEVENT];
int iter = 1;

// stop counters
 //      incAndStopCnt();

cl_kernel krnl0 = compileKernel(kernelname_, kern_file, kern_opt);



  

       if ( !krnl0 )
	   {

		   return(-1);
	   }

int dim = *(int*)args_[OPENCL_ARG_DIM];
size_t glbl[3] = {1,1,1};
size_t lcl[3] = {1,1,1};
   for(int i = 0; i < dim; i++)
   {
      glbl[i] = ((size_t*)args_[OPENCL_ARG_GLOBALSIZE])[i];
      lcl[i] = ((size_t*)args_[OPENCL_ARG_LOCALSIZE])[i];
   }

	   ret = clGetKernelInfo(krnl0,CL_KERNEL_NUM_ARGS, sizeof(int), &nmb_arg, NULL);
	   ASSERT_CL_RETURN(ret);

	   if ( nmb_arg != (int)nargs_ - NUM_OPENCL_ARGUMENTS )
	   {
		   cout << "PIM OpenCl: Wrong number of kernel arguments. Compiled: " << nmb_arg <<
			   " Passsed: " << (int)nargs_ - NUM_OPENCL_ARGUMENTS << "\n";
	   }

// critical section
// setarg is not thread-safe

	   pthread_rwlock_wrlock(&m_comp_rwlock);

	   for(int i = 0, k = NUM_OPENCL_ARGUMENTS; i < nmb_arg; i++, k++)
	   {
       std::string type_nm;
	   char c_type_nm[512];
	   size_t real_size;
	   void * argPtr = (void*)args_[k];
	   ::size_t arg_sz = arg_sizes_[k];
	   cl_mem mem_obj;


    	   ret = clGetKernelArgInfo(krnl0,
			                        i,
									CL_KERNEL_ARG_TYPE_NAME,
									511,
			                        c_type_nm,
									&real_size);
	       ASSERT_CL_RETURN(ret);
 // TO DO: local memory size

		   if (c_type_nm[real_size-2] == '*' )
		   {
           cl_int aq_id;
    	       ret = clGetKernelArgInfo(krnl0,
			                        i,
									CL_KERNEL_ARG_ADDRESS_QUALIFIER,
									sizeof(cl_int),
			                        &aq_id,
									&real_size);
	       ASSERT_CL_RETURN(ret);

           cl::Buffer* bPtr = (cl::Buffer* )args_[k]; 
		   // Ptr == NULL
		   // it's either local memory
		   // or an error
		       if ( !bPtr && CL_KERNEL_ARG_ADDRESS_LOCAL != aq_id )
			   {
		           cout << "PIM OpenCl: Kernel buffer argument with NULL ptr. \n";
			   }
	           else if ( bPtr )
			   {
		            mem_obj = (*bPtr)();
			        argPtr = &mem_obj;
			        arg_sz = sizeof(cl_mem);
			   }
		   }

		   ret |= clSetKernelArg(krnl0   /* kernel */,
                                 i     /* arg_index */,
                                 arg_sz      /* arg_size */,
                                 argPtr /* arg_value */);

	   }

	   pthread_rwlock_unlock(&m_comp_rwlock);

// end of critical section

	   ASSERT_CL_RETURN(ret);



cl_int k_ret;


   for( int i = 0; i < iter; i++ )
   {
#if PIMRT
        if( isEmuMode() )
		{

        launchOclKernelStruct ocl_args;


              memset(&ocl_args, 0, sizeof(ocl_args));

		      ocl_args.command_queue =  m_launch_q();
		      ocl_args.kernel = krnl0;
              ocl_args.work_dim = dim;
		      ocl_args.global_work_offset = NULL;
              ocl_args.global_work_size = glbl;
              ocl_args.local_work_size = lcl;
			  ocl_args.num_events_in_wait_list = *(int*)args_[OPENCL_ARG_NUMPREEVENTS] ;
              ocl_args.event_wait_list = (cl_event*)args_[OPENCL_ARG_PREEVENTS];
			  ocl_args.event = post_event;  


              k_ret = CaptureEnqKernel(&ocl_args,
	                           "OCL_LAUNCH_PIM",
					           getID());

 
// start counters
//            decAndStartCnt();

		}
		else
#endif
		{
	    cl::Kernel krnl(krnl0);
        VECTOR_CLASS<cl::Event> pre_events;
              for ( int k = 0; k < *(int*)args_[OPENCL_ARG_NUMPREEVENTS]; k++)
	          {
		      cl::Event tmp(((cl_event*)args_[OPENCL_ARG_PREEVENTS])[k]);
		           pre_events.push_back(tmp);
	          }

       cl::NDRange ndrg(glbl[0], glbl[1], glbl[2]);
       cl::NDRange ndrl(lcl[0], lcl[1], lcl[2]);
       cl::Event ev;

       VECTOR_CLASS<cl::Event> *pre_events_ptr = (pre_events.size() > 0 )? &pre_events : NULL;

             if ( post_event )
             {
	              ev = (cl::Event)(*post_event);
             }
             k_ret = m_launch_q.enqueueNDRangeKernel(krnl, cl::NullRange,ndrg, ndrl,pre_events_ptr, &ev);
 
             ASSERT_CL_RETURN(k_ret);

             if ( post_event )
             {
	             *post_event = ev();
	             clRetainEvent(*post_event);
             }
             else
             {
                  ev.wait();
             }

		}


   }


 //  cout << "Execute kernel: " << kernelname_ << " on PIM GPU with id " << getID() << "\n";
   ret |= k_ret;

   
   return(ret);
}






 



cl_kernel CPIM_GPU :: compileKernel(const char *kernel_, const char *filename_, const char *options_)
{
cl_kernel ret = 0;
	    pthread_rwlock_wrlock(&m_comp_rwlock);
	    ret = getPimGpuFactory()->compileKernel(kernel_, filename_, options_);
		pthread_rwlock_unlock(&m_comp_rwlock);
		return(ret);

}


cl::Buffer * CPIM_GPU :: allocBuffer(
                            ::size_t size_,
                            void* host_ptr_,
							cl_mem_flags flags_
						    )
{
cl::Buffer * ret = 0;
cl_int err = CL_SUCCESS;
    ret = new cl::Buffer(getPimGpuFactory()->getContext(),
		                 flags_,
                         size_,
                         host_ptr_,
						 &err);
     ASSERT_CL_RETURN(err);

	return(ret);

}

cl_int CPIM_GPU :: releaseBuffer( cl::Buffer * buffer_)
{
cl_int ret = CL_SUCCESS;
   if ( buffer_ )
   {
	   delete buffer_;
   }
   return(ret);
}

// map buffer for a default device 
// TODO: test for multiple maps
// TODO: clean cache
void * CPIM_GPU :: mapBuffer(cl::Buffer * buffer_, cl_map_flags map_flags_)
{
void * ret = 0;
cl_int err = CL_SUCCESS;
    if ( buffer_ )
	{
    size_t buf_sz;
//		 ::itIsRuntimeCall(1);
//        ::beforeGenericEvent();

		buf_sz = buffer_->getInfo<CL_MEM_SIZE>();
        ret =  m_launch_q.enqueueMapBuffer(*buffer_,
		                                    CL_TRUE,
											map_flags_,
											0,
											buf_sz,
											NULL,
											NULL,
											&err);
		 ASSERT_CL_RETURN(err);
 //        ::afterGenericEvent();
//		 ::itIsRuntimeCall(0);
	 

	}
 
	return(ret);

}

// unmap buffer for a default device 
cl_int CPIM_GPU :: unmapBuffer( cl::Buffer * buffer_, void * mapped_ptr_)
{
cl_int ret = CL_SUCCESS;
   if ( buffer_ && mapped_ptr_ )
   {
   	cl::Event implicit;
//		 ::itIsRuntimeCall(1);
//        ::beforeGenericEvent();

	  ret = m_launch_q.enqueueUnmapMemObject(*buffer_, mapped_ptr_,NULL, &implicit);


		ASSERT_CL_RETURN(ret);
		implicit.wait();
//         ::afterGenericEvent();
//		 ::itIsRuntimeCall(0);

   }

   return(ret);
}

static
string extractProgName( const char *filename_)
{
string ret;
string str(filename_);
unsigned int found = (unsigned int)str.find_last_of("/\\");

   ret = ( found ==  string::npos ) ? str :  str.substr(found+1);
   return(ret);
}

static int convertToString(const char *filename, std::string& s)
{
    size_t size;
    char*  str;

    // create a file stream object by filename
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));


    if(!f.is_open())
    {
     	return EXIT_FAILURE;   
    }
    else
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size+1];
        if(!str)
        {
            f.close();
            return EXIT_FAILURE;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';

        s = str;
        delete[] str;
        return CL_SUCCESS;
    }
}

