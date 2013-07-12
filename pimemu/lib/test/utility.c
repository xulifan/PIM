#include <utility.h>


#define ASSERT_CL_RETURN( ret )\
   if( (ret) != CL_SUCCESS )\
   {\
      fprintf( stderr, "%s:%d: error: %s\n", \
             __FILE__, __LINE__, getOpenCLErrorString( (ret) ));\
      exit(-1);\
   }

static const char *getOpenCLErrorString(cl_int err) {

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


#define ASSERT_PIM_RETURN( ret )\
   if( (ret) != CL_SUCCESS )\
   {\
      fprintf( stderr, "%s:%s:%d: error: %s\n", \
             __FILE__, __func__, __LINE__, getPIMErrorString( (ret) ));\
      exit(-1);\
   }

static const char *getPIMErrorString(int err) {

    switch(err) {

        case PIM_SUCCESS: return "PIM_SUCCESS";
        case PIM_GENERAL_ERROR: return "PIM_GENERAL_ERROR";
        case PIM_BUILTWITHOUT_PTHREAD: return "PIM_BUILTWITHOUT_PTHREAD";
        case PIM_INIT_FILE_ACCESS_ERROR: return "PIM_INIT_FILE_ACCESS_ERROR";
        case PIM_INIT_FILE_SYNTAX_ERROR: return "PIM_INIT_FILE_SYNTAX_ERROR";
        case PIM_INVALID_DEVICE_TYPE: return "PIM_INVALID_DEVICE_TYPE";
        case PIM_INVALID_DEVICE_ID: return "PIM_INVALID_DEVICE_ID";
        case PIM_INVALID_PARAMETER: return "PIM_INVALID_PARAMETER";
        case PIM_INSUFFICIENT_SPACE: return "PIM_INSUFFICIENT_SPACE";
        case PIM_SPAWN_ERROR_BAD_TYPE: return "PIM_SPAWN_ERROR_BAD_TYPE";
        case PIM_SPAWN_ERROR_DEVICE_TYPE_MISMATCH: return "PIM_SPAWN_ERROR_DEVICE_TYPE_MISMATCH";
        case PIM_INVALID_CPU_CONFIGURATION_PARAMETER: return "PIM_INVALID_CPU_CONFIGURATION_PARAMETER";
        case PIM_INVALID_GPU_CONFIGURATION_PARAMETER: return "PIM_INVALID_GPU_CONFIGURATION_PARAMETER";
        case PIM_INVALID_DRAM_CONFIGURATION_PARAMETER: return "PIM_INVALID_DRAM_CONFIGURATION_PARAMETER";
        case PIM_INVALID_EHP_CONFIGURATION_PARAMETER: return "PIM_INVALID_EHP_CONFIGURATION_PARAMETER";
        case PIM_INVALID_PIM_CONFIGURATION_PARAMETER: return "PIM_INVALID_PIM_CONFIGURATION_PARAMETER";
        case PIM_INVALID_CONFIGURATION: return "PIM_INVALID_CONFIGURATION";

        default: return "UNKNOWN PIM ERROR CODE";
    }
}



/*****************************************************************************/
/********* End Functions to find correct file to send to pim_init() **********/
/*****************************************************************************/
uint32_t get_number_pim_compute_cores(pim_device_id pim_id, uint32_t core_type)
{
    uint32_t number_of_cores = 0;
    int failure = 0;
    size_t needed_size;
    void *device_info;
    failure = pim_get_device_info(pim_id, core_type, 0, NULL, &needed_size);
    ASSERT_PIM_RETURN(failure);
    device_info = malloc(needed_size);
    if (device_info == NULL) {
        fprintf(stderr, "%s (%d): Out of memory for size %" PRIuMAX
                ". Die.\n", __func__, __LINE__, (uintmax_t)needed_size);
        exit(-1);
    }
    failure = pim_get_device_info(pim_id, core_type, needed_size, device_info, NULL);
    ASSERT_PIM_RETURN(failure);

    number_of_cores = *(uint32_t *)device_info;
    free(device_info);
    return(number_of_cores);
}


// Use PIM API discovery functions to find the number of PIMs within the system
// As an exercise, this also finds the number of CPUs contained within those
// PIMs. Currently, we only launch one thread per PIM, however, so that is
// what this function returns.


unsigned int find_pims()
{
    int failure = 0;
    uint32_t num_pims = 0;
    failure = pim_get_device_ids(PIM_CLASS_0, 0, NULL, &num_pims);
    ASSERT_PIM_RETURN(failure);
    
    return num_pims;
}

void map_pims(int num_pims, int *num_gpus, int *num_cpus, int *gpus_per_pim, int *cpus_per_pim, pim_device_id *list_of_pims)
{
    int i;
    int failure = 0;
    unsigned int number_of_pim_cpus = 0;
    unsigned int number_of_pim_gpus = 0;
    
    failure = pim_get_device_ids(PIM_CLASS_0, num_pims, list_of_pims, NULL);
    ASSERT_PIM_RETURN(failure);
    //for(i=0;i<num_pims;i++) printf("%d\n",list_of_pims[i]);

    for (i = 0; i < (int)num_pims; i++) {
	     gpus_per_pim[i] = get_number_pim_compute_cores(list_of_pims[i], PIM_GPU_CORES);
		 number_of_pim_gpus += gpus_per_pim[i];
         cpus_per_pim[i] = get_number_pim_compute_cores(list_of_pims[i], PIM_CPU_CORES);
		 number_of_pim_cpus += cpus_per_pim[i];
	}

    printf("Number of PIM CPUs: %u \n", number_of_pim_cpus);
    printf("Number of PIM GPUs: %u \n", number_of_pim_gpus);

	*num_gpus = number_of_pim_gpus;
    *num_cpus = number_of_pim_cpus;
    return;
}

/*
void get_pim_cpu_ids(int num_cpus, pim_device_id *list_of_cpus)
{
    int failure = 0;
    failure = pim_get_device_ids(PIM_CLASS_CPU, num_cpus, list_of_cpus, NULL);
    ASSERT_PIM_RETURN(failure);
    for(int i=0;i<num_cpus;i++) printf("%d\n",list_of_cpus[i]);
}

void get_pim_gpu_ids(int num_gpus, pim_device_id *list_of_gpus)
{
    int failure = 0;
    failure = pim_get_device_ids(PIM_CLASS_GPU, num_gpus, list_of_gpus, NULL);
    ASSERT_PIM_RETURN(failure);
    for(int i=0;i<num_gpus;i++) printf("%d\n",list_of_gpus[i]);
}
*/


void pim_property(int num_pims, int *gpus_per_pim, int *cpus_per_pim, pim_device_id *list_of_pims)
{

    printf("\nProperies:\n\n");
    int failure=0;

    for (int i = 0; i < num_pims; i++) {
        int num_gpus=gpus_per_pim[i];
        int num_cpus=cpus_per_pim[i];
        size_t ret_size;
        uint32_t mem_mbps,num_channels;
        uint32_t mem_size=0;
        printf("PIM %d:\n", i);
        printf("Num of CPUs %d:\n", num_cpus);
        printf("Num of GPUs %d:\n\n", num_gpus);
        printf("Memory info:\n");
        
        failure=pim_get_device_info(list_of_pims[i], PIM_MEM_CAPACITY, sizeof(uint32_t), &mem_size, &ret_size);
        ASSERT_PIM_RETURN(failure);
        failure=pim_get_device_info(list_of_pims[i], PIM_MEM_MBPS, sizeof(uint32_t), &mem_mbps, &ret_size);
        ASSERT_PIM_RETURN(failure);
        failure=pim_get_device_info(list_of_pims[i], PIM_MEM_NUM_CHANNELS, sizeof(int), &num_channels, &ret_size);
        ASSERT_PIM_RETURN(failure);
        
        printf("MEMORY SIZE(MB): %d\n",mem_size);
        printf("MEMORY GIGABITS PER SECOND: %5.2f\n", (double)mem_mbps/1000);
        printf("NUM CHANNELS: %d\n\n", num_channels);

        if ( num_gpus > 0 ) {
            uint32_t num_cus;
            uint32_t eng_clock;
            
            failure=pim_get_device_info(list_of_pims[i], PIM_GPU_NUM_CUS, sizeof(uint32_t), &num_cus, &ret_size);
            ASSERT_PIM_RETURN(failure);
            failure=pim_get_device_info(list_of_pims[i], PIM_GPU_FREQ, sizeof(uint32_t), &eng_clock, &ret_size);
            ASSERT_PIM_RETURN(failure);            

            printf("GPU %d:\n", 0);
            printf("NUM COMPUTE UNITS: %d\n", num_cus);
            printf("ENGINE CLOCK: %5.2f\n\n", (double)eng_clock/1000 );
            
		}

        if ( num_cpus > 0 ) {
            uint32_t eng_clock;
            
            failure=pim_get_device_info(list_of_pims[i], PIM_CPU_FREQ, sizeof(uint32_t), &eng_clock, &ret_size);
            ASSERT_PIM_RETURN(failure);

            printf("CPU %d:\n", 0);
            printf("ENGINE CLOCK: %5.2f\n\n", (double)eng_clock/1000 );
        }
    }

    return;
}

void pim_spawn_args(void **args,size_t *sizes, size_t *nargs, char *ocl_source, char *build_flags, int dim, size_t glbl[3], size_t lcl[3], int num_pre_event, cl_event * pre_event_list, cl_event *post_event_list)
{
    size_t nargs_tmp=0;
    args[nargs_tmp] =  ocl_source;
    sizes[nargs_tmp] = sizeof(char*);
    nargs_tmp++;

    //build program flags
    args[nargs_tmp] =  build_flags;
    sizes[nargs_tmp] = sizeof(char*);
    nargs_tmp++;

    //work dim	
    args[nargs_tmp] =  &dim;
    sizes[nargs_tmp] = sizeof(int);
    nargs_tmp++;

    /*//global work size
    args[nargs_tmp] =  glbl[0];
    sizes[nargs_tmp] = sizeof(size_t) * dim;
    nargs_tmp++;

    //local work size
    args[nargs_tmp] =  lcl[0];
    sizes[nargs_tmp] = sizeof(size_t) * dim;
    nargs_tmp++;

// event arguments
// pre-events
    args[nargs_tmp] =  &num_pre_event;
    sizes[nargs_tmp] = sizeof(int);
    nargs_tmp++;

// pre-events
    if(pre_event_list==NULL){
        args[nargs_tmp] =  NULL;
        sizes[nargs_tmp] = 0;
        nargs_tmp++;
    }
    else {
        args[nargs_tmp] =  pre_event_list;
        sizes[nargs_tmp] = sizeof(cl_event);
        nargs_tmp++;
    }
// post-events
    if(post_event_list==NULL){
        args[nargs_tmp] =  NULL;
        sizes[nargs_tmp] = 0;
        nargs_tmp++;
    }
    else {
        args[nargs_tmp] =  post_event_list;
        sizes[nargs_tmp] = sizeof(cl_event);
        nargs_tmp++;
    }
    */
    
    *nargs=nargs_tmp;
}
