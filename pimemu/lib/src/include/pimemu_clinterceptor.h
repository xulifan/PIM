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

#ifndef __PIMEMU_CLINTERCEPTOR_H
#define __PIMEMU_CLINTERCEPTOR_H


#ifndef WIN32
#include <dlfcn.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif



/* Platform API */
typedef CL_API_ENTRY cl_int 
	(CL_API_CALL * piminterceptor_clGetPlatformIDs)(cl_uint          /* num_entries */,
                                            cl_platform_id * /* platforms */,
                                            cl_uint *        /* num_platforms */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetPlatformInfo)(cl_platform_id   /* platform */, 
                                            cl_platform_info /* param_name */,
                                            size_t           /* param_value_size */, 
                                            void *           /* param_value */,
                                            size_t *         /* param_value_size_ret */);

/* Device APIs */
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetDeviceIDs)(cl_platform_id   /* platform */,
                                          cl_device_type   /* device_type */, 
                                          cl_uint          /* num_entries */, 
                                          cl_device_id *   /* devices */, 
                                          cl_uint *        /* num_devices */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetDeviceInfo)(cl_device_id    /* device */,
                                          cl_device_info  /* param_name */, 
                                          size_t          /* param_value_size */, 
                                          void *          /* param_value */,
                                          size_t *        /* param_value_size_ret */);
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clCreateSubDevices)(cl_device_id                         in_device ,
                   const cl_device_partition_property * properties ,
                   cl_uint                              num_devices ,
                   cl_device_id *                       out_devices ,
                   cl_uint *                            num_devices_ret );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clRetainDevice)(cl_device_id device );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clReleaseDevice)(cl_device_id /* device */);

/* Context APIs  */
typedef CL_API_ENTRY cl_context (CL_API_CALL * piminterceptor_clCreateContext)(const cl_context_properties * /* properties */,
                cl_uint                 /* num_devices */,
                const cl_device_id *    /* devices */,
                void (CL_CALLBACK * /* pfn_notify */)(const char *, const void *, size_t, void *),
                void *                  /* user_data */,
                cl_int *                /* errcode_ret */);

typedef CL_API_ENTRY cl_context 
(CL_API_CALL * piminterceptor_clCreateContextFromType)(const cl_context_properties * /* properties */,
                        cl_device_type          /* device_type */,
                        void (CL_CALLBACK *     /* pfn_notify*/ )(const char *, const void *, size_t, void *),
                        void *                  /* user_data */,
                        cl_int *                /* errcode_ret */);

typedef CL_API_ENTRY cl_int 
(CL_API_CALL * piminterceptor_clRetainContext)(cl_context /* context */);

typedef CL_API_ENTRY cl_int 
(CL_API_CALL * piminterceptor_clReleaseContext)(cl_context /* context */);

typedef CL_API_ENTRY cl_int 
(CL_API_CALL * piminterceptor_clGetContextInfo)(cl_context         /* context */, 
                 cl_context_info    /* param_name */, 
                 size_t             /* param_value_size */, 
                 void *             /* param_value */, 
                 size_t *           /* param_value_size_ret */);

/* Command Queue APIs */
typedef CL_API_ENTRY cl_command_queue 
(CL_API_CALL * piminterceptor_clCreateCommandQueue)(cl_context                     /* context */, 
                     cl_device_id                   /* device */, 
                     cl_command_queue_properties    /* properties */,
                     cl_int *                       /* errcode_ret */);


typedef CL_API_ENTRY cl_int 
(CL_API_CALL * piminterceptor_clRetainCommandQueue)(cl_command_queue /* command_queue */);

typedef CL_API_ENTRY cl_int 
(CL_API_CALL * piminterceptor_clReleaseCommandQueue)(cl_command_queue /* command_queue */);

typedef CL_API_ENTRY cl_int 
(CL_API_CALL * piminterceptor_clGetCommandQueueInfo)(cl_command_queue      /* command_queue */,
                      cl_command_queue_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */);

/* Memory Object APIs */
typedef CL_API_ENTRY cl_mem 
(CL_API_CALL * piminterceptor_clCreateBuffer)(cl_context   /* context */,
               cl_mem_flags /* flags */,
               size_t       /* size */,
               void *       /* host_ptr */,
               cl_int *     /* errcode_ret */);

typedef CL_API_ENTRY cl_mem
(CL_API_CALL * piminterceptor_clCreateSubBuffer)(cl_mem                   buffer ,
                  cl_mem_flags             flags ,
                  cl_buffer_create_type    buffer_create_type ,
                  const void *             buffer_create_info ,
                  cl_int *                 errcode_ret );

typedef CL_API_ENTRY cl_mem
(CL_API_CALL * piminterceptor_clCreateImage)(cl_context              context ,
              cl_mem_flags            flags ,
              const cl_image_format * image_format ,
              const cl_image_desc *   image_desc , 
              void *                  host_ptr ,
              cl_int *                errcode_ret );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clRetainMemObject)(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clReleaseMemObject)(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0;


typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetMemObjectInfo)(cl_mem           /* memobj */,
                   cl_mem_info      /* param_name */, 
                   size_t           /* param_value_size */,
                   void *           /* param_value */,
                   size_t *         /* param_value_size_ret */);


typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetSupportedImageFormats)(cl_context           context ,
                           cl_mem_flags         flags ,
                           cl_mem_object_type   image_type ,
                           cl_uint              num_entries ,
                           cl_image_format *    image_formats ,
                           cl_uint *            num_image_formats );



typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetImageInfo)(cl_mem           image ,
               cl_image_info    param_name , 
               size_t           param_value_size ,
               void *           param_value ,
               size_t *         param_value_size_ret );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clSetMemObjectDestructorCallback)(  cl_mem  memobj , 
                                    void (CL_CALLBACK * pfn_notify)( cl_mem /* memobj */, void* /*user_data*/), 
                                    void * user_data  );

/* Sampler APIs */
typedef CL_API_ENTRY cl_sampler
(CL_API_CALL * piminterceptor_clCreateSampler)(cl_context           context ,
                cl_bool              normalized_coords , 
                cl_addressing_mode   addressing_mode , 
                cl_filter_mode       filter_mode ,
                cl_int *             errcode_ret );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clRetainSampler)(cl_sampler  sampler );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clReleaseSampler)(cl_sampler  sampler );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetSamplerInfo)(cl_sampler          sampler ,
                 cl_sampler_info     param_name ,
                 size_t              param_value_size ,
                 void *              param_value ,
                 size_t *            param_value_size_ret );

/* Program Object APIs  */
typedef CL_API_ENTRY cl_program
(CL_API_CALL * piminterceptor_clCreateProgramWithSource)(cl_context        /* context */,
                          cl_uint           /* count */,
                          const char **     /* strings */,
                          const size_t *    /* lengths */,
                          cl_int *          /* errcode_ret */);

typedef CL_API_ENTRY cl_program
(CL_API_CALL * piminterceptor_clCreateProgramWithBinary)(cl_context                     /* context */,
                          cl_uint                        /* num_devices */,
                          const cl_device_id *           /* device_list */,
                          const size_t *                 /* lengths */,
                          const unsigned char **         /* binaries */,
                          cl_int *                       /* binary_status */,
                          cl_int *                       /* errcode_ret */);

typedef CL_API_ENTRY cl_program
(CL_API_CALL * piminterceptor_clCreateProgramWithBuiltInKernels)(cl_context            /* context */,
                                  cl_uint               /* num_devices */,
                                  const cl_device_id *  /* device_list */,
                                  const char *          /* kernel_names */,
                                  cl_int *              /* errcode_ret */) ;

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clRetainProgram)(cl_program /* program */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clReleaseProgram)(cl_program /* program */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clBuildProgram)(cl_program           /* program */,
               cl_uint              /* num_devices */,
               const cl_device_id * /* device_list */,
               const char *         /* options */, 
               void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
               void *               /* user_data */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clCompileProgram)(cl_program           /* program */,
                 cl_uint              /* num_devices */,
                 const cl_device_id * /* device_list */,
                 const char *         /* options */, 
                 cl_uint              /* num_input_headers */,
                 const cl_program *   /* input_headers */,
                 const char **        /* header_include_names */,
                 void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                 void *               /* user_data */);

typedef CL_API_ENTRY cl_program
(CL_API_CALL * piminterceptor_clLinkProgram)(cl_context           /* context */,
              cl_uint              /* num_devices */,
              const cl_device_id * /* device_list */,
              const char *         /* options */, 
              cl_uint              /* num_input_programs */,
              const cl_program *   /* input_programs */,
              void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
              void *               /* user_data */,
              cl_int *             /* errcode_ret */ );


typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clUnloadPlatformCompiler)(cl_platform_id /* platform */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetProgramInfo)(cl_program         /* program */,
                 cl_program_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetProgramBuildInfo)(cl_program            /* program */,
                      cl_device_id          /* device */,
                      cl_program_build_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */);
                            
/* Kernel Object APIs */
typedef CL_API_ENTRY cl_kernel
(CL_API_CALL * piminterceptor_clCreateKernel)(cl_program      /* program */,
               const char *    /* kernel_name */,
               cl_int *        /* errcode_ret */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clCreateKernelsInProgram)(cl_program     /* program */,
                         cl_uint        /* num_kernels */,
                         cl_kernel *    /* kernels */,
                         cl_uint *      /* num_kernels_ret */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clRetainKernel)(cl_kernel    /* kernel */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clReleaseKernel)(cl_kernel   /* kernel */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clSetKernelArg)(cl_kernel    /* kernel */,
               cl_uint      /* arg_index */,
               size_t       /* arg_size */,
               const void * /* arg_value */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetKernelInfo)(cl_kernel       /* kernel */,
                cl_kernel_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetKernelArgInfo)(cl_kernel       /* kernel */,
                   cl_uint         /* arg_indx */,
                   cl_kernel_arg_info  /* param_name */,
                   size_t          /* param_value_size */,
                   void *          /* param_value */,
                   size_t *        /* param_value_size_ret */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetKernelWorkGroupInfo)(cl_kernel                  /* kernel */,
                         cl_device_id               /* device */,
                         cl_kernel_work_group_info  /* param_name */,
                         size_t                     /* param_value_size */,
                         void *                     /* param_value */,
                         size_t *                   /* param_value_size_ret */);

/* Event Object APIs */
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clWaitForEvents)(cl_uint             /* num_events */,
                const cl_event *    /* event_list */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetEventInfo)(cl_event         /* event */,
               cl_event_info    /* param_name */,
               size_t           /* param_value_size */,
               void *           /* param_value */,
               size_t *         /* param_value_size_ret */);
                            
typedef CL_API_ENTRY cl_event
(CL_API_CALL * piminterceptor_clCreateUserEvent)(cl_context    /* context */,
                  cl_int *      /* errcode_ret */);               
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clRetainEvent)(cl_event /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clReleaseEvent)(cl_event /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clSetUserEventStatus)(cl_event   /* event */,
                     cl_int     /* execution_status */);
                     
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clSetEventCallback)( cl_event    /* event */,
                    cl_int      /* command_exec_callback_type */,
                    void (CL_CALLBACK * /* pfn_notify */)(cl_event, cl_int, void *),
                    void *      /* user_data */);

/* Profiling APIs */
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clGetEventProfilingInfo)(cl_event            /* event */,
                        cl_profiling_info   /* param_name */,
                        size_t              /* param_value_size */,
                        void *              /* param_value */,
                        size_t *            /* param_value_size_ret */);
                                
/* Flush and Finish APIs */
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clFlush)(cl_command_queue /* command_queue */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clFinish)(cl_command_queue /* command_queue */);


/* Enqueued Commands APIs */
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueReadBuffer)(cl_command_queue    /* command_queue */,
                    cl_mem              /* buffer */,
                    cl_bool             /* blocking_read */,
                    size_t              /* offset */,
                    size_t              /* size */, 
                    void *              /* ptr */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */);
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueReadBufferRect)(cl_command_queue    /* command_queue */,
                        cl_mem              /* buffer */,
                        cl_bool             /* blocking_read */,
                        const size_t *      /* buffer_offset */,
                        const size_t *      /* host_offset */, 
                        const size_t *      /* region */,
                        size_t              /* buffer_row_pitch */,
                        size_t              /* buffer_slice_pitch */,
                        size_t              /* host_row_pitch */,
                        size_t              /* host_slice_pitch */,                        
                        void *              /* ptr */,
                        cl_uint             /* num_events_in_wait_list */,
                        const cl_event *    /* event_wait_list */,
                        cl_event *          /* event */);
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueWriteBuffer)(cl_command_queue   /* command_queue */, 
                     cl_mem             /* buffer */, 
                     cl_bool            /* blocking_write */, 
                     size_t             /* offset */, 
                     size_t             /* size */, 
                     const void *       /* ptr */, 
                     cl_uint            /* num_events_in_wait_list */, 
                     const cl_event *   /* event_wait_list */, 
                     cl_event *         /* event */);
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueWriteBufferRect)(cl_command_queue    /* command_queue */,
                         cl_mem              /* buffer */,
                         cl_bool             /* blocking_write */,
                         const size_t *      /* buffer_offset */,
                         const size_t *      /* host_offset */, 
                         const size_t *      /* region */,
                         size_t              /* buffer_row_pitch */,
                         size_t              /* buffer_slice_pitch */,
                         size_t              /* host_row_pitch */,
                         size_t              /* host_slice_pitch */,                        
                         const void *        /* ptr */,
                         cl_uint             /* num_events_in_wait_list */,
                         const cl_event *    /* event_wait_list */,
                         cl_event *          /* event */);
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueFillBuffer)(cl_command_queue   /* command_queue */,
                    cl_mem             /* buffer */, 
                    const void *       /* pattern */, 
                    size_t             /* pattern_size */, 
                    size_t             /* offset */, 
                    size_t             /* size */, 
                    cl_uint            /* num_events_in_wait_list */, 
                    const cl_event *   /* event_wait_list */, 
                    cl_event *         /* event */);
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueCopyBuffer)(cl_command_queue    /* command_queue */, 
                    cl_mem              /* src_buffer */,
                    cl_mem              /* dst_buffer */, 
                    size_t              /* src_offset */,
                    size_t              /* dst_offset */,
                    size_t              /* size */, 
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */);
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueCopyBufferRect)(cl_command_queue    /* command_queue */, 
                        cl_mem              /* src_buffer */,
                        cl_mem              /* dst_buffer */, 
                        const size_t *      /* src_origin */,
                        const size_t *      /* dst_origin */,
                        const size_t *      /* region */, 
                        size_t              /* src_row_pitch */,
                        size_t              /* src_slice_pitch */,
                        size_t              /* dst_row_pitch */,
                        size_t              /* dst_slice_pitch */,
                        cl_uint             /* num_events_in_wait_list */,
                        const cl_event *    /* event_wait_list */,
                        cl_event *          /* event */) ;
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueReadImage)(cl_command_queue     /* command_queue */,
                   cl_mem               /* image */,
                   cl_bool              /* blocking_read */, 
                   const size_t *       /* origin[3] */,
                   const size_t *       /* region[3] */,
                   size_t               /* row_pitch */,
                   size_t               /* slice_pitch */, 
                   void *               /* ptr */,
                   cl_uint              /* num_events_in_wait_list */,
                   const cl_event *     /* event_wait_list */,
                   cl_event *           /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueWriteImage)(cl_command_queue    /* command_queue */,
                    cl_mem              /* image */,
                    cl_bool             /* blocking_write */, 
                    const size_t *      /* origin[3] */,
                    const size_t *      /* region[3] */,
                    size_t              /* input_row_pitch */,
                    size_t              /* input_slice_pitch */, 
                    const void *        /* ptr */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueFillImage)(cl_command_queue   /* command_queue */,
                   cl_mem             /* image */, 
                   const void *       /* fill_color */, 
                   const size_t *     /* origin[3] */, 
                   const size_t *     /* region[3] */, 
                   cl_uint            /* num_events_in_wait_list */, 
                   const cl_event *   /* event_wait_list */, 
                   cl_event *         /* event */) ;
                            
typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueCopyImage)(cl_command_queue     /* command_queue */,
                   cl_mem               /* src_image */,
                   cl_mem               /* dst_image */, 
                   const size_t *       /* src_origin[3] */,
                   const size_t *       /* dst_origin[3] */,
                   const size_t *       /* region[3] */, 
                   cl_uint              /* num_events_in_wait_list */,
                   const cl_event *     /* event_wait_list */,
                   cl_event *           /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueCopyImageToBuffer)(cl_command_queue /* command_queue */,
                           cl_mem           /* src_image */,
                           cl_mem           /* dst_buffer */, 
                           const size_t *   /* src_origin[3] */,
                           const size_t *   /* region[3] */, 
                           size_t           /* dst_offset */,
                           cl_uint          /* num_events_in_wait_list */,
                           const cl_event * /* event_wait_list */,
                           cl_event *       /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueCopyBufferToImage)(cl_command_queue /* command_queue */,
                           cl_mem           /* src_buffer */,
                           cl_mem           /* dst_image */, 
                           size_t           /* src_offset */,
                           const size_t *   /* dst_origin[3] */,
                           const size_t *   /* region[3] */, 
                           cl_uint          /* num_events_in_wait_list */,
                           const cl_event * /* event_wait_list */,
                           cl_event *       /* event */);

typedef CL_API_ENTRY void *
(CL_API_CALL * piminterceptor_clEnqueueMapBuffer)(cl_command_queue /* command_queue */,
                   cl_mem           /* buffer */,
                   cl_bool          /* blocking_map */, 
                   cl_map_flags     /* map_flags */,
                   size_t           /* offset */,
                   size_t           /* size */,
                   cl_uint          /* num_events_in_wait_list */,
                   const cl_event * /* event_wait_list */,
                   cl_event *       /* event */,
                   cl_int *         /* errcode_ret */);

typedef CL_API_ENTRY void *
(CL_API_CALL * piminterceptor_clEnqueueMapImage)(cl_command_queue  /* command_queue */,
                  cl_mem            /* image */, 
                  cl_bool           /* blocking_map */, 
                  cl_map_flags      /* map_flags */, 
                  const size_t *    /* origin[3] */,
                  const size_t *    /* region[3] */,
                  size_t *          /* image_row_pitch */,
                  size_t *          /* image_slice_pitch */,
                  cl_uint           /* num_events_in_wait_list */,
                  const cl_event *  /* event_wait_list */,
                  cl_event *        /* event */,
                  cl_int *          /* errcode_ret */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueUnmapMemObject)(cl_command_queue /* command_queue */,
                        cl_mem           /* memobj */,
                        void *           /* mapped_ptr */,
                        cl_uint          /* num_events_in_wait_list */,
                        const cl_event *  /* event_wait_list */,
                        cl_event *        /* event */);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueMigrateMemObjects)(cl_command_queue       /* command_queue */,
                           cl_uint                /* num_mem_objects */,
                           const cl_mem *         /* mem_objects */,
                           cl_mem_migration_flags /* flags */,
                           cl_uint                /* num_events_in_wait_list */,
                           const cl_event *       /* event_wait_list */,
                           cl_event *             /* event */) ;

typedef CL_API_ENTRY cl_int
	(CL_API_CALL * piminterceptor_clEnqueueNDRangeKernel)(cl_command_queue /* command_queue */,
                       cl_kernel        /* kernel */,
                       cl_uint          /* work_dim */,
                       const size_t *   /* global_work_offset */,
                       const size_t *   /* global_work_size */,
                       const size_t *   /* local_work_size */,
                       cl_uint          /* num_events_in_wait_list */,
                       const cl_event * /* event_wait_list */,
                       cl_event *       /* event */);



typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueTask)(cl_command_queue   command_queue ,
              cl_kernel          kernel ,
              cl_uint            num_events_in_wait_list ,
              const cl_event *   event_wait_list ,
              cl_event *         event );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueNativeKernel)(cl_command_queue   command_queue ,
					  void (CL_CALLBACK * user_func)(void *), 
                      void *             args ,
                      size_t             cb_args , 
                      cl_uint            num_mem_objects ,
                      const cl_mem *     mem_list ,
                      const void **      args_mem_loc ,
                      cl_uint            num_events_in_wait_list ,
                      const cl_event *   event_wait_list ,
                      cl_event *         event );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueMarkerWithWaitList)(cl_command_queue  command_queue ,
                            cl_uint            num_events_in_wait_list ,
                            const cl_event *   event_wait_list ,
                            cl_event *         event );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clEnqueueBarrierWithWaitList)(cl_command_queue  command_queue ,
                             cl_uint            num_events_in_wait_list ,
                             const cl_event *   event_wait_list ,
                             cl_event *         event );

typedef CL_API_ENTRY cl_int
(CL_API_CALL * piminterceptor_clSetPrintfCallback)(cl_context           context ,
                    void (CL_CALLBACK *  pfn_notify )(cl_context  program , 
                                                          cl_uint printf_data_len , 
                                                          char *  printf_data_ptr , 
                                                          void *  user_data ),
                    void *               user_data );

/* Extension function access */
/*
 * Returns the extension function address for the given function name,
 * or NULL if a valid function can not be found.  The client must
 * check to make sure the address is not NULL, before using or 
 * calling the returned function address.
 */

typedef CL_API_ENTRY void *
(CL_API_CALL * piminterceptor_clGetExtensionFunctionAddressForPlatform)(cl_platform_id  platform ,
                                         const char *    func_name );



#define DECLARE_GET_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
piminterceptor_cl##_FUNCNAME_ get##_FUNCNAME_( void);



/*********************************************************************************
 ******************** interface *************************************************/

int pim_cliterceptorInit( void );


//piminterceptor_clEnqueueNDRangeKernel * getEnqueueNDRangeKernel( void);
DECLARE_GET_INTERCEPTOR_FUNCTION( EnqueueNDRangeKernel );
DECLARE_GET_INTERCEPTOR_FUNCTION( WaitForEvents );

/*********************************************************************************
 ******************** interbal interface *************************************************/

#define ASSERT_CL_RETURN( ret )\
   if( (ret) != CL_SUCCESS )\
   {\
      fprintf( stderr, "%s:%d: error: %s\n", \
             __FILE__, __LINE__, cluErrorString( (ret) ));\
      exit(-1);\
   }

#define  PIMRT 1

const char *cluErrorString(cl_int);

void BuildOurPreEvents(
                             cl_command_queue     *command_queue ,
                             cl_uint              num_events ,
                             const cl_event *     event_list,
							 cl_uint  *           num_our_events,
							 cl_event **          our_events);

// capture orthans
int clInterceptorCaptureOrphanWaitForOurEvents( void );

// capture EnqKerl calls
int CaptureEnqKernel(launchOclKernelStruct *ocl_args_,
	                 const char * event_nm_,
					 int pim_id_ );
int beforeGenericEvent( void);
int afterGenericEvent( void);

#ifdef __cplusplus
}
#endif

#endif  /* __OPENCL_CL_H */

