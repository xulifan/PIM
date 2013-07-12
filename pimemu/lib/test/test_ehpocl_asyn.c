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

/* Parallel Prefix Sum ported to run on PIM CPUs */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#ifdef WIN32
#include "wpthread.h"
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include <CL/cl.h>
#include <pim.h>
#include <string.h>
#include <math.h>



#define ARRAY_DIM_SIZE 200
#define ARRAY_SIZE (ARRAY_DIM_SIZE*ARRAY_DIM_SIZE*ARRAY_DIM_SIZE)
#define __DEFAULT_DIR__ "./test/"
#define __DEFUALT_CONF__ "1.xml"
#define TYPE       float 



// The map from the list of threads (sequential list) to PIM device ID.
static pim_device_id* list_of_pims;
static int * gpus_per_pim = 0;

/*****************************************************************************/
/************************* EHP OCL Functions *********************************/
/*****************************************************************************/
#define ASSERT_CL_RETURN( ret )\
   if( (ret) != CL_SUCCESS )\
   {\
      fprintf( stderr, "%s:%d: error: %s\n", \
             __FILE__, __LINE__, getOpenCLErrorString( (ret) ));\
      exit(-1);\
   }

static
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


#define _MAX_PLATFORMS_ 256
#define _MAX_DEVICES_   2048
static
cl_int EHP_initOcl( cl_context *context_, cl_device_id *device_  ) {
	
cl_uint plat_count = 0;
cl_int clerr = CL_SUCCESS;
cl_platform_id clplatform;
//cl_device_id cldevice;
//cl_context clcontext;
cl_platform_id platforms[_MAX_PLATFORMS_];
cl_device_type	device_type = CL_DEVICE_TYPE_GPU;
cl_uint device_count = 0;
cl_device_id devices[_MAX_DEVICES_];


	printf("EHP OpenCL: Initializing Interface...\n");

	clerr = clGetPlatformIDs(0, NULL, &plat_count);
	
	if(clerr == CL_SUCCESS) {
		printf("EHP OpenCL: Found %u platforms.\n", plat_count);
	} else {
		printf("EHP OpenCL: Unable to find any OpenCL compatible platforms.\n");
		printf("EHP OpenCL: %s\n", getOpenCLErrorString(clerr));
		return (clerr);
	}
	

	clerr = clGetPlatformIDs(plat_count, platforms, NULL);
	clplatform = platforms[0];
	
	//////////////////////////////////////////////////////////////////////////////////
	
	printf("EHP OpenCL: Querying for all OpenCL compatible GPU devices...\n");

	clerr = clGetDeviceIDs(clplatform, device_type, 0, NULL, &device_count);
	
	if(clerr == CL_SUCCESS) {
		printf("EHP OpenCL: Found %u OpenCL devices.\n", device_count);
	} else {
		printf("EHP OpenCL: Device query on OpenCL platform failed.\n");
		printf("EHP OpenCL: %s\n", getOpenCLErrorString(clerr));
		return (clerr);
	}
	
	clerr = clGetDeviceIDs(clplatform, device_type, device_count, devices, NULL);
	
	*device_ = devices[0];
	
	//////////////////////////////////////////////////////////////////////////////////
	*context_ = clCreateContext( NULL, 1, device_, NULL, NULL, &clerr);
	
	if(clerr == CL_SUCCESS) {
		printf("EHP OpenCL: Successfully established OpenCL context.\n");
	} else {
		printf("EHP OpenCL: Unable to create a context on the selected device.\n");
		printf("EHP OpenCL: %s\n", getOpenCLErrorString(clerr));
		return (clerr);

	}
	return (clerr);
	
}

static
cl_int EHP_getDevicePropoery(void *ehp_queue_, cl_device_id device_, int property_id_, int property_ln_)
{
cl_int clerr = CL_SUCCESS;
size_t tmp;
   clerr = clGetDeviceInfo( device_, property_id_, property_ln_, ehp_queue_, &tmp);
   return(clerr);
}
static
cl_int EHP_createOclQueue(cl_command_queue *ehp_queue_, cl_context context_, cl_device_id device_)
{
cl_int clerr = CL_SUCCESS;
	//////////////////////////////////////////////////////////////////////////////////
	
   *ehp_queue_ = clCreateCommandQueue(context_, device_, 0, &clerr);
	
	if(clerr == CL_SUCCESS) {
		printf("EHP OpenCL: Successfully established OpenCL command queue.\n");
	} else {
		printf("EHP OpenCL: Unable to create an OpenCL command queue for device.\n");
		printf("EHP OpenCL: %s\n", getOpenCLErrorString(clerr));
	}
	return (clerr);
}


static
cl_int EHP_compileOclKernels(cl_program *program_, cl_context context_, cl_device_id device_, const char *file_name_, const char *options_)
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
		printf("EHP OpenCL: ABORT, cannot read file %s\n", file_name_);
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
	
	printf("EHP OpenCL: Building OpenCL program and kernels...\n");
	
	compileOptions = (char*) malloc(sizeof(char) * 65536);
	compileOptions[0] = '\0';
    if ( options_ )
	{
		strcpy(compileOptions, options_);
	}
	
	compileOptions = strcat(compileOptions, " -D __GPU__");

	printf("EHP OpenCL: Compiling program with options: %s\n", compileOptions);
	
	*program_ = clCreateProgramWithSource(context_, 1, (const char**) buf, len, &clerr);
	clerr   = clBuildProgram(*program_, 1, &device_, compileOptions, NULL, NULL);
	if(clerr == CL_SUCCESS) {
		printf("EHP OpenCL: Build program call completed with success.\n");
	} else 
	{

	char programBuildLog[65536];
	programBuildLog[0] = '\0';

		printf("EHP OpenCL: OpenCL program build failed.\n");
		printf("EHP OpenCL: %s\n", getOpenCLErrorString(clerr));
	      clGetProgramBuildInfo(*program_, device_, CL_PROGRAM_BUILD_LOG, 65536, 
							(void*) programBuildLog, NULL);
	
	      printf("EHP OpenCL: Build Log:\n");
	      printf("%s", programBuildLog);
	      printf("\n");
	
	}

	free(buf[0]);
	free(buf);
	free(compileOptions);

    return(clerr);
}

static
cl_int EHP_getKernel(cl_kernel *kernel_,cl_program program_, const char *kernel_name_)
{
	cl_int clerr = CL_SUCCESS;

	*kernel_ = clCreateKernel(program_, kernel_name_,  &clerr);
    return(clerr);
}

/*****************************************************************************/
/********** Functions to find correct file to send to pim_init() *************/
/*****************************************************************************/
char *concat_directory_and_name(const char *directory, const char *name)
{
    size_t directory_length, name_length;
    char* return_str;

    // Directory is guaranteed to have a slash after it.
    directory_length = strlen(directory);
    name_length = strlen(name);

    return_str = malloc(sizeof(char) * (directory_length + name_length + 1));
    if (return_str == NULL) {
        fprintf(stderr, "Unable to allocate Directory+Name Concat. Die.\n");
        exit(-1);
    }

    return_str[(directory_length + name_length)] = '\0';
    strcpy(return_str, directory);
    strcpy((return_str+directory_length), name);
    return return_str;
}

char* fix_directory(char *input)
{
    char *directory;
    size_t directory_length;

    directory_length = strlen(input);

    if (input[directory_length-1] != '/')
        directory_length += 1;

    directory = malloc(sizeof(char) * (directory_length + 1));
    if (directory == NULL) {
        fprintf(stderr, "Unable to allocate directory string. Die.\n");
        exit(-1);
    }
    directory[directory_length] = '\0';

    strcpy(directory, input);
    if (input[directory_length] != '/')
        directory[directory_length-1] = '/';
	return(directory);
}

char* find_directory(char *input)
{
    char *directory;
    size_t directory_length;

	char * pos1 = strrchr(input, '/');
	char * pos2 = strrchr(input, '\\');

	char *faraway = (!pos1)? pos2 : ((!pos2) ? pos1 : (( pos1 < pos2) ? pos2 : pos1));

    directory_length = (size_t)(faraway - input) + 1;

    directory = malloc(sizeof(char) * (directory_length + 1));

    strncpy(directory, input, directory_length);
	directory[directory_length] = 0;
	return(directory);
}


/*****************************************************************************/
/********* End Functions to find correct file to send to pim_init() **********/
/*****************************************************************************/
uint32_t get_number_pim_compute_cores(uint32_t pim_id, uint32_t core_type)
{
    uint32_t number_of_cores = 0;
    uint32_t i = pim_id;
    int failure = 0;
    size_t needed_size;
    void *device_info;
    failure = pim_get_device_info(list_of_pims[i], core_type, 0, NULL, &needed_size);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr,
                "%s (%d): pim_get_device_info failed (%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }
    device_info = malloc(needed_size);
    if (device_info == NULL) {
        fprintf(stderr, "%s (%d): Out of memory for size %" PRIuMAX
                ". Die.\n", __func__, __LINE__, (uintmax_t)needed_size);
        exit(-1);
    }
    failure = pim_get_device_info(list_of_pims[i], core_type,
            needed_size, device_info, NULL);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d). pim_get_device_info failed (%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }

    number_of_cores = *(uint32_t *)device_info;
    free(device_info);
    return(number_of_cores);
}

uint32_t get_number_of_compute_cores(uint32_t num_pims, uint32_t core_type )
{
	int i;

    unsigned int number_of_cores = 0;

    for (i = 0; i < (int)num_pims; i++) {

		number_of_cores +=get_number_pim_compute_cores(i, core_type);
    }
	return(number_of_cores);
}


// Use PIM API discovery functions to find the number of PIMs within the system
// As an exercise, this also finds the number of CPUs contained within those
// PIMs. Currently, we only launch one thread per PIM, however, so that is
// what this function returns.


unsigned int find_and_map_pims(int * num_gpus)
{
    int i;
    int failure = 0;
    uint32_t num_pims = 0;
    unsigned int number_of_pim_cpus = 0;
    unsigned int number_of_pim_gpus = 0;
    failure = pim_get_device_ids(PIM_CLASS_0, 0, NULL, &num_pims);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): pim_get_device_ids failed(%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }
    list_of_pims = malloc(num_pims * sizeof(pim_device_id));
    failure = pim_get_device_ids(PIM_CLASS_0, num_pims, list_of_pims, NULL);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): pim_get_device_ids failed(%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }

    gpus_per_pim = (int*)malloc(num_pims * sizeof(int));

	number_of_pim_cpus = get_number_of_compute_cores(num_pims, PIM_CPU_CORES);
    printf("Number of PIM CPUs: %u \n", number_of_pim_cpus);

    for (i = 0; i < (int)num_pims; i++) {
	     gpus_per_pim[i] = get_number_pim_compute_cores(i, PIM_GPU_CORES);
		 number_of_pim_gpus += gpus_per_pim[i];
	}
    printf("Number of PIM GPUs: %u \n", number_of_pim_gpus);
	*num_gpus = number_of_pim_gpus;
    return num_pims;
}


// Fill a host buffer with random values

int fillWithRand(
         TYPE * buffer_, 
         size_t width_,
         size_t height_,
         const TYPE rangeMin_,
         const TYPE rangeMax_,
         unsigned int seed_)
{
double range;
size_t i, j;

    // set seed

    srand(seed_);

    // set the range
    range = (double)rangeMax_ - (double)rangeMin_ + 1.0; 

    /* random initialisation of input */
    for(i = 0; i < height_; i++)
        for(j = 0; j < width_; j++)
        {
         size_t index = i*width_ + j;
            buffer_[index] = rangeMin_ + (TYPE)(range*rand()/(RAND_MAX + 1.0)); 
        }

    return 0;
}



// waxpby kerenl
	void waxpby_kernel( TYPE alpha_,
		               TYPE beta_,
					   TYPE * x_,
					   TYPE * y_,
					   TYPE * w_,
					   size_t length_)
	{
	  int i;
		for( i = 0; i < length_; i++)
		{
			w_[i] = alpha_ * x_[i] + beta_ * y_[i];
		}
	}
// dotprod kerenel

	TYPE dotprod_kernel(
					   TYPE * x_,
					   TYPE * y_,
					   size_t length_)
	{
     TYPE ret = 0;  
	 double tmp_ret = 0;
	 int i;
		for( i = 0; i < length_; i++)
		{
			tmp_ret += (double)x_[i] * (double)y_[i];
		}
		ret = (TYPE)tmp_ret;
		return(ret);
	}

int getSysNumCores(void)
{
int numCPU = 0;
#ifdef WIN32
SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
   
    numCPU = sysinfo.dwNumberOfProcessors;
#else

//Linux, Solaris, & AIX and Mac OS X (for all OS releases >= 10.4, i.e., Tiger onwards) - per comments:
numCPU = sysconf( _SC_NPROCESSORS_ONLN );
#endif

#if 0
//FreeBSD, MacOS X, NetBSD, OpenBSD, etc.:

int mib[4];
size_t len = sizeof(numCPU); 

/* set the mib for hw.ncpu */
mib[0] = CTL_HW;
mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

/* get the number of CPUs from the system */
sysctl(mib, 2, &numCPU, &len, NULL, 0);

if( numCPU < 1 ) 
{
     mib[1] = HW_NCPU;
     sysctl( mib, 2, &numCPU, &len, NULL, 0 );

     if( numCPU < 1 )
     {
          numCPU = 1;
     }
}
#endif

    return(numCPU);
}


int main(int argc, char **argv)
{
    int i = 0, k;

	size_t problem_size = ARRAY_SIZE;
	int pim_num_gpus;
	int pim_num_threads;
	int ehp_num_gpus;
	int ehp_num_threads;
    int verification_fails = 0;

    // Parallel vector linear combination arrays
    void **parallel_input_array;
    void **parallel_output_array;
    TYPE **seq_output_array;

    TYPE **parallel_mapped_input_array;
    TYPE **parallel_mapped_output_array;

    // amote of work per GPU
    size_t amount_to_move = 0;
    size_t amount_to_moveCPU = 0;
	    

// EHP GPU interface
cl_int clerr = CL_SUCCESS;
cl_device_id cldevice = 0;
cl_context clcontext = 0;
cl_command_queue ehp_queue = 0;
cl_program clprogram = 0;
cl_kernel clkernel_dotprod = 0;
int nmb_CUs;
// DOT PROD
// fill the machine 
size_t dotprod_exec_range;
size_t dotprod_x_sz;
int wvfrnt_sz = 64;
cl_mem cldotprod, clx, cly;
float *mapped_cldotprod = NULL;
float *mapped_clx = NULL, *mapped_cly = NULL;
float host_dotprod = 0;
size_t glbl_dotprod_sz[3] = {1,1,1};
size_t lcl_dotprod_sz[3] = {wvfrnt_sz, 1, 1};
cl_event ehp_completion;
cl_uint ehp_nargs = 0;


// PIM kernel compiler arguments
const char * source_nm = "minife.cl";
const char * waxpby = "waxpby";
const char * dotprod = "dotprod";
pim_f gpu_kernel;

char* ocl_source;
const void * args[1024];
size_t sizes[1024];
size_t nargs = 0;
int dim = 1;
size_t lcl[3] = {wvfrnt_sz};
size_t glbl[3]; 
int num_pre_event = 0;
cl_event *pim_completion = 0;


float alpha = 0.3f;
float beta  = 0.5f;

int ehp_only = 0;
int verification = 1;
int a;

    printf("**************************************************\n");
    printf("Beginning EHP GPU/ PIM GPU Asynchronous Spawn Test\n");

    /* Get PIM information */

    if (argc < 2) {
        fprintf(stderr, "Requires a directory to search for .cl file as its argument.\n");
        exit(-1);
    }

    printf("Arguments:\n");
	for(a = 1; a < argc; a++)
	{
		printf(argv[a]);
		printf("\n");
	}

// PIM PER MODEL
    pim_num_threads = find_and_map_pims(&pim_num_gpus);
  //  printf("Number of threads: %d\n", num_threads);

    // FIXME: I believe this will not quite work if the array size and num_threads don't
    // divide evently. We will leave the remainder uselessly dangling at the end of the last
    // partial array. FIXME LATER
    if (pim_num_threads != 0 && pim_num_gpus != 0)
	{
		pim_num_gpus = (( ehp_only) ? 1 : pim_num_gpus);
        amount_to_move = problem_size/pim_num_gpus;
	}
    else {
        fprintf(stderr, "This test must be run with at least one PIM-GPU.\n");
        exit(-1);
    }

// build kernels from the source location
	{
    char * directory = 0;
	    directory = fix_directory(argv[1]);
		if ( directory ) 
		{
	        ocl_source = concat_directory_and_name(directory, source_nm);
			free(directory);
		}
		else
		{
			exit(0);
		}
	}
   
// initialize EHP GPU interface
	EHP_initOcl( &clcontext, &cldevice  );
    EHP_createOclQueue(&ehp_queue, clcontext, cldevice);
    EHP_compileOclKernels(&clprogram, clcontext, cldevice, ocl_source, NULL);
	EHP_getDevicePropoery(&nmb_CUs, cldevice, CL_DEVICE_MAX_COMPUTE_UNITS, 4);
	dotprod_exec_range = nmb_CUs * 32 * wvfrnt_sz;
	cldotprod = clCreateBuffer(clcontext, CL_MEM_WRITE_ONLY, dotprod_exec_range * sizeof(float), NULL, &clerr);
	dotprod_x_sz = problem_size * sizeof(float);
	clx = clCreateBuffer(clcontext, CL_MEM_READ_ONLY, dotprod_x_sz, NULL, &clerr);
	cly = clCreateBuffer(clcontext, CL_MEM_READ_ONLY, dotprod_x_sz, NULL, &clerr);
	ASSERT_CL_RETURN(clerr);
	pim_get_device_info(PIM_HOST_ID, HOST_CPU_CORES, sizeof(uint32_t), &ehp_num_threads, NULL);
	pim_get_device_info(PIM_HOST_ID, HOST_GPU_CORES, sizeof(uint32_t), &ehp_num_gpus, NULL);



#ifdef _PROPERTIES_
	// Properties
	printf("\nProperies:\n\n");

		{
         uint32_t cpu_freq;
		 uint32_t num_gpus, eng_clock;
			pim_get_device_info(PIM_HOST_ID, HOST_CPU_FREQ, sizeof(uint32_t), &cpu_freq, NULL);
			pim_get_device_info(PIM_HOST_ID, HOST_GPU_FREQ, sizeof(uint32_t), &eng_clock, NULL);


         	printf("Exascale HOST:\n");
		    printf("NUM CPU CORES: %d\n", ehp_num_threads);
		    printf("CPU CLOCK: %5.2f\n", (double)cpu_freq/1000 );
		    printf("NUM GPUs: %d\n", ehp_num_gpus);
		    printf("GPU ENGINE CLOCK: %5.2f\n", (double)eng_clock/1000);
	
		}


    for (i = 0, k = 0; i < pim_num_threads; i++) {
 

		if ( gpus_per_pim[i] > 0 )
		{
         uint32_t num_cus, num_channels;
		 uint32_t eng_clock;
		 uint32_t mem_mbps;
		 size_t ret_size;
			pim_get_device_info(i, PIM_GPU_NUM_CUS, sizeof(uint32_t), &num_cus, &ret_size);
			pim_get_device_info(i, PIM_GPU_FREQ, sizeof(uint32_t), &eng_clock, &ret_size);
			pim_get_device_info(i, PIM_MEM_MBPS, sizeof(uint32_t), &mem_mbps, &ret_size);
			pim_get_device_info(i, PIM_MEM_NUM_CHANNELS, sizeof(uint32_t), &num_channels, &ret_size);

    	    printf("PIM %d:\n", i);

         	printf("GPU %d:\n", 0);
		    printf("NUM COMPUTE UNITS: %d\n", num_cus);
		    printf("ENGINE CLOCK: %5.2f\n", (double)eng_clock/1000 );
		    printf("MEMORY GIGABITS PER SECOND: %5.2f\n", (double)mem_mbps/1000);
		    printf("NUM CHANNELS: %d\n\n", num_channels);
		     k++;
		}
    }

#endif


// alloctae cl control data
	pim_completion = (cl_event *)calloc(pim_num_gpus, sizeof(cl_event));
	assert(pim_completion);

    // Allocates the per-PIM structures.
// 2 inputs per kernel
    parallel_input_array = (void**)malloc(sizeof(void *) * pim_num_gpus * 2);
	parallel_mapped_input_array = (TYPE**)malloc(sizeof(TYPE *) * pim_num_gpus * 2);
// 1 output
    parallel_output_array = (void**)malloc(sizeof(void *) * pim_num_gpus);
	parallel_mapped_output_array = (TYPE**)malloc(sizeof(TYPE *) * pim_num_gpus);

// verification arrays
    seq_output_array = (TYPE**)malloc(sizeof(TYPE *) * pim_num_gpus);

	assert ( parallel_input_array
		    && parallel_mapped_input_array
			&& parallel_output_array
			&& parallel_mapped_output_array );
// FIXME: CHECK SUCCESS
    for (i = 0, k = 0; i < pim_num_threads && k < pim_num_gpus; i++) {
		if ( gpus_per_pim[i] > 0 )
		{
             parallel_input_array[k*2] = pim_malloc(sizeof(TYPE) * amount_to_move, list_of_pims[i], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
		     assert(parallel_input_array[k*2]);
             parallel_input_array[k*2 + 1] = pim_malloc(sizeof(TYPE) * amount_to_move, list_of_pims[i], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
		     assert(parallel_input_array[k*2 + 1]);
             parallel_output_array[k] = pim_malloc(sizeof(TYPE) * amount_to_move, list_of_pims[i], PIM_MEM_PIM_WRITE | PIM_MEM_HOST_READ, PIM_PLATFORM_OPENCL_GPU);
		     assert(parallel_output_array[k]);
// seq verification
		     seq_output_array[k] = (TYPE*)malloc(sizeof(TYPE) * amount_to_move);
		     assert(seq_output_array[k]);

		     k++;
		}
    }




    printf("Problem size %dx%dx%d...\n",ARRAY_DIM_SIZE, ARRAY_DIM_SIZE, ARRAY_DIM_SIZE );
    printf("Map input baffers.\nGenerating initial arrays...\n" );
    for (i = 0; i < pim_num_gpus * 2; i++) {
         parallel_mapped_input_array[i] = (TYPE*)pim_map(parallel_input_array[i],PIM_PLATFORM_OPENCL_GPU);
	     assert(parallel_mapped_input_array[i]);
       	 fillWithRand(parallel_mapped_input_array[i], amount_to_move, 1, 0., 255., 0);
    }


    // **** Start Mark 1 *********
	pim_emu_begin();


 if ( verification )
 {
    printf("Calculating W = Ax + By, sequentially...\n");
    for (i = 0; i < pim_num_gpus; i++) {

		waxpby_kernel( alpha,
		               beta,
					   parallel_mapped_input_array[i*2],
					   parallel_mapped_input_array[i*2 + 1],
					   seq_output_array[i],
					   amount_to_move);
    }
 }

   //**** Exit Mark 1 *****
         pim_emu_end();

 // EMULATION: NO COPY in REAL SYSTEM??!!
 // fill EHP buffers 

    mapped_clx = (float*)clEnqueueMapBuffer(ehp_queue,
                                            clx,
                                            CL_TRUE, 
                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                            0,
                                            dotprod_x_sz,
                                            0,
                                            NULL,
                                            NULL,
                                            &clerr);
		ASSERT_CL_RETURN(clerr);

    mapped_cly = (float*)clEnqueueMapBuffer(ehp_queue,
                                            cly,
                                            CL_TRUE, 
                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                            0,
                                            dotprod_x_sz,
                                            0,
                                            NULL,
                                            NULL,
                                            &clerr);
		ASSERT_CL_RETURN(clerr);

	{
     size_t curr_pos, copy_sz, left_sz;
	   left_sz = dotprod_x_sz;
       for (i = 0; i < pim_num_gpus; i++) {
    	     copy_sz = (left_sz < amount_to_move * sizeof(float)) ? left_sz : amount_to_move * sizeof(float);
			 curr_pos = dotprod_x_sz - left_sz;
	    	 memcpy( (char*)mapped_clx + curr_pos, parallel_mapped_input_array[i*2], copy_sz);
	    	 memcpy( (char*)mapped_cly + curr_pos, parallel_mapped_input_array[i*2 + 1], copy_sz);
			 left_sz -= copy_sz;
       }
	}

    if ( verification )
    {
        printf("Calculating DOT(x, y), sequentially...\n");

        host_dotprod = dotprod_kernel(mapped_clx,
					              mapped_cly,
					              problem_size);
	}


    printf("Unmap input buffers...\n");
// make data accessable by GPU
    for (i = 0; i < pim_num_gpus * 2; i++) {
        pim_unmap(parallel_mapped_input_array[i]);
    }

 // EMULATION: NO COPY in REAL SYSTEM??!!
 // fill EHP buffers 
    clerr = clEnqueueUnmapMemObject(ehp_queue,
                            clx,
                            mapped_clx,
                            0,
                            NULL,
                            NULL);
	ASSERT_CL_RETURN(clerr);

    clerr = clEnqueueUnmapMemObject(ehp_queue,
                            cly,
                            mapped_cly,
                            0,
                            NULL,
                            NULL);
	ASSERT_CL_RETURN(clerr);


// launch the kernel
   printf("Parallel W = Ax + By on PIMs...\n");

	  gpu_kernel.func_name = (void*)waxpby;


       args[nargs] =  ocl_source;
	   sizes[nargs] = sizeof(char*);
	   nargs++;

       args[nargs] =  " -D __GPU__";
	   sizes[nargs] = sizeof(char*);
	   nargs++;

// kernel launch arguments
       args[nargs] =  &dim;
	   sizes[nargs] = sizeof(int);
	   nargs++;


	   glbl[0] = ((amount_to_move / lcl[0]) + 1) * lcl[0];

       args[nargs] =  glbl;
	   sizes[nargs] = sizeof(size_t) * dim;
	   nargs++;

       args[nargs] =  lcl;
	   sizes[nargs] = sizeof(size_t) * dim;
	   nargs++;

// event arguments
// pre-events
       args[nargs] =  &num_pre_event;
	   sizes[nargs] = sizeof(int);
	   nargs++;

// pre-events
       args[nargs] =  NULL;
	   sizes[nargs] = 0;
	   nargs++;

// post-events

       args[nargs] =  NULL;
	   sizes[nargs] = 0;
	   nargs++;


// kernel arguments
       args[nargs] = &amount_to_move;
	   sizes[nargs] = sizeof(int);
	   nargs++;

       args[nargs] = &alpha;
	   sizes[nargs] = sizeof(TYPE);
	   nargs++;

       args[nargs] = &beta;
	   sizes[nargs] = sizeof(TYPE);
	   nargs++;

 

    // **** Start Mark 2 *********
        pim_emu_begin();

    for (i = 0, k = 0; i < pim_num_threads && k < pim_num_gpus; i++) {
		if ( gpus_per_pim[i] > 0 )
		{
        size_t tnargs = nargs;
			args[tnargs] = parallel_input_array[k*2];
	        sizes[tnargs++] = sizeof(void*);
			args[tnargs] = parallel_input_array[k*2 + 1];
	        sizes[tnargs++] = sizeof(void*);
			args[tnargs] = parallel_output_array[k];
	        sizes[tnargs++] = sizeof(void*);


// insert compeletion event

			args[OPENCL_ARG_POSTEVENT] = pim_completion + k;
			sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);


			pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, list_of_pims[i], PIM_PLATFORM_OPENCL_GPU);

			k++;
		}
    }

	glbl_dotprod_sz[0] = dotprod_exec_range;

	EHP_getKernel(&clkernel_dotprod,clprogram, dotprod);
// wait for all PIMs to complete
	ehp_nargs = 0;
	clSetKernelArg(clkernel_dotprod,
                   ehp_nargs++,
                   sizeof(int),
                   &problem_size);

	clSetKernelArg(clkernel_dotprod,
                   ehp_nargs++,
                   sizeof(cl_mem),
                   &clx);

	clSetKernelArg(clkernel_dotprod,
                   ehp_nargs++,
                   sizeof(cl_mem),
                   &cly);

	clSetKernelArg(clkernel_dotprod,
                   ehp_nargs++,
                   sizeof(cl_mem),
                   &cldotprod);

// wait for PIM gpus to finish

	clerr = clWaitForEvents(pim_num_gpus, pim_completion) ;

	ASSERT_CL_RETURN(clerr);

 // launch the kernel
    printf("Parallel DOT(x, y) on EHP...\n");
	clerr = clEnqueueNDRangeKernel(ehp_queue,
                                   clkernel_dotprod,
                                   1,
                                   NULL,
                                   glbl_dotprod_sz,
                                   lcl_dotprod_sz,
                                   0,
                                   NULL,
                                   &ehp_completion);
    ASSERT_CL_RETURN(clerr);


	for(i = 0; i < pim_num_gpus; i++)
	{
		clReleaseEvent(pim_completion[i]);
	}
	

// wait for EHP gpu to finish

	clerr = clWaitForEvents(1, &ehp_completion) ;

	ASSERT_CL_RETURN(clerr);

	clReleaseEvent(ehp_completion);

    // **** End Mark 2 *********
        pim_emu_end();

 

	free(ocl_source);



 if ( verification )
 {
    printf("Checking results.\n");

   // **** Start Mark 3 *********
    pim_emu_begin();
	verification_fails = 0;

	// waxpby verification
    for (i = 0; i < pim_num_gpus &&  !verification_fails; i++) {
	int j;

    
         parallel_mapped_output_array[i] = (float*)pim_map(parallel_output_array[i],PIM_PLATFORM_OPENCL_GPU);

        for (j = 0; j < amount_to_move; j++)
		{

 
 
		    double h = (double)seq_output_array[i][j];
			double g = (double)parallel_mapped_output_array[i][j];

            verification_fails = (abs(h - g) /g) > 0.0000001 ;
            if ( verification_fails ) {
                fprintf(stderr,
                        "Parallel Ax + By. First differs at Index %d,%d. %f != %f\n",
                        i, j,
						seq_output_array[i][j],
                        parallel_mapped_output_array[i][j]);
                break;
            }
        }
		pim_unmap(parallel_mapped_output_array[i]);



    }

	// dotprod verification
		if ( !verification_fails )
		{
		 int j;
	     float gpu_dotprod = 0;
            mapped_cldotprod = (float*)clEnqueueMapBuffer(ehp_queue,
                                            cldotprod,
                                            CL_TRUE, 
                                            CL_MAP_READ,
                                            0,
                                            dotprod_exec_range * sizeof(float),
                                            0,
                                            NULL,
                                            NULL,
                                            &clerr);
	        ASSERT_CL_RETURN(clerr);


			gpu_dotprod = 0;
		    for(j = 0; j < dotprod_exec_range; j++)
		    {
			    gpu_dotprod += mapped_cldotprod[j];
		    }


            clerr = clEnqueueUnmapMemObject(ehp_queue,
                            cldotprod,
                            mapped_cldotprod,
                            0,
                            NULL,
                            NULL);
	         ASSERT_CL_RETURN(clerr);
		     {

		     double delta = abs((double)host_dotprod - (double)gpu_dotprod)/(double)gpu_dotprod;
		          verification_fails = (delta > .00001);
		     }

		}


       // **** End Mark 3 *********
    pim_emu_end();


	if (!verification_fails )
	{
	    printf("Success!\n");
	}
	else
	{
	    printf("Failure!\n");
	}
 }

// delete buffers
	printf("Deleting buffers...\n");


    for (i = 0; i < pim_num_gpus * 2; i++) {
		if ( parallel_input_array[i] )
		{
           pim_free(parallel_input_array[i]);
		}
    }
    for (i = 0; i < pim_num_gpus; i++) {
		if ( parallel_output_array[i])
		{
           pim_free(parallel_output_array[i]);
		}
		if ( seq_output_array[i] )
		{
           free(seq_output_array[i]);
		}
    }

	if ( seq_output_array ) 
	{
       free(seq_output_array);
	}
	if ( parallel_input_array) 
	{
       free(parallel_input_array);
	}
	if ( parallel_mapped_input_array)
	{
       free(parallel_mapped_input_array);
	}
	if ( parallel_output_array) 
	{
       free(parallel_output_array);
	}
	if ( parallel_mapped_output_array) 
	{
      free(parallel_mapped_output_array);
	}

// free EHP OCL resources
	clReleaseMemObject(cldotprod);
	clReleaseMemObject(clx);
	clReleaseMemObject(cly);

	if ( pim_completion )
	{
		free(pim_completion);
	}



    return 0;
}
