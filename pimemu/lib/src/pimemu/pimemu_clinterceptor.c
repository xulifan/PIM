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

#include <memory.h>
#include <assert.h>
#include "pim-private.h"
#include "pimemu_clinterceptor.h"


#define PIM_CLICTR_MSG(msg)               \
{                                         \
   if (msg != NULL)                       \
   { \
      fprintf(stderr, "PIM CLINTERCEPTOR: %s (%d): %s\n", __func__, __LINE__, msg);    \
   }\
}

#define DECLARE_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
	static piminterceptor_cl##_FUNCNAME_ _FUNCNAME_ = NULL;


#ifdef WIN32
#define GET_INTERCEPTOR_FUNCTION_ADDRESS(_FUNCNAME_) \
{ \
  char func_name[128];\
    sprintf(func_name,"cl%s",#_FUNCNAME_); \
	_FUNCNAME_ = (piminterceptor_cl##_FUNCNAME_)GetProcAddress(oclDllHandle, func_name); \
}
#else
#define GET_INTERCEPTOR_FUNCTION_ADDRESS(_FUNCNAME_) \
{ \
  char *error; \
  char func_name[128];\
    sprintf(func_name,"cl%s",#_FUNCNAME_); \
	_FUNCNAME_ = (piminterceptor_cl##_FUNCNAME_)dlsym(oclDllHandle, func_name); \
	 error = dlerror(); \
    if (error != NULL) { \
       PIM_CLICTR_MSG("No function"); \
	} \
}

#endif

#define GET_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
piminterceptor_cl##_FUNCNAME_ get##_FUNCNAME_( void) \
{ \
	return	(_FUNCNAME_); \
}

/*************** intercepted functions ********************************************************/
DECLARE_INTERCEPTOR_FUNCTION( GetPlatformIDs );
DECLARE_INTERCEPTOR_FUNCTION( GetPlatformInfo );
DECLARE_INTERCEPTOR_FUNCTION( GetDeviceIDs );
DECLARE_INTERCEPTOR_FUNCTION( GetDeviceInfo );
DECLARE_INTERCEPTOR_FUNCTION( CreateSubDevices );
DECLARE_INTERCEPTOR_FUNCTION( RetainDevice );
DECLARE_INTERCEPTOR_FUNCTION( ReleaseDevice );
DECLARE_INTERCEPTOR_FUNCTION( CreateContext );
DECLARE_INTERCEPTOR_FUNCTION( CreateContextFromType );
DECLARE_INTERCEPTOR_FUNCTION( RetainContext );
DECLARE_INTERCEPTOR_FUNCTION( ReleaseContext );
DECLARE_INTERCEPTOR_FUNCTION( GetContextInfo );
DECLARE_INTERCEPTOR_FUNCTION( CreateCommandQueue );
DECLARE_INTERCEPTOR_FUNCTION( RetainCommandQueue);
DECLARE_INTERCEPTOR_FUNCTION( ReleaseCommandQueue);
DECLARE_INTERCEPTOR_FUNCTION( GetCommandQueueInfo);

DECLARE_INTERCEPTOR_FUNCTION( CreateBuffer);
DECLARE_INTERCEPTOR_FUNCTION( CreateSubBuffer );
DECLARE_INTERCEPTOR_FUNCTION( CreateImage );
DECLARE_INTERCEPTOR_FUNCTION( GetMemObjectInfo);
DECLARE_INTERCEPTOR_FUNCTION( RetainMemObject);
DECLARE_INTERCEPTOR_FUNCTION( ReleaseMemObject);
DECLARE_INTERCEPTOR_FUNCTION( GetSupportedImageFormats );
DECLARE_INTERCEPTOR_FUNCTION( GetImageInfo );
DECLARE_INTERCEPTOR_FUNCTION( SetMemObjectDestructorCallback );
DECLARE_INTERCEPTOR_FUNCTION( CreateSampler );
DECLARE_INTERCEPTOR_FUNCTION( RetainSampler );
DECLARE_INTERCEPTOR_FUNCTION( ReleaseSampler );
DECLARE_INTERCEPTOR_FUNCTION( GetSamplerInfo );


DECLARE_INTERCEPTOR_FUNCTION( CreateProgramWithSource);
DECLARE_INTERCEPTOR_FUNCTION( CreateProgramWithBinary);
DECLARE_INTERCEPTOR_FUNCTION( CreateProgramWithBuiltInKernels);
DECLARE_INTERCEPTOR_FUNCTION( RetainProgram);
DECLARE_INTERCEPTOR_FUNCTION( ReleaseProgram);
DECLARE_INTERCEPTOR_FUNCTION( BuildProgram);
DECLARE_INTERCEPTOR_FUNCTION( CompileProgram);
DECLARE_INTERCEPTOR_FUNCTION( LinkProgram);
DECLARE_INTERCEPTOR_FUNCTION( UnloadPlatformCompiler);
DECLARE_INTERCEPTOR_FUNCTION( GetProgramInfo);
DECLARE_INTERCEPTOR_FUNCTION( GetProgramBuildInfo);
 /* Kernel Object APIs */
DECLARE_INTERCEPTOR_FUNCTION( CreateKernel);
DECLARE_INTERCEPTOR_FUNCTION( CreateKernelsInProgram);
DECLARE_INTERCEPTOR_FUNCTION( RetainKernel);
DECLARE_INTERCEPTOR_FUNCTION( ReleaseKernel);
DECLARE_INTERCEPTOR_FUNCTION( SetKernelArg);
DECLARE_INTERCEPTOR_FUNCTION( GetKernelInfo);
DECLARE_INTERCEPTOR_FUNCTION( GetKernelArgInfo);
DECLARE_INTERCEPTOR_FUNCTION( GetKernelWorkGroupInfo);
/* Event Object APIs */
DECLARE_INTERCEPTOR_FUNCTION( WaitForEvents);
DECLARE_INTERCEPTOR_FUNCTION( GetEventInfo);
DECLARE_INTERCEPTOR_FUNCTION( CreateUserEvent);               
DECLARE_INTERCEPTOR_FUNCTION( RetainEvent);
DECLARE_INTERCEPTOR_FUNCTION( ReleaseEvent);
DECLARE_INTERCEPTOR_FUNCTION( SetUserEventStatus);
DECLARE_INTERCEPTOR_FUNCTION( SetEventCallback);
/* Profiling APIs */
DECLARE_INTERCEPTOR_FUNCTION( GetEventProfilingInfo);                                
/* Flush and Finish APIs */
DECLARE_INTERCEPTOR_FUNCTION( Flush);
DECLARE_INTERCEPTOR_FUNCTION( Finish);
/* Enqueued Commands APIs */
DECLARE_INTERCEPTOR_FUNCTION( EnqueueReadBuffer);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueReadBufferRect);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueWriteBuffer);                            
DECLARE_INTERCEPTOR_FUNCTION( EnqueueWriteBufferRect);                            
DECLARE_INTERCEPTOR_FUNCTION( EnqueueFillBuffer);                            
DECLARE_INTERCEPTOR_FUNCTION( EnqueueCopyBuffer);                            
DECLARE_INTERCEPTOR_FUNCTION( EnqueueCopyBufferRect);                            
DECLARE_INTERCEPTOR_FUNCTION( EnqueueReadImage);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueWriteImage);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueFillImage);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueCopyImage);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueCopyImageToBuffer);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueCopyBufferToImage);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueMapBuffer);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueMapImage);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueUnmapMemObject);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueMigrateMemObjects);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueNDRangeKernel);
DECLARE_INTERCEPTOR_FUNCTION( EnqueueTask );
DECLARE_INTERCEPTOR_FUNCTION( EnqueueNativeKernel );
DECLARE_INTERCEPTOR_FUNCTION( EnqueueMarkerWithWaitList );
DECLARE_INTERCEPTOR_FUNCTION( EnqueueBarrierWithWaitList );
DECLARE_INTERCEPTOR_FUNCTION( SetPrintfCallback );
DECLARE_INTERCEPTOR_FUNCTION( GetExtensionFunctionAddressForPlatform );



/*******************************************************************************************************/

/*****************************************************************************/
/************ internals *****************************************************/
//GET_INTERCEPTOR_FUNCTION_ADDRESS

static
#ifdef WIN32
int pim_GET_CLINTERCEPTOR_FUNCTION_ADDRESSes( HINSTANCE oclDllHandle )
#else
int pim_GET_CLINTERCEPTOR_FUNCTION_ADDRESSes( void* oclDllHandle )
#endif
{
int ret = 0;
	   GET_INTERCEPTOR_FUNCTION_ADDRESS(GetPlatformIDs);
	   GET_INTERCEPTOR_FUNCTION_ADDRESS(GetPlatformInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetDeviceIDs );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetDeviceInfo );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateSubDevices );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainDevice ); 
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseDevice );
	   GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateContext );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateContextFromType );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainContext );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseContext );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetContextInfo );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateCommandQueue );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseDevice );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainCommandQueue);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseCommandQueue);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetCommandQueueInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateBuffer);
	   GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateSubBuffer );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateImage );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetMemObjectInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainMemObject);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseMemObject);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetSupportedImageFormats );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetImageInfo );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( SetMemObjectDestructorCallback );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateSampler );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainSampler );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseSampler );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetSamplerInfo );

       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateProgramWithSource);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateProgramWithBinary); 
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateProgramWithBuiltInKernels);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainProgram);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseProgram);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( BuildProgram);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CompileProgram);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( LinkProgram);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( UnloadPlatformCompiler);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetProgramInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetProgramBuildInfo);
 /* Kernel Object APIs */
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateKernel);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateKernelsInProgram);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainKernel);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseKernel);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( SetKernelArg);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetKernelInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetKernelArgInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetKernelWorkGroupInfo);
/* Event Object APIs */
       GET_INTERCEPTOR_FUNCTION_ADDRESS( WaitForEvents);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetEventInfo);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( CreateUserEvent);               
       GET_INTERCEPTOR_FUNCTION_ADDRESS( RetainEvent);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseEvent);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( SetUserEventStatus);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( SetEventCallback);
/* Profiling APIs */
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetEventProfilingInfo);                                
/* Flush and Finish APIs */
       GET_INTERCEPTOR_FUNCTION_ADDRESS( Flush);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( Finish);
/* Enqueued Commands APIs */
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueReadBuffer);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueReadBufferRect);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueWriteBuffer);                            
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueWriteBufferRect);                            
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueFillBuffer);                            
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyBuffer);                            
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyBufferRect);                            
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueReadImage);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueWriteImage);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueFillImage);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyImage);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyImageToBuffer);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyBufferToImage);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueMapBuffer);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueMapImage);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueUnmapMemObject);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueMigrateMemObjects);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueNDRangeKernel);
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueTask );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueNativeKernel );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueMarkerWithWaitList );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueBarrierWithWaitList );
  //     GET_INTERCEPTOR_FUNCTION_ADDRESS( SetPrintfCallback );
       GET_INTERCEPTOR_FUNCTION_ADDRESS( GetExtensionFunctionAddressForPlatform );


   return(ret);
}

/******************************************************************************/


int beforeGenericEvent( void)
{
launchThreadStruct *currlaunchStruct;

 
   if ( isEmuMode()  && isCapturing() && isThreadCounting() )
   {

// alway stop CPU counter 
	    OnStopCPUCounters();
// and bump order
		pim_thread_incorder();


   }
   return(0);
}


int afterGenericEvent( void)
{
launchThreadStruct *currlaunchStruct;
int i;
char msg[1024];

 
   if ( isEmuMode()  && isCapturing() && isThreadCounting() )
   {

	    currlaunchStruct = pim_thread_getlaunchstruct();
		sprintf(msg, "PIMEMU_END,\t%d,\t%llu\n",currlaunchStruct->thread_id, currlaunchStruct->call_order);
		OnOclWaitEvent(NULL,msg);
   }
   return(0);
}

static
#ifdef WIN32
HINSTANCE
#else
void *
#endif
oclDllHandle = NULL; 

int pim_cliterceptorInit( void )
{
int ret = 0;
 
   if ( NULL == oclDllHandle )
   {
   //Load the dll and keep the handle to it
#ifdef WIN32
        oclDllHandle = LoadLibrary(L"OpenCl.dll");
#else
        oclDllHandle = dlopen("libOpenCL.so", RTLD_LAZY);
#endif

   // If the handle is valid, try to get the function address. 
        if (NULL != oclDllHandle) 
        { 
	        PIM_CLICTR_MSG("Loaded OpenCl lib");
      //Get pointers to OCL functions with GET_INTERCEPTOR_FUNCTION_ADDRESS MACRO:
	        ret = pim_GET_CLINTERCEPTOR_FUNCTION_ADDRESSes(oclDllHandle);
         }
         else
         {
	        ret = -100;
	        PIM_CLICTR_MSG("Cannot load OpenCl lib");
         }
   }

   return(ret);
}



/*******************************************************************************************************/

/* Platform API */
CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint          num_entries ,
                 cl_platform_id * platforms ,
                 cl_uint *        num_platforms )
{
cl_int err = -100;
   if ( GetPlatformIDs )
   {
	   err = 
		    GetPlatformIDs(num_entries,
                           platforms,
                           num_platforms);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL 
clGetPlatformInfo(cl_platform_id   platform , 
                  cl_platform_info param_name ,
                  size_t           param_value_size , 
                  void *           param_value ,
                  size_t *         param_value_size_ret )
{
cl_int err = -100;
   if ( GetPlatformInfo )
   {
      err = 
		  GetPlatformInfo(platform, 
                          param_name,
                          param_value_size, 
                          param_value,
                          param_value_size_ret); 
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

/* Device APIs */
CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id   platform ,
               cl_device_type   device_type , 
               cl_uint          num_entries , 
               cl_device_id *   devices , 
               cl_uint *        num_devices )
{
cl_int err = -100;
   if (GetDeviceIDs)
   {
	 err = 
		 GetDeviceIDs(platform ,
                      device_type , 
                      num_entries , 
                      devices , 
                      num_devices);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}


CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id    device ,
                cl_device_info  param_name , 
                size_t          param_value_size , 
                void *          param_value ,
                size_t *        param_value_size_ret )
{
cl_int err = -100;
   if ( GetDeviceInfo )
   {
	   err = 
		   GetDeviceInfo(device ,
                         param_name , 
                         param_value_size , 
                         param_value ,
                         param_value_size_ret);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}
    
CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(cl_device_id                         in_device ,
                   const cl_device_partition_property * properties ,
                   cl_uint                              num_devices ,
                   cl_device_id *                       out_devices ,
                   cl_uint *                            num_devices_ret )
{
cl_int err = CL_SUCCESS;
   if ( CreateSubDevices )
   {
	   err = 
            CreateSubDevices(in_device ,
                             properties ,
                             num_devices ,
                             out_devices ,
                             num_devices_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDevice(cl_device_id device )
{
cl_int err = CL_SUCCESS;
   if ( RetainDevice)
   {
	   err =
		   RetainDevice( device );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
    
CL_API_ENTRY cl_int CL_API_CALL
clReleaseDevice(cl_device_id device )
{
cl_int err = -100;
   if ( ReleaseDevice )
   {
	   err = 
		   ReleaseDevice(device);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}
    
/* Context APIs  */
CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties * properties ,
                cl_uint                 num_devices ,
                const cl_device_id *    devices ,
                void (CL_CALLBACK * pfn_notify )(const char *, const void *, size_t, void *),
                void *                  user_data ,
                cl_int *                errcode_ret )
{
cl_context ret = 0;
   if ( CreateContext )
   {
	   ret = 
		   CreateContext(properties ,
                         num_devices ,
                         devices ,
                         pfn_notify,
                         user_data ,
                         errcode_ret);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(ret);
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContextFromType(const cl_context_properties * properties ,
                        cl_device_type          device_type ,
                        void (CL_CALLBACK *     pfn_notify )(const char *, const void *, size_t, void *),
                        void *                  user_data ,
                        cl_int *                errcode_ret )
{
cl_context ret = 0;
   if ( CreateContextFromType )
   {
	   ret =
		   CreateContextFromType(properties ,
                                 device_type ,
                                 pfn_notify,
                                 user_data ,
                                 errcode_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context )
{
cl_int err = -100;
   if ( RetainContext )
   {
	   err = 
		   RetainContext(context);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context )
{
cl_int err = -100;
   if ( ReleaseContext )
   {
	   err = 
		   ReleaseContext(context);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(cl_context         context , 
                 cl_context_info    param_name , 
                 size_t             param_value_size , 
                 void *             param_value , 
                 size_t *           param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetContextInfo )
   {
	   err =
		   GetContextInfo(context , 
                          param_name , 
                          param_value_size , 
                          param_value , 
                          param_value_size_ret );

   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

/* Command Queue APIs */
CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context                     context , 
                     cl_device_id                   device , 
                     cl_command_queue_properties    properties ,
                     cl_int *                       errcode_ret )
{
cl_command_queue ret = 0;
   if ( CreateCommandQueue )
   {


	   ret = 
		   CreateCommandQueue(context , 
                              device , 
                              properties ,
							  errcode_ret);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue )
{
cl_int err = -100;
   if ( RetainCommandQueue )
   {
	   err = 
		   RetainCommandQueue( command_queue );
   }
   else
   {
       PIM_CLICTR_MSG("NOT INTERCEPTED YET!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue )
{
cl_int err = -100;
   if ( ReleaseCommandQueue )
   {
	   err = 
		   ReleaseCommandQueue( command_queue );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}


CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(cl_command_queue      command_queue ,
                      cl_command_queue_info param_name ,
                      size_t                param_value_size ,
                      void *                param_value ,
                      size_t *              param_value_size_ret )
{
cl_int err = -100;
   if ( GetCommandQueueInfo )
   {
	   err = 
		   GetCommandQueueInfo( command_queue ,
                                param_name ,
                                param_value_size ,
                                param_value ,
                                param_value_size_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

/* Memory Object APIs */
CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context ,
               cl_mem_flags flags ,
               size_t       size ,
               void *       host_ptr ,
               cl_int *     errcode_ret )
{
cl_mem ret = 0;
   if ( CreateBuffer )
   {
	   ret = 
		   CreateBuffer( context ,
                         flags ,
                         size ,
                         host_ptr ,
                         errcode_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(cl_mem                   buffer ,
                  cl_mem_flags             flags ,
                  cl_buffer_create_type    buffer_create_type ,
                  const void *             buffer_create_info ,
                  cl_int *                 errcode_ret )
{
cl_mem ret = 0;
   if ( CreateSubBuffer )
   {
	   ret = 
		   CreateSubBuffer(buffer ,
                           flags ,
                           buffer_create_type ,
                           buffer_create_info ,
                           errcode_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(cl_context              context ,
              cl_mem_flags            flags ,
              const cl_image_format * image_format ,
              const cl_image_desc *   image_desc , 
              void *                  host_ptr ,
              cl_int *                errcode_ret )
{
cl_mem ret = 0;
   if ( CreateImage )
   {
	   ret =
		   CreateImage(context ,
                       flags ,
                       image_format ,
                       image_desc , 
                       host_ptr ,
                       errcode_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}
                        
CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj )
{
cl_int err = -100;
   if ( RetainMemObject )
   {
	   err =
		   RetainMemObject( memobj );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj )
{
cl_int err = -100;
   if ( ReleaseMemObject )
   {
	   err =
		   ReleaseMemObject( memobj );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context           context ,
                           cl_mem_flags         flags ,
                           cl_mem_object_type   image_type ,
                           cl_uint              num_entries ,
                           cl_image_format *    image_formats ,
                           cl_uint *            num_image_formats )
{
cl_int err = CL_SUCCESS;
   if ( GetSupportedImageFormats )
   {
	   err=
		   GetSupportedImageFormats(context ,
                                    flags ,
                                    image_type ,
                                    num_entries ,
                                    image_formats ,
                                    num_image_formats );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                                    
CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem           memobj ,
                   cl_mem_info      param_name , 
                   size_t           param_value_size ,
                   void *           param_value ,
                   size_t *         param_value_size_ret )
{
cl_int err = -100;

   if ( GetMemObjectInfo )
   {
	   err = 
		   GetMemObjectInfo(  memobj ,
                              param_name , 
                              param_value_size ,
                              param_value ,
                              param_value_size_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }   
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem           image ,
               cl_image_info    param_name , 
               size_t           param_value_size ,
               void *           param_value ,
               size_t *         param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetImageInfo )
   {
	   err =
		   GetImageInfo(image ,
                        param_name , 
                        param_value_size ,
                        param_value ,
                        param_value_size_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(  cl_mem  memobj , 
                                    void (CL_CALLBACK * pfn_notify)( cl_mem /* memobj */, void* /*user_data*/), 
                                    void * user_data  )
{
cl_int err = CL_SUCCESS;
   if ( SetMemObjectDestructorCallback )
   {
	   err =
		   SetMemObjectDestructorCallback( memobj , 
                                           pfn_notify, 
                                           user_data  );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

/* Sampler APIs */
CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context           context ,
                cl_bool              normalized_coords , 
                cl_addressing_mode   addressing_mode , 
                cl_filter_mode       filter_mode ,
                cl_int *             errcode_ret )
{
cl_sampler ret = 0;
   if ( CreateSampler)
   {
	   ret =
           CreateSampler(context ,
                         normalized_coords , 
                         addressing_mode , 
                         filter_mode ,
                         errcode_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler  sampler )
{
cl_int err = CL_SUCCESS;
   if ( RetainSampler )
   {
	   err =
		   RetainSampler(sampler );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler  sampler )
{
cl_int err = CL_SUCCESS;
   if ( ReleaseSampler )
   {
	   err =
		   ReleaseSampler( sampler );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler          sampler ,
                 cl_sampler_info     param_name ,
                 size_t              param_value_size ,
                 void *              param_value ,
                 size_t *            param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetSamplerInfo )
   {
	   err =
		   GetSamplerInfo(sampler ,
                          param_name ,
                          param_value_size ,
                          param_value ,
                          param_value_size_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
/* Program Object APIs  */
CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context         context ,
                          cl_uint            count ,
                          const char **      strings ,
                          const size_t *     lengths ,
                          cl_int *           errcode_ret )
{
cl_program ret = 0;
   if ( CreateProgramWithSource )
   {
	   ret = 
		   CreateProgramWithSource( context ,
                                      count ,
                                      strings ,
                                      lengths ,
                                      errcode_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }

   return(ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context                      context ,
                          cl_uint                         num_devices ,
                          const cl_device_id *            device_list ,
                          const size_t *                  lengths ,
                          const unsigned char **          binaries ,
                          cl_int *                        binary_status ,
                          cl_int *                        errcode_ret )
{
cl_program ret = 0;
   if ( CreateProgramWithBinary )
   {
	   ret =
		   CreateProgramWithBinary(context ,
                                   num_devices ,
                                   device_list ,
                                   lengths ,
                                   binaries ,
                                   binary_status ,
                                   errcode_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBuiltInKernels(cl_context             context ,
                                  cl_uint                num_devices ,
                                  const cl_device_id *   device_list ,
                                  const char *           kernel_names ,
                                  cl_int *               errcode_ret )
{
cl_program ret = 0;
   if ( CreateProgramWithBuiltInKernels )
   {
	   ret =
		   CreateProgramWithBuiltInKernels(context ,
                                           num_devices ,
                                           device_list ,
                                           kernel_names ,
                                           errcode_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program  program )
{
cl_int err = -100;
   if (RetainProgram)
   {
	   err = 
		   RetainProgram(program);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program  program )
{
cl_int err = -100;
   if ( ReleaseProgram )
   {
	   err = 
		   ReleaseProgram( program );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options, 
               void (CL_CALLBACK *  pfn_notify)(cl_program /* program */, void * /* user_data */),
               void *               user_data)
{
cl_int err = -100;
   if ( BuildProgram )
   {
#if PIMRT
		  if ( !isOclThread() )
		  {
	         beforeGenericEvent( );
		  }

	   err = 
		   BuildProgram( program,
                         num_devices,
                         device_list,
                         options, 
                         pfn_notify,
                         user_data);
		  if ( !isOclThread() )
		  {
     	     afterGenericEvent( );
		  }

#else

	   err = 
		   BuildProgram( program,
                         num_devices,
                         device_list,
                         options, 
                         pfn_notify,
                         user_data);

#endif
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clCompileProgram(cl_program           program,
                 cl_uint              num_devices,
                 const cl_device_id * device_list,
                 const char *         options, 
                 cl_uint              num_input_headers,
                 const cl_program *   input_headers,
                 const char **        header_include_names,
                 void (CL_CALLBACK *  pfn_notify)(cl_program /* program */, void * /* user_data */),
                 void *               user_data)
{
cl_int err = CL_SUCCESS;
   if ( CompileProgram )
   {
	   err =
		   CompileProgram(program,
                          num_devices,
                          device_list,
                          options, 
                          num_input_headers,
                          input_headers,
                          header_include_names,
                          pfn_notify ,
                          user_data);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_program CL_API_CALL
clLinkProgram(cl_context           context,
              cl_uint              num_devices,
              const cl_device_id * device_list,
              const char *         options, 
              cl_uint              num_input_programs,
              const cl_program *   input_programs,
              void (CL_CALLBACK *  pfn_notify)(cl_program /* program */, void * /* user_data */),
              void *               user_data,
              cl_int *             errcode_ret)
{
cl_program ret = 0;
   if ( LinkProgram )
   {
	   ret =
		   LinkProgram(context,
                       num_devices,
                       device_list,
                       options, 
                       num_input_programs,
                       input_programs,
                       pfn_notify,
                       user_data,
                       errcode_ret);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}



CL_API_ENTRY cl_int CL_API_CALL
clUnloadPlatformCompiler(cl_platform_id  platform )
{
cl_int err = CL_SUCCESS;
   if ( UnloadPlatformCompiler )
   {
	   err =
		   UnloadPlatformCompiler( platform );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program          program ,
                 cl_program_info     param_name ,
                 size_t              param_value_size ,
                 void *              param_value ,
                 size_t *            param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetProgramInfo )
   {
	   err =
		   GetProgramInfo( program ,
                           param_name ,
                           param_value_size ,
                           param_value ,
                           param_value_size_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program             program ,
                      cl_device_id           device ,
                      cl_program_build_info  param_name ,
                      size_t                 param_value_size ,
                      void *                 param_value ,
                      size_t *               param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if( GetProgramBuildInfo)
   {
	   err = 
		   GetProgramBuildInfo(program ,
                               device ,
                               param_name ,
                               param_value_size ,
                               param_value ,
                               param_value_size_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
/* Kernel Object APIs */
CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program       program ,
               const char *     kernel_name ,
               cl_int *         errcode_ret )
{
cl_kernel ret = 0;
   if ( CreateKernel )
   {
	   ret = 
		   CreateKernel( program ,
                         kernel_name ,
                         errcode_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateKernelsInProgram(cl_program      program ,
                         cl_uint         num_kernels ,
                         cl_kernel *     kernels ,
                         cl_uint *       num_kernels_ret )
{
cl_int err = -100;
   if (CreateKernelsInProgram)
   {
	   err =
		   CreateKernelsInProgram(  program ,
                                    num_kernels ,
                                    kernels ,
                                    num_kernels_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel     kernel )
{
cl_int err = -100;
   if ( RetainKernel )
   {
	   err = 
		   RetainKernel(kernel);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel    kernel )
{
cl_int err = -100;
   if ( ReleaseKernel )
   {
	   err =
		   ReleaseKernel( kernel);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel     kernel ,
               cl_uint       arg_index ,
               size_t        arg_size ,
               const void *  arg_value )
{
cl_int err = -100;
   if ( SetKernelArg )
   {
	   err = 
		   SetKernelArg(  kernel ,
                          arg_index ,
                          arg_size ,
                          arg_value );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel        kernel ,
                cl_kernel_info   param_name ,
                size_t           param_value_size ,
                void *           param_value ,
                size_t *         param_value_size_ret )
{
cl_int err = -100;
   if ( GetKernelInfo )
   {
	   err = 
		   GetKernelInfo( kernel ,
                          param_name ,
                          param_value_size ,
                          param_value ,
                          param_value_size_ret );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelArgInfo(cl_kernel        kernel ,
                   cl_uint          arg_indx ,
                   cl_kernel_arg_info   param_name ,
                   size_t           param_value_size ,
                   void *           param_value ,
                   size_t *         param_value_size_ret )
{
cl_int err = -100;
   if ( GetKernelArgInfo )
   {
	   err = 
		   GetKernelArgInfo( kernel ,
                             arg_indx ,
                             param_name ,
                             param_value_size ,
                             param_value ,
                             param_value_size_ret );

   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                         cl_device_id                device ,
                         cl_kernel_work_group_info   param_name ,
                         size_t                      param_value_size ,
                         void *                      param_value ,
                         size_t *                    param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetKernelWorkGroupInfo )
   {
	   err =
		   GetKernelWorkGroupInfo(kernel ,
                                  device ,
                                  param_name ,
                                  param_value_size ,
                                  param_value ,
                                  param_value_size_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }

   return(err);
}


int clInterceptorCaptureOrphanWaitForOurEvents( void )
{
//OCL_EVT_WAIT
int ret = 0;
    if ( isEmuMode() && isCapturing() && isThreadCounting() )
    {

	char msg[1024];
	launchThreadStruct *currlaunchStruct;
	int i;
	int array_sz;
	cl_event *orphans = 0;
 

// extract all orhan events and clear the cache
	    getPimImplicitClEvenArray(&array_sz, &orphans);
		if ( array_sz > 0 )
		{
		    beforeGenericEvent();

		    currlaunchStruct = pim_thread_getlaunchstruct();

	        sprintf(msg, "OCL_EVT_WAIT,\t%u,\t%llu,\t%d",
			            currlaunchStruct->thread_id, currlaunchStruct->call_order, array_sz);

	        for(i = 0; i < array_sz; i++)
		    {
		    
                    sprintf(msg, "%s,\t%llu",
		                    msg, (long long)orphans[i]);

		    	    clReleaseEvent(orphans[i]);

		     }

             strcat(msg, "\n");

		     OnOclWaitEvent(currlaunchStruct,msg);
		}
        if ( orphans ) 
		{
			free(orphans);
		}
	}
	return(ret);
}



void BuildOurPreEvents(
                             cl_command_queue     *command_queue ,
                             cl_uint              num_events ,
                             const cl_event *     event_list,
							 cl_uint  *           num_our_events,
							 cl_event **          our_events)
{


	cl_event impicit_event;
    *num_our_events = num_events;
	*our_events = 0;

    if ( isEmuMode() )
	{
		if ( command_queue && (impicit_event = removePimImplicitClEvent(*command_queue)) != 0)
		{
			(*num_our_events)++;

		}
	
	    if ( (*num_our_events) > 0 )
	   {
		   *our_events = (cl_event*) calloc((*num_our_events), sizeof(cl_event));
		    assert(*our_events);
		   if ( event_list )
		   {
		      memcpy(*our_events, event_list, num_events * sizeof (cl_event));
		   }
// implicit event
		   if ( (*num_our_events) > num_events )
		   {
			  (*our_events)[num_events] = impicit_event;
		   }

	   }
	}
	else
	{
       *num_our_events = num_events;
	   *our_events = (cl_event *)event_list;
	}
}


static
int buildOurEventsToCapture(int *my_num_events,
                            cl_event *my_event_list,
							int num_our_events,
							const cl_event * our_events, 
							int num_events)
{
int i;
		      *my_num_events = 0;
		      for ( i = 0; i < (int)num_events; i++ )
		      {
		
					  if ( removePimClEvent((our_events)[i]) )
					  {
					      my_event_list[(*my_num_events)++] = (our_events)[i];
					  }
					  
				  
			  }

        // implicit event
			  if ( (num_our_events) > num_events )
			  {
				  my_event_list[(*my_num_events)++] = (our_events)[num_events];
			  }
			  return(0);
}

static
int CaptureWaitForOurEvents(
                             cl_command_queue     *command_queue ,
                             cl_uint              num_events ,
                             const cl_event *     event_list,
							 cl_uint  *           num_our_events,
							 cl_event **          our_events)
{
int ret = 0;
launchThreadStruct *currlaunchStruct;
int i;
char msg[1024];
// event filter
// FILTER EVENTS that have not been generated by PIM EMU
int my_num_events;
cl_event my_event_list[1024];


 
   if ( isEmuMode() && !isItRuntimeCall() && !isOclThread())
   {



		beforeGenericEvent();

	    currlaunchStruct = pim_thread_getlaunchstruct();

        BuildOurPreEvents(command_queue , 
		              num_events ,
                      event_list,
		              num_our_events,
					  our_events);
//FIX ME: HAS TO BE PER CONTEXT
        if ( *our_events)
	    {    

	
			 buildOurEventsToCapture(&my_num_events,
                                     my_event_list,
							         (*num_our_events),
							         (const cl_event *) (*our_events), 
							         num_events);

			  if ( my_num_events > 0 && isCapturing() && isThreadCounting())
			  {
//OCL_EVT_WAIT

   
                       sprintf(msg, "OCL_EVT_WAIT,\t%u,\t%llu,\t%d", currlaunchStruct->thread_id, currlaunchStruct->call_order, my_num_events);

     		           for ( i = 0; i < my_num_events; i++ )
	    	           {
		    	          sprintf(msg,"%s,\t%llu", msg, (long long)my_event_list[i]);
		               }

			           strcat(msg,"\n");

					   stackTimeLineMsg(msg);
	
					   ret = 1;

  
			  }
	     }
// if we do not send wait for event message
// send generic end interval message
		if ( ret == 0 && isCapturing() && isThreadCounting())
		{
		     sprintf(msg, "PIMEMU_END,\t%d,\t%llu\n",currlaunchStruct->thread_id, currlaunchStruct->call_order);
			 stackTimeLineMsg(msg);
		}
   }
   else
   {
       *our_events = (cl_event *)event_list;
        *num_our_events = num_events;
   }
   return(ret);
}

static
void afterCaptureOurEvents(  cl_event ** our_event_list,
                            cl_uint our_num_events,
							cl_uint              num_events_in_wait_list)
{
	if ( isEmuMode() && !isItRuntimeCall() & !isOclThread())
    {

		 if ( our_event_list && *our_event_list)
		 {
// implicit event
    		if ( our_num_events > num_events_in_wait_list )
	    	{
			     clReleaseEvent((*our_event_list)[num_events_in_wait_list]);
		    }

			free((*our_event_list));
	
		  }
// generic start - message inside CaptureWaitForOurEvents
		OnOclWaitEvent(NULL, NULL);
    }
}


/* Event Object APIs */
CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(cl_uint              num_events ,
                const cl_event *     event_list )
{
cl_int err = -100;
   if ( WaitForEvents )
   {
#if PIMRT
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
   
	    CaptureWaitForOurEvents(NULL, num_events , event_list,
			                              &our_num_events, &our_event_list);
	
	    err = WaitForEvents(  our_num_events,
                              our_event_list );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events);
#else
	    err = WaitForEvents(  num_events,
                              event_list );

#endif

   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(cl_event          event ,
               cl_event_info     param_name ,
               size_t            param_value_size ,
               void *            param_value ,
               size_t *          param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetEventInfo )
   {
	   err =
		   GetEventInfo( event ,
                         param_name ,
                         param_value_size ,
                         param_value ,
                         param_value_size_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
CL_API_ENTRY cl_event CL_API_CALL
clCreateUserEvent(cl_context     context ,
                  cl_int *       errcode_ret )               
{
cl_event err = 0;
   if ( CreateUserEvent )
   {
	   err =
		   CreateUserEvent(context ,
                           errcode_ret ); 
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event  event )
{
cl_int err = -100;
   if ( RetainEvent )
   {
	   err = 
		   RetainEvent(event);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event  event )
{
cl_int err = -100;
   if ( ReleaseEvent )
   {

	   err = 
		   ReleaseEvent(event);
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetUserEventStatus(cl_event    event ,
                     cl_int      execution_status )
{
cl_int err = CL_SUCCESS;
   if ( SetUserEventStatus )
   {
	   err = 
	       SetUserEventStatus(event ,
                              execution_status );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}
                     
CL_API_ENTRY cl_int CL_API_CALL
clSetEventCallback( cl_event     event ,
                    cl_int       command_exec_callback_type ,
                    void (CL_CALLBACK *  pfn_notify)(cl_event, cl_int, void *),
                    void *       user_data )
{
cl_int err = CL_SUCCESS;
   if ( SetEventCallback )
   {
	   err =
		   SetEventCallback( event ,
                             command_exec_callback_type ,
                             pfn_notify,
                             user_data );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

/* Profiling APIs */
CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event             event ,
                        cl_profiling_info    param_name ,
                        size_t               param_value_size ,
                        void *               param_value ,
                        size_t *             param_value_size_ret )
{
cl_int err = CL_SUCCESS;
   if ( GetEventProfilingInfo )
   {
	   err =
		   GetEventProfilingInfo(event ,
                                 param_name ,
                                 param_value_size ,
                                 param_value ,
                                 param_value_size_ret );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
  return(err);
}
                                
/* Flush and Finish APIs */
CL_API_ENTRY cl_int CL_API_CALL
clFlush(cl_command_queue  command_queue )
{
cl_int err = -100;
   if ( Flush )
   {
	   err = 
		   Flush( command_queue );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue  command_queue )
{
cl_int err = -100;
   if ( Finish )
   {
	   err = 
		   Finish( command_queue );
   }
   else
   {
      PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

/* Enqueued Commands APIs */
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue     command_queue ,
                    cl_mem               buffer ,
                    cl_bool              blocking_read ,
                    size_t               offset ,
                    size_t               size , 
                    void *               ptr ,
                    cl_uint              num_events_in_wait_list ,
                    const cl_event *     event_wait_list ,
                    cl_event *           event )
{
cl_int err = CL_SUCCESS;
   if (EnqueueReadBuffer)
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);



	   err =
             EnqueueReadBuffer(command_queue ,
                               buffer ,
                               blocking_read ,
                               offset ,
                               size , 
                               ptr ,
                               our_num_events ,
                               our_event_list ,
                               event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);		

   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue     command_queue ,
                        cl_mem               buffer ,
                        cl_bool              blocking_read ,
                        const size_t *       buffer_offset ,
                        const size_t *       host_offset , 
                        const size_t *       region ,
                        size_t               buffer_row_pitch ,
                        size_t               buffer_slice_pitch ,
                        size_t               host_row_pitch ,
                        size_t               host_slice_pitch ,                        
                        void *               ptr ,
                        cl_uint              num_events_in_wait_list ,
                        const cl_event *     event_wait_list ,
                        cl_event *           event )
{
cl_int err = CL_SUCCESS;
   if (EnqueueReadBufferRect)
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err = 
            EnqueueReadBufferRect(command_queue ,
                                  buffer ,
                                  blocking_read ,
                                  buffer_offset ,
                                  host_offset , 
                                  region ,
                                  buffer_row_pitch ,
                                  buffer_slice_pitch ,
                                  host_row_pitch ,
                                  host_slice_pitch ,                        
                                  ptr ,
                                  our_num_events ,
                                  our_event_list ,
                                  event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);	

   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue    command_queue , 
                     cl_mem              buffer , 
                     cl_bool             blocking_write , 
                     size_t              offset , 
                     size_t              size , 
                     const void *        ptr , 
                     cl_uint             num_events_in_wait_list , 
                     const cl_event *    event_wait_list , 
                     cl_event *          event )
{
cl_int err = CL_SUCCESS;
   if (EnqueueWriteBuffer)
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err = 
		   EnqueueWriteBuffer(command_queue , 
                              buffer , 
                              blocking_write , 
                              offset , 
                              size , 
                              ptr , 
                              our_num_events , 
                              our_event_list , 
                              event );


		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);	
   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue     command_queue ,
                         cl_mem               buffer ,
                         cl_bool              blocking_write ,
                         const size_t *       buffer_offset ,
                         const size_t *       host_offset , 
                         const size_t *       region ,
                         size_t               buffer_row_pitch ,
                         size_t               buffer_slice_pitch ,
                         size_t               host_row_pitch ,
                         size_t               host_slice_pitch ,                        
                         const void *         ptr ,
                         cl_uint              num_events_in_wait_list ,
                         const cl_event *     event_wait_list ,
                         cl_event *           event )
{
cl_int err = CL_SUCCESS;
   if (EnqueueWriteBufferRect)
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err =
		   EnqueueWriteBufferRect(command_queue ,
                                  buffer ,
                                  blocking_write ,
                                  buffer_offset ,
                                  host_offset , 
                                  region ,
                                  buffer_row_pitch ,
                                  buffer_slice_pitch ,
                                  host_row_pitch ,
                                  host_slice_pitch ,                        
                                  ptr ,
                                  our_num_events ,
                                  our_event_list ,
                                  event );


		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);

   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }

   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue    command_queue ,
                    cl_mem              buffer , 
                    const void *        pattern , 
                    size_t              pattern_size , 
                    size_t              offset , 
                    size_t              size , 
                    cl_uint             num_events_in_wait_list , 
                    const cl_event *    event_wait_list , 
                    cl_event *          event )
{
cl_int err = CL_SUCCESS;
   if (EnqueueFillBuffer)
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err =
		   EnqueueFillBuffer(command_queue ,
                             buffer , 
                             pattern , 
                             pattern_size , 
                             offset , 
                             size , 
                             our_num_events , 
                             our_event_list , 
                             event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);


   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue     command_queue , 
                    cl_mem               src_buffer ,
                    cl_mem               dst_buffer , 
                    size_t               src_offset ,
                    size_t               dst_offset ,
                    size_t               size , 
                    cl_uint              num_events_in_wait_list ,
                    const cl_event *     event_wait_list ,
                    cl_event *           event )
{
cl_int err = CL_SUCCESS;

   if (EnqueueCopyBuffer)
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err =
		   EnqueueCopyBuffer(command_queue , 
                             src_buffer ,
                             dst_buffer , 
                             src_offset ,
                             dst_offset ,
                             size , 
                             our_num_events ,
                             our_event_list ,
                             event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }

   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue     command_queue , 
                        cl_mem               src_buffer ,
                        cl_mem               dst_buffer , 
                        const size_t *       src_origin ,
                        const size_t *       dst_origin ,
                        const size_t *       region , 
                        size_t               src_row_pitch ,
                        size_t               src_slice_pitch ,
                        size_t               dst_row_pitch ,
                        size_t               dst_slice_pitch ,
                        cl_uint              num_events_in_wait_list ,
                        const cl_event *     event_wait_list ,
                        cl_event *           event )
{
cl_int err = CL_SUCCESS;
   if (EnqueueCopyBufferRect)
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err = 
		   EnqueueCopyBufferRect(command_queue , 
                                 src_buffer ,
                                 dst_buffer , 
                                 src_origin ,
                                 dst_origin ,
                                 region , 
                                 src_row_pitch ,
                                 src_slice_pitch ,
                                 dst_row_pitch ,
                                 dst_slice_pitch ,
                                 our_num_events ,
                                 our_event_list ,
                                 event );


		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue      command_queue ,
                   cl_mem                image ,
                   cl_bool               blocking_read , 
                   const size_t          origin[3] ,
                   const size_t          region[3] ,
                   size_t                row_pitch ,
                   size_t                slice_pitch , 
                   void *                ptr ,
                   cl_uint               num_events_in_wait_list ,
                   const cl_event *      event_wait_list ,
                   cl_event *            event )
{
cl_int err = CL_SUCCESS;
 
   if ( EnqueueReadImage )
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err =
		   EnqueueReadImage(command_queue ,
                            image ,
                            blocking_read , 
                            origin ,
                            region ,
                            row_pitch ,
                            slice_pitch , 
                            ptr ,
                            our_num_events ,
                            our_event_list ,
                            event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue     command_queue ,
                    cl_mem               image ,
                    cl_bool              blocking_write , 
                    const size_t         origin[3] ,
                    const size_t         region[3] ,
                    size_t               input_row_pitch ,
                    size_t               input_slice_pitch , 
                    const void *         ptr ,
                    cl_uint              num_events_in_wait_list ,
                    const cl_event *     event_wait_list ,
                    cl_event *           event )
{
cl_int err = CL_SUCCESS;

   if ( EnqueueWriteImage )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueWriteImage(command_queue ,
                             image ,
                             blocking_write , 
                             origin ,
                             region ,
                             input_row_pitch ,
                             input_slice_pitch , 
                             ptr ,
                             our_num_events ,
                             our_event_list ,
                             event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 

   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(cl_command_queue    command_queue ,
                   cl_mem              image , 
                   const void *        fill_color , 
                   const size_t        origin[3] , 
                   const size_t        region[3] , 
                   cl_uint             num_events_in_wait_list , 
                   const cl_event *    event_wait_list , 
                   cl_event *          event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueFillImage )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueFillImage( command_queue ,
                             image , 
                             fill_color , 
                             origin , 
                             region , 
                             our_num_events ,
                             our_event_list ,
                             event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 

   return(err);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue      command_queue ,
                   cl_mem                src_image ,
                   cl_mem                dst_image , 
                   const size_t          src_origin[3] ,
                   const size_t          dst_origin[3] ,
                   const size_t          region[3] , 
                   cl_uint               num_events_in_wait_list ,
                   const cl_event *      event_wait_list ,
                   cl_event *            event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueCopyImage )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueCopyImage(command_queue ,
                            src_image ,
                            dst_image , 
                            src_origin ,
                            dst_origin ,
                            region , 
                            our_num_events ,
                            our_event_list ,
                            event );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 

   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue  command_queue ,
                           cl_mem            src_image ,
                           cl_mem            dst_buffer , 
                           const size_t      src_origin[3] ,
                           const size_t      region[3] , 
                           size_t            dst_offset ,
                           cl_uint           num_events_in_wait_list ,
                           const cl_event *  event_wait_list ,
                           cl_event *        event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueCopyImageToBuffer )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueCopyImageToBuffer(command_queue ,
                                    src_image ,
                                    dst_buffer , 
                                    src_origin ,
                                    region , 
                                    dst_offset ,
                                    our_num_events ,
                                    our_event_list ,
                                    event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 

   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue  command_queue ,
                           cl_mem            src_buffer ,
                           cl_mem            dst_image , 
                           size_t            src_offset ,
                           const size_t      dst_origin[3] ,
                           const size_t      region[3] , 
                           cl_uint           num_events_in_wait_list ,
                           const cl_event *  event_wait_list ,
                           cl_event *        event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueCopyBufferToImage )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueCopyBufferToImage(command_queue ,
                                    src_buffer ,
                                    dst_image , 
                                    src_offset ,
                                    dst_origin ,
                                    region , 
                                    our_num_events ,
                                    our_event_list ,
                                    event );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 

   return(err);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapBuffer(cl_command_queue  command_queue ,
                   cl_mem            buffer ,
                   cl_bool           blocking_map , 
                   cl_map_flags      map_flags ,
                   size_t            offset ,
                   size_t            size ,
                   cl_uint           num_events_in_wait_list ,
                   const cl_event *  event_wait_list ,
                   cl_event *        event ,
                   cl_int *          errcode_ret )
{
void * ret = 0;
   if ( EnqueueMapBuffer )
   {
#if PIMRT
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   ret = 
		   EnqueueMapBuffer( command_queue ,
                             buffer ,
                             blocking_map , 
                             map_flags ,
                             offset ,
                             size ,
                             our_num_events ,
                             our_event_list ,
                             event ,
                             errcode_ret );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);

#else
	   ret = 
		   EnqueueMapBuffer( command_queue ,
                             buffer ,
                             blocking_map , 
                             map_flags ,
                             offset ,
                             size ,
                             num_events_in_wait_list ,
                             event_wait_list ,
                             event ,
                             errcode_ret );

#endif

   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(cl_command_queue   command_queue ,
                  cl_mem             image , 
                  cl_bool            blocking_map , 
                  cl_map_flags       map_flags , 
                  const size_t       origin[3] ,
                  const size_t       region[3] ,
                  size_t *           image_row_pitch ,
                  size_t *           image_slice_pitch ,
                  cl_uint            num_events_in_wait_list ,
                  const cl_event *   event_wait_list ,
                  cl_event *         event ,
                  cl_int *           errcode_ret )
{
void * ret = 0;
   if ( EnqueueMapImage )
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   ret = 
		   EnqueueMapImage(command_queue ,
                           image , 
                           blocking_map , 
                           map_flags , 
                           origin ,
                           region ,
                           image_row_pitch ,
                           image_slice_pitch ,
                           our_num_events ,
                           our_event_list ,
                           event ,
                           errcode_ret );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue  command_queue ,
                        cl_mem            memobj ,
                        void *            mapped_ptr ,
                        cl_uint           num_events_in_wait_list ,
                        const cl_event *   event_wait_list ,
                        cl_event *         event )
{
cl_int err = -100;
   if( EnqueueUnmapMemObject )
   {
#if PIMRT
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err = 
		   EnqueueUnmapMemObject( command_queue ,
                                  memobj ,
                                  mapped_ptr ,
                                  our_num_events ,
                                  our_event_list ,
                                  event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
#else
	   err = 
		   EnqueueUnmapMemObject( command_queue ,
                                  memobj ,
                                  mapped_ptr ,
                                  num_events_in_wait_list ,
                                  event_wait_list ,
                                  event );

#endif
   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue        command_queue ,
                           cl_uint                 num_mem_objects ,
                           const cl_mem *          mem_objects ,
                           cl_mem_migration_flags  flags ,
                           cl_uint                 num_events_in_wait_list ,
                           const cl_event *        event_wait_list ,
                           cl_event *              event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueMigrateMemObjects )
   {
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);

	   err =
		   EnqueueMigrateMemObjects( command_queue ,
                                     num_mem_objects ,
                                     mem_objects ,
                                     flags ,
                                     our_num_events ,
                                     our_event_list ,
                                     event );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

/****************************************************************/
/********** call from interseptor  *****************************/
static
void *kernelLaunchFunc(void * thread_args_)
{
void *ret = 0;
launchOclKernelStruct *ocl_args = (launchOclKernelStruct *)thread_args_; 
cl_int cl_err = CL_SUCCESS;


piminterceptor_clEnqueueNDRangeKernel EnqueueNDRangeKernel = getEnqueueNDRangeKernel();


   cl_err = EnqueueNDRangeKernel(ocl_args->command_queue,
                                  ocl_args->kernel,
                                  ocl_args->work_dim,
                                  ocl_args->global_work_offset,
                                  ocl_args->global_work_size,
                                  ocl_args->local_work_size,
                                  ocl_args->num_events_in_wait_list,
                                  ocl_args->event_wait_list,
                                  ocl_args->event );


  ASSERT_CL_RETURN(cl_err);

   if ( ocl_args->event && ocl_args->event != getPimImplicitClEvent(ocl_args->command_queue) )
   {   
	    addPimClEvent(*ocl_args->event);
   }
   return(ret);
}


int CaptureEnqKernel(launchOclKernelStruct *ocl_args_,
	                 const char * event_nm_,
					 int pim_id_ )
{
   cl_int err = -100;
   launchThreadStruct *currlaunchStruct;
   int i;
   char msg[1024];
   char kernelname[128];
   size_t ret;
   pthread_t thread;
   long long launch_order;
   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;
   const cl_event *     in_event_list = 0;
   cl_uint in_num_events = 0;
   int postevent = 1;
   int my_num_events;
   cl_event my_event_list[1024];

currlaunchStruct = pim_thread_getlaunchstruct();
// stop counters
       incAndStopCnt();

       BuildOurPreEvents(&ocl_args_->command_queue,
		                 ocl_args_->num_events_in_wait_list,
						 ocl_args_->event_wait_list ,
			             &our_num_events, &our_event_list);

	    buildOurEventsToCapture(&my_num_events,
                                     my_event_list,
							         our_num_events,
							         (const cl_event *)our_event_list, 
							         ocl_args_->num_events_in_wait_list);

	   in_num_events = ocl_args_->num_events_in_wait_list;
	   in_event_list = (cl_event *)ocl_args_->event_wait_list;

       ocl_args_->num_events_in_wait_list = our_num_events;
       ocl_args_->event_wait_list = our_event_list;

	    if ( !ocl_args_->event )
		{
         cl_event impilcit_event = 0;
		      postevent = 0;
			  ocl_args_->event = addPimImplicitClEvent(ocl_args_->command_queue, impilcit_event);
	     }

         clGetKernelInfo (	ocl_args_->kernel,
 	                           CL_KERNEL_FUNCTION_NAME,
 	                           127,
							   kernelname,
							   &ret);

// set pim to pass down
		  if ( !strcmp(event_nm_, "OCL_LAUNCH_PIM" ) )
		  {
		      setAboutToLaunchPIMID(pim_id_);
		  }

 


	      pthread_create(&thread, NULL, kernelLaunchFunc, ocl_args_);
	      launch_order= pim_thread_order(); 



		  pthread_join(thread, NULL);

		  OnStopCPUCounters();

   
	
	

	      if ( isCapturing() && isThreadCounting() )
		  {
		       if ( !strcmp(event_nm_, "OCL_LAUNCH" ) )
		       {
                    sprintf(msg, "OCL_LAUNCH,\t%u,\t%llu,\t%s,\t%u,", currlaunchStruct->thread_id,
			                                                  launch_order,
															  kernelname,
															  my_num_events);
		       }
		       else if ( !strcmp(event_nm_, "OCL_LAUNCH_PIM" ) )
		       {

                  sprintf(msg, "OCL_LAUNCH_PIM,\t%u,\t%llu,\t%u,"
			       "\t%s,\t%u,",
			        currlaunchStruct->thread_id, launch_order, pim_id_,
				    kernelname, my_num_events
			      );
		       }


		       for ( i = 0; i < (int)my_num_events; i++ )
		       {
			         sprintf(msg,"%s\t%llu,", msg, (long long)my_event_list[i]);
		       }

		       sprintf(msg,"%s\t%llu\n", msg, (long long)((ocl_args_->event) ? (*ocl_args_->event) : 0));

//			   pimemuTimelineSendString(msg);
		       stackTimeLineMsg(msg);

		  }

		   if ( our_event_list)
		   {
// implicit event
    		    if ( our_num_events > in_num_events )
	    	    {
			        clReleaseEvent(our_event_list[our_num_events-1]);
		        }

			    free(our_event_list);
		    }

           ocl_args_->num_events_in_wait_list = in_num_events;
           ocl_args_->event_wait_list = in_event_list;
		   if ( !postevent )
		   {
			   ocl_args_->event = 0;
		   }


	  		err = CL_SUCCESS;

// start counters
          decAndStartCnt();

		  return (err);
}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue  command_queue ,
                       cl_kernel         kernel ,
                       cl_uint           work_dim ,
                       const size_t *    global_work_offset ,
                       const size_t *    global_work_size ,
                       const size_t *    local_work_size ,
                       cl_uint           num_events_in_wait_list ,
                       const cl_event *  event_wait_list ,
                       cl_event *        event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueNDRangeKernel )
   {
#if PIMRT

        if( isEmuMode() )
		{

			{
        launchOclKernelStruct ocl_args;

	          ocl_args.command_queue =  command_queue;
              ocl_args.kernel = kernel;
              ocl_args.work_dim = work_dim;
              ocl_args.global_work_offset = global_work_offset;
              ocl_args.global_work_size = global_work_size;
              ocl_args.local_work_size = local_work_size;
			  ocl_args.num_events_in_wait_list = num_events_in_wait_list ;
              ocl_args.event_wait_list = event_wait_list;
			  ocl_args.event = event;  


              err = CaptureEnqKernel(&ocl_args,
	                           "OCL_LAUNCH",
					           0);
			}
		}
		else
#endif
		{

	         err =
		         EnqueueNDRangeKernel(  command_queue ,
                                  kernel ,
                                  work_dim ,
                                  global_work_offset ,
                                  global_work_size ,
                                  local_work_size ,
                                  num_events_in_wait_list ,
                                  event_wait_list ,
                                  event );
		}


   }
   else
   {
        PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueTask(cl_command_queue   command_queue ,
              cl_kernel          kernel ,
              cl_uint            num_events_in_wait_list ,
              const cl_event *   event_wait_list ,
              cl_event *         event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueTask )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueTask(command_queue ,
                       kernel ,
                       our_num_events ,
                       our_event_list ,
                       event );

		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(cl_command_queue   command_queue ,
					  void (CL_CALLBACK * user_func)(void *), 
                      void *             args ,
                      size_t             cb_args , 
                      cl_uint            num_mem_objects ,
                      const cl_mem *     mem_list ,
                      const void **      args_mem_loc ,
                      cl_uint            num_events_in_wait_list ,
                      const cl_event *   event_wait_list ,
                      cl_event *         event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueNativeKernel )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueNativeKernel( command_queue ,
					            user_func, 
                                args ,
                                cb_args , 
                                num_mem_objects ,
                                mem_list ,
                                args_mem_loc ,
                                our_num_events ,
                                our_event_list ,
                                event );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   } 
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarkerWithWaitList(cl_command_queue  command_queue ,
                            cl_uint            num_events_in_wait_list ,
                            const cl_event *   event_wait_list ,
                            cl_event *         event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueMarkerWithWaitList )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueMarkerWithWaitList(command_queue ,
                                     our_num_events ,
                                     our_event_list ,
                                     event );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrierWithWaitList(cl_command_queue  command_queue ,
                             cl_uint            num_events_in_wait_list ,
                             const cl_event *   event_wait_list ,
                             cl_event *         event )
{
cl_int err = CL_SUCCESS;
   if ( EnqueueBarrierWithWaitList )
   {

   cl_event *     our_event_list = 0;
   cl_uint our_num_events = 0;

       CaptureWaitForOurEvents(&command_queue,  num_events_in_wait_list , event_wait_list,
			                      &our_num_events, &our_event_list);
	   err =
		   EnqueueBarrierWithWaitList(command_queue ,
                                      our_num_events ,
                                      our_event_list ,
                                      event );
		afterCaptureOurEvents( &our_event_list,
                               our_num_events,
							   num_events_in_wait_list);
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetPrintfCallback(cl_context           context ,
                    void (CL_CALLBACK *  pfn_notify )(cl_context  program , 
                                                          cl_uint printf_data_len , 
                                                          char *  printf_data_ptr , 
                                                          void *  user_data ),
                    void *               user_data )

{
cl_int err = CL_SUCCESS;
   if ( SetPrintfCallback )
   {
	   err =
		   SetPrintfCallback(context ,
                             pfn_notify,
                             user_data );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(err);
}


/* Extension function access */
/*
 * Returns the extension function address for the given function name,
 * or NULL if a valid function can not be found.  The client must
 * check to make sure the address is not NULL, before using or 
 * calling the returned function address.
 */

CL_API_ENTRY void * CL_API_CALL 
clGetExtensionFunctionAddressForPlatform(cl_platform_id  platform ,
                                         const char *    func_name )
{
void * ret = 0;
   if ( GetExtensionFunctionAddressForPlatform )
   {
	   ret = 
		   GetExtensionFunctionAddressForPlatform(platform ,
                                                  func_name );
   }
   else
   {
       PIM_CLICTR_MSG("NOT FOUND!");
   }
   return(ret);
}
    


/***************************************************************************************/
/*************** external interface to obtain intercepting functions *******************/

GET_INTERCEPTOR_FUNCTION(EnqueueNDRangeKernel)

GET_INTERCEPTOR_FUNCTION( WaitForEvents )
