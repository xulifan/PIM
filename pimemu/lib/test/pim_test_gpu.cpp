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

#include "pim_test_gpu.hpp"

/*****************************************************************************/
/************************* OCL Functions *********************************/
/*****************************************************************************/

const char *getOpenCLErrorString(cl_int err) {

   switch(err) {

      case CL_SUCCESS: return "CL_SUCCESS";
      case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
      case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
      case CL_COMPILER_NOT_AVAILABLE: return
                                       "CL_COMPILER_NOT_AVAILABLE";
      case CL_MEM_OBJECT_ALLOCATION_FAILURE: return
                                       "CL_MEM_OBJECT_ALLOCATION_FAILURE";
      case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
      case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
      case CL_PROFILING_INFO_NOT_AVAILABLE: return
                                       "CL_PROFILING_INFO_NOT_AVAILABLE";
      case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
      case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
      case CL_IMAGE_FORMAT_NOT_SUPPORTED: return
                                       "CL_IMAGE_FORMAT_NOT_SUPPORTED";
      case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
      case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
      case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
      case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
      case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM";
      case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
      case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
      case CL_INVALID_QUEUE_PROPERTIES: return "CL_INVALID_QUEUE_PROPERTIES";
      case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
      case CL_INVALID_HOST_PTR: return "CL_INVALID_HOST_PTR";
      case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
      case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return
                                       "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
      case CL_INVALID_IMAGE_SIZE: return "CL_INVALID_IMAGE_SIZE";
      case CL_INVALID_SAMPLER: return "CL_INVALID_SAMPLER";
      case CL_INVALID_BINARY: return "CL_INVALID_BINARY";
      case CL_INVALID_BUILD_OPTIONS: return "CL_INVALID_BUILD_OPTIONS";
      case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
      case CL_INVALID_PROGRAM_EXECUTABLE: return
                                       "CL_INVALID_PROGRAM_EXECUTABLE";
      case CL_INVALID_KERNEL_NAME: return "CL_INVALID_KERNEL_NAME";
      case CL_INVALID_KERNEL_DEFINITION: return "CL_INVALID_KERNEL_DEFINITION";
      case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
      case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
      case CL_INVALID_ARG_VALUE: return "CL_INVALID_ARG_VALUE";
      case CL_INVALID_ARG_SIZE: return "CL_INVALID_ARG_SIZE";
      case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
      case CL_INVALID_WORK_DIMENSION: return "CL_INVALID_WORK_DIMENSION";
      case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
      case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
      case CL_INVALID_GLOBAL_OFFSET: return "CL_INVALID_GLOBAL_OFFSET";
      case CL_INVALID_EVENT_WAIT_LIST: return "CL_INVALID_EVENT_WAIT_LIST";
      case CL_INVALID_EVENT: return "CL_INVALID_EVENT";
      case CL_INVALID_OPERATION: return "CL_INVALID_OPERATION";
      case CL_INVALID_GL_OBJECT: return "CL_INVALID_GL_OBJECT";
      case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
      case CL_INVALID_MIP_LEVEL: return "CL_INVALID_MIP_LEVEL";
      case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";

      default: return "UNKNOWN CL ERROR CODE";
   }
}




cl_int initOcl( cl_context *context_, cl_device_id *device_  ) {
	
cl_uint plat_count = 0;
cl_int clerr = CL_SUCCESS;
cl_platform_id clplatform;
//cl_device_id cldevice;
//cl_context clcontext;
cl_platform_id platforms[_MAX_PLATFORMS_];
cl_device_type	device_type = CL_DEVICE_TYPE_GPU;
cl_uint device_count = 0;
cl_device_id devices[_MAX_DEVICES_];


	printf("OpenCL: Initializing Interface...\n");

	clerr = clGetPlatformIDs(0, NULL, &plat_count);
	
	if(clerr == CL_SUCCESS) {
		printf("OpenCL: Found %u platforms.\n", plat_count);
	} else {
		printf("OpenCL: Unable to find any OpenCL compatible platforms.\n");
		printf("OpenCL: %s\n", getOpenCLErrorString(clerr));
		return (clerr);
	}
	

	clerr = clGetPlatformIDs(plat_count, platforms, NULL);
	clplatform = platforms[0];
	
	//////////////////////////////////////////////////////////////////////////////////
	
	printf("OpenCL: Querying for all OpenCL compatible GPU devices...\n");

	clerr = clGetDeviceIDs(clplatform, device_type, 0, NULL, &device_count);
	
	if(clerr == CL_SUCCESS) {
		printf("OpenCL: Found %u OpenCL devices.\n", device_count);
	} else {
		printf("OpenCL: Device query on OpenCL platform failed.\n");
		printf("OpenCL: %s\n", getOpenCLErrorString(clerr));
		return (clerr);
	}
	
	clerr = clGetDeviceIDs(clplatform, device_type, device_count, devices, NULL);
	
	*device_ = devices[0];
	
	//////////////////////////////////////////////////////////////////////////////////
	*context_ = clCreateContext( NULL, 1, device_, NULL, NULL, &clerr);
	
	if(clerr == CL_SUCCESS) {
		printf("OpenCL: Successfully established OpenCL context.\n");
	} else {
		printf("OpenCL: Unable to create a context on the selected device.\n");
		printf("OpenCL: %s\n", getOpenCLErrorString(clerr));
		return (clerr);

	}
	return (clerr);
	
}


cl_int getDevicePropoery(void *queue_, cl_device_id device_, int property_id_, int property_ln_)
{
cl_int clerr = CL_SUCCESS;
size_t tmp;
   clerr = clGetDeviceInfo( device_, property_id_, property_ln_, queue_, &tmp);
   return(clerr);
}


cl_int createOclQueue(cl_command_queue *queue_, cl_context context_, cl_device_id device_)
{
cl_int clerr = CL_SUCCESS;
	//////////////////////////////////////////////////////////////////////////////////
	
   *queue_ = clCreateCommandQueue(context_, device_, 0, &clerr);
	
	if(clerr == CL_SUCCESS) {
		printf("OpenCL: Successfully established OpenCL command queue.\n");
	} else {
		printf("OpenCL: Unable to create an OpenCL command queue for device.\n");
		printf("OpenCL: %s\n", getOpenCLErrorString(clerr));
	}
	return (clerr);
}



cl_int compileOclKernels(cl_program *program_, cl_context context_, cl_device_id device_, const char *file_name_, const char *options_)
{

	cl_int clerr = CL_SUCCESS;
	
//	cl_program clprogram;
	//////////////////////////////////////////////////////////////////////////////////
	
	FILE *fp;
	size_t len[1];
	char **buf;

	char* compileOptions;


	//////////////////////////////////////////////////////////////////////////////////
	
	fp=fopen(file_name_,"rb");
	if ( !fp )
	{
		printf("OpenCL: ABORT, cannot read file %s\n", file_name_);
		exit(0);
	}
	fseek(fp,0,SEEK_END);
	len[0] = ftell(fp) ;
	fseek(fp,0,SEEK_SET);
	buf    = (char**) malloc(sizeof(char*) * 1);
	buf[0] = (char *) malloc(sizeof(char)  * (len[0] + 1)); 
	fread(buf[0],len[0],1,fp); 
	fclose(fp);
	buf[0][len[0]] = 0;


	
	//////////////////////////////////////////////////////////////////////////////////
	
	printf("OpenCL: Building OpenCL program and kernels...\n");
	
	compileOptions = (char*) malloc(sizeof(char) * 65536);
	compileOptions[0] = '\0';
    if ( options_ )
	{
		strcpy(compileOptions, options_);
	}
	
	compileOptions = strcat(compileOptions, " -D __GPU__");

	printf("OpenCL: Compiling program %s\nwith options: %s\n", file_name_, compileOptions);
	
	*program_ = clCreateProgramWithSource(context_, 1, (const char**) buf, len, &clerr);
	clerr   = clBuildProgram(*program_, 1, &device_, compileOptions, NULL, NULL);
	if(clerr == CL_SUCCESS) {
		printf("OpenCL: Build program call completed with success.\n");
	} else 
	{

	char programBuildLog[65536];
	programBuildLog[0] = '\0';

		printf("OpenCL: OpenCL program build failed.\n");
		printf("OpenCL: %s\n", getOpenCLErrorString(clerr));
	      clGetProgramBuildInfo(*program_, device_, CL_PROGRAM_BUILD_LOG, 65536, 
							(void*) programBuildLog, NULL);
	
	      printf("OpenCL: Build Log:\n");
	      printf("%s", programBuildLog);
	      printf("\n");
	
	}

	free(buf[0]);
	free(buf);
	free(compileOptions);

    return(clerr);
}


cl_int getKernel(cl_kernel *kernel_,cl_program program_, const char *kernel_name_)
{
	cl_int clerr = CL_SUCCESS;

	*kernel_ = clCreateKernel(program_, kernel_name_,  &clerr);
    return(clerr);
}

