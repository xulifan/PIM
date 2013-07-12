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
#include <math.h>
#ifdef EMULATE_PTHREADS
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include <CL/cl.h>
#include <pim.h>
#include <string.h>


#define ARRAY_DIM_SIZE 200
#define ARRAY_SIZE (ARRAY_DIM_SIZE*ARRAY_DIM_SIZE*ARRAY_DIM_SIZE)
#define TYPE       float 

// The map from the list of threads (sequential list) to PIM device ID.
static pim_device_id* list_of_pims;
static int * gpus_per_pim = 0;
static int * cpus_per_pim = 0;

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


/*****************************************************************************/
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

	return_str = (char *) malloc(sizeof(char) * (directory_length + name_length + 1));
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

	directory = (char *) malloc(sizeof(char) * (directory_length + 1));
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

	directory = (char *)malloc(sizeof(char) * (directory_length + 1));

	strncpy(directory, input, directory_length);
	directory[directory_length] = 0;
	return(directory);
}

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
	list_of_pims = (pim_device_id *) malloc(num_pims * sizeof(pim_device_id));
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
	//*num_gpus = number_of_pim_cpus;
	return num_pims;
}


// Basic function for performing a prefix some from some base address
// through some length. Note that this is not an in-place prefix sum
// The output is stored into out_array.
void prefix_sum(uint32_t *input, unsigned int length, uint64_t *out_array)
{
	int i = 1;
	if (length == 0 || input == NULL || out_array == NULL)
		return;
	if (length >= 1)
		out_array[0] = input[0];
	while (i < (int)length) {
		out_array[i] = input[i] + out_array[i-1];
		i++;
	}
}

typedef struct {
	uint32_t *input;
	unsigned int length;
	uint64_t *out_array;
} prefix_sum_input_t;

// Called by pthreads, so needs a new fnction prototype. This stage is
// just a wrapper for the regular prefix_sum() above.
void *parallel_prefix_sum(void *args)
{
	prefix_sum_input_t *temp = (prefix_sum_input_t *)args;
	prefix_sum(temp->input, temp->length, temp->out_array);
	return NULL;
}

typedef struct {
	uint64_t input;
	unsigned int length;
	uint64_t *out_array;
} prefix_sum_second_input_t;

// The naive PPS algorithm used here splits the giant array into num_threads
// equal parts, and the threads do local prefix sum seperately. A
// smaller "prefix sum of last values of each local collection" is then done.
// This second part of the algorithm adds those needed values into the
// output array in order to make the correct numbers.
void *parallel_prefix_sum_second(void *args)
{
	int i = 0;
	uint64_t input, *out_array;
	unsigned int length;
	prefix_sum_second_input_t *temp = (prefix_sum_second_input_t *)args;
	out_array = temp->out_array;
	input = temp->input;
	length = temp->length;
	if (length == 0 || input == 0 || out_array == NULL)
		return NULL;
	while (i < (int)length) {
		out_array[i] = out_array[i] + input;
		i++;
	}
	return NULL;
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

int main(int argc, char **argv)
{

	int i = 0, j = 0;

	// Serial prefix sum arrays
	uint32_t *input_array;
	uint64_t *output_array;
	
	// Parallel prefix sum arrays
	uint32_t **parallel_input_array;
	uint64_t **parallel_output_array;
	uint64_t *intermediate_vals;

	// The number of variables that each thread will work on
	int amount_to_move = 0;

	// Variables needed for spawning off the parallel prefix sum threads
	int num_threads;
	int num_gpus;
	pthread_t *pim_threads;
	void *pim_args[NUM_PTHREAD_ARGUMENTS];
	size_t arg_size[NUM_PTHREAD_ARGUMENTS];
	pim_f pim_function;
	prefix_sum_input_t *thread_input;
	prefix_sum_second_input_t *second_thread_input;

	void *spawn_error = NULL;

// arguments
int counters= 0;
int ehp_only = 0;
int verification = 1;
int loops = 1;
float dummy0 = 0;
int a;

    if (argc < 2) {
        fprintf(stderr, "Requires a directory to search for .cl file as its argument.\n");
        exit(-1);
    }

	for(a = 2; a < argc; a++)
	{
// counters?
	    if ( !strcmp(argv[a], (char*)"-ccounters") )
     	{
		     counters= 1;
	    }
	   else if ( !strcmp(argv[a], (char*)"-noverify") )
	   {
		      verification = 0;
	   }
	   
	   else if ( !strcmp(argv[a], (char*)"-loop") && a+ 1 < argc )
	   {
		      loops = atoi(argv[++a]);
	   }
	}

    printf("Arguments:\n");
	for(a = 1; a < argc; a++)
	{
		printf(argv[a]);
		printf("\n");
	}

	srand(0);

	printf("**************************************************\n");
	printf("Beginning CPU-PThreads PIM Spawn Test\n");




	num_threads = find_and_map_pims(&num_gpus);
	printf("Number of threads: %d\n", num_threads);

	// FIXME: I believe this will not quite work if the array size and num_threads don't
	// divide evently. We will leave the remainder uselessly dangling at the end of the last
	// partial array. FIXME LATER
	if (num_threads != 0)
		amount_to_move = ARRAY_SIZE/num_threads;
	else {
		fprintf(stderr, "This test must be run with at least one PIM.\n");
		exit(-1);
	}
	// Allocates the per-PIM structures.
	parallel_input_array = (uint32_t **) calloc(sizeof(uint32_t *), num_threads);
	parallel_output_array = (uint64_t **)calloc(sizeof(uint64_t *), num_threads);

	assert(parallel_input_array
		&& parallel_output_array);

	for (i = 0; i < num_threads; i++) {
		parallel_input_array[i] = (uint32_t *)pim_malloc(sizeof(uint32_t) * amount_to_move, list_of_pims[i], PIM_MEM_DEFAULT_FLAGS, PIM_PLATFORM_PTHREAD_CPU);
		parallel_output_array[i] = (uint64_t *)pim_malloc(sizeof(uint64_t) * amount_to_move, list_of_pims[i], PIM_MEM_DEFAULT_FLAGS, PIM_PLATFORM_PTHREAD_CPU);
		assert(parallel_input_array[i] 
		        && parallel_output_array[i]);
	}

	input_array = (uint32_t *) calloc(sizeof(uint32_t), ARRAY_SIZE);
	output_array = (uint64_t *) calloc(sizeof(uint64_t), ARRAY_SIZE);
	pim_threads = (pthread_t *)calloc(sizeof(pthread_t), num_threads);
	thread_input = (prefix_sum_input_t*)calloc(sizeof(prefix_sum_input_t), num_threads);
	second_thread_input = (prefix_sum_second_input_t*)calloc(sizeof(prefix_sum_second_input_t), num_threads);
	intermediate_vals = (uint64_t*)calloc(sizeof(uint64_t), num_threads);

	assert(	input_array 
	        && output_array
	        && pim_threads
	        && thread_input
	        && second_thread_input
	        && intermediate_vals);

	printf("Generating initial array...");



	fflush(stdout);
	for (i = 0; i < ARRAY_SIZE; i++) {
		input_array[i] = rand() % 1000;
		output_array[i] = 0;
	}
	for (i = 0; i < num_threads; i++) {
		for (j = 0; j < amount_to_move; j++) {
			parallel_input_array[i][j] = input_array[(i*amount_to_move)+j];
			parallel_output_array[i][j] = 0;
		}
	}
	printf("Calculating serial parallel prefix...\n");
	fflush(stdout);
	// Serial Version for Corectness Checking
	prefix_sum(input_array, ARRAY_SIZE, output_array);

// START CLOCK COUNTING

	pim_emu_begin();



	printf("Parallel prefix stage 1...");
	fflush(stdout);
	// First Parallel Part
	for (i = 0; i < num_threads; i++) {
		(thread_input[i]).input = parallel_input_array[i];
		(thread_input[i]).length = amount_to_move;
		(thread_input[i]).out_array = parallel_output_array[i];
		pim_args[PTHREAD_ARG_THREAD] = &(pim_threads[i]); // pthread_t
		arg_size[PTHREAD_ARG_THREAD] = sizeof(pthread_t);
		pim_args[PTHREAD_ARG_ATTR] = NULL; // pthread_attr_t
		arg_size[PTHREAD_ARG_ATTR] = sizeof(pthread_attr_t);
		pim_args[PTHREAD_ARG_INPUT] = &(thread_input[i]); // void * for thread input
		arg_size[PTHREAD_ARG_INPUT] = sizeof(void *);
		pim_function.func_ptr = parallel_prefix_sum;
		spawn_error = pim_spawn(pim_function, pim_args, arg_size, NUM_PTHREAD_ARGUMENTS, list_of_pims[i], PIM_PLATFORM_PTHREAD_CPU);
		if (spawn_error == NULL)
			exit(-1);
	}
	for (i = 0; i < num_threads; i++) {
		pthread_join(pim_threads[i], NULL);
	}

	printf("stage 2...");
	fflush(stdout);
	// Reduce part
	intermediate_vals[0] = parallel_output_array[0][amount_to_move-1];
	for (i = 1; i < (num_threads-1); i++) {
		intermediate_vals[i] = parallel_output_array[i][amount_to_move-1] + intermediate_vals[i-1];
	}

	printf("stage 3...\n");
	fflush(stdout);
	// Add intermediate vals into everything in this PIM's section
	for (i = 1; i < num_threads; i++) {
		(second_thread_input[i]).input = intermediate_vals[i-1];
		(second_thread_input[i]).length = amount_to_move;
		(second_thread_input[i]).out_array = parallel_output_array[i];
		pim_args[PTHREAD_ARG_THREAD] = &(pim_threads[i]); // pthread_t
		arg_size[PTHREAD_ARG_THREAD] = sizeof(pthread_t);
		pim_args[PTHREAD_ARG_ATTR] = NULL; // pthread_attr_t
		arg_size[PTHREAD_ARG_ATTR] = sizeof(pthread_attr_t);
		pim_args[PTHREAD_ARG_INPUT] = &(second_thread_input[i]); // void * for thread input
		arg_size[PTHREAD_ARG_INPUT] = sizeof(void *);
		pim_function.func_ptr = parallel_prefix_sum_second;
		spawn_error = pim_spawn(pim_function, pim_args, arg_size, NUM_PTHREAD_ARGUMENTS, list_of_pims[i], PIM_PLATFORM_PTHREAD_CPU);
		if (spawn_error == NULL)
			exit(-1);
	}

// STOP CLOCK COUNTING

	pim_emu_end();


	free(intermediate_vals);
	intermediate_vals = 0;
	free(second_thread_input);
	second_thread_input = 0;
	free(thread_input);
	thread_input = 0;


	free(input_array);
	input_array = 0;

	for (i = 0; i < num_threads; i++) {

		free(parallel_input_array[i]);
		parallel_input_array[i] = 0;
	}
	free(parallel_input_array);
	parallel_input_array = 0;

	printf("Success!\n");

 
	/**************************************************************/
	/************test gpu opencl with waxpby***********************/
	/**************************************************************/

	int k;

	size_t problem_size = ARRAY_SIZE;
	int verification_fails = 0;
	cl_int clerr = CL_SUCCESS;
	int  amount_to_move_gpu;

	// Parallel vector linear combination arrays
	void **parallel_input_array_gpu;
	void **parallel_output_array_gpu;
	TYPE **seq_output_array;

	TYPE **parallel_mapped_input_array;
	TYPE **parallel_mapped_output_array;

	// amount of work per GPU
	amount_to_move = 0;

// kernel compiler arguments
const char * source_nm = "minife.cl";
const char * kernel_nm = "waxpby_withloop";
pim_f gpu_kernel;

char* ocl_source;
const void * args[1024];
size_t sizes[1024];
size_t nargs = 0;
int dim = 1;
size_t lcl[3] = {64};
size_t glbl[3]; 
int num_pre_event = 0;
cl_event *pim_completion = 0;



float alpha = 0.3f;
float beta  = 0.5f;




    printf("**************************************************\n");
    printf("Beginning GPU PIM Spawn Test\n");





// PIM PER MODEL

    num_threads = find_and_map_pims(&num_gpus);
  //  printf("Number of threads: %d\n", num_threads);



    // FIXME: I believe this will not quite work if the array size and num_threads don't
    // divide evently. We will leave the remainder uselessly dangling at the end of the last
    // partial array. FIXME LATER
    if (num_threads != 0 && num_gpus != 0)
	{
		num_gpus = (( ehp_only) ? 1 : num_gpus);
        amount_to_move_gpu = (int)(problem_size/num_gpus);
	}
    else {
        fprintf(stderr, "This test must be run with at least one PIM-GPU.\n");
        exit(-1);
    }
   






#ifdef _PROPERTIES_
	// Properties
	printf("\nProperies:\n\n");

    for (i = 0, k = 0; i < num_threads; i++) {
 

		if ( gpus_per_pim[i] > 0 )
		{
         uint32_t num_cus, num_channels;
		 uint32_t eng_clock;
		 uint32_t mem_mbps;
		 size_t ret_size;
			pim_get_device_info(i, PIM_GPU_NUM_CUS, sizeof(uint32_t), &num_cus, &ret_size);
			pim_get_device_info(i, PIM_GPU_FREQ, sizeof(uint32_t), &eng_clock, &ret_size);
			pim_get_device_info(i, PIM_MEM_MBPS, sizeof(uint32_t), &mem_mbps, &ret_size);
			pim_get_device_info(i, PIM_MEM_NUM_CHANNELS, sizeof(int), &num_channels, &ret_size);

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

    // Allocates the per-PIM structures.
// alloctae cl control data
	pim_completion = (cl_event *) calloc(num_gpus, sizeof(cl_event));

// 2 inputs per kernel
    parallel_input_array_gpu = (void**)calloc(sizeof(void *), num_gpus * 2);
	parallel_mapped_input_array = (TYPE**)calloc(sizeof(TYPE *), num_gpus * 2);
// 1 output
    parallel_output_array_gpu = (void**)calloc(sizeof(void *), num_gpus);
	parallel_mapped_output_array = (TYPE**)calloc(sizeof(TYPE *), num_gpus);

// verification arrays
    seq_output_array = (TYPE**)calloc(sizeof(TYPE *), num_gpus);

	assert(pim_completion
		   && parallel_input_array_gpu
		   && parallel_mapped_input_array
		   && parallel_output_array_gpu
		   && parallel_mapped_output_array);


// FIXME: CHECK SUCCESS
    for (i = 0, k = 0; i < num_threads && k < num_gpus; i++) {
		if ( gpus_per_pim[i] > 0 )
		{
             parallel_input_array_gpu[k*2] = pim_malloc(sizeof(TYPE) * amount_to_move_gpu, list_of_pims[i], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

			 parallel_input_array_gpu[k*2 + 1] = pim_malloc(sizeof(TYPE) * amount_to_move_gpu, list_of_pims[i], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

			 parallel_output_array_gpu[k] = pim_malloc(sizeof(TYPE) * amount_to_move_gpu, list_of_pims[i], PIM_MEM_PIM_WRITE | PIM_MEM_HOST_READ, PIM_PLATFORM_OPENCL_GPU);

			 // seq verification
		     seq_output_array[k] = (TYPE*)malloc(sizeof(TYPE) * amount_to_move_gpu);

			  assert(parallel_input_array_gpu[k*2]
			         && parallel_input_array_gpu[k*2 + 1]
					 && parallel_output_array_gpu[k]
					 && seq_output_array[k]);
		     k++;
		}
    }





    printf("Problem size %dx%dx%d...\n",ARRAY_DIM_SIZE, ARRAY_DIM_SIZE, ARRAY_DIM_SIZE );
    printf("Map input baffers.\nGenerating initial arrays...\n" );
    for (i = 0; i < num_gpus * 2; i++) {
         parallel_mapped_input_array[i] = (TYPE*)pim_map(parallel_input_array_gpu[i],PIM_PLATFORM_OPENCL_GPU);
	     assert(parallel_mapped_input_array[i]);
       	 fillWithRand(parallel_mapped_input_array[i], amount_to_move_gpu, 1, 0., 255., 0);
    }



 if ( verification )
 {
    printf("Calculating Ax + By array, sequentially...\n");
    for (i = 0; i < num_gpus; i++) {

		waxpby_kernel( alpha,
		               beta,
					   parallel_mapped_input_array[i*2],
					   parallel_mapped_input_array[i*2 + 1],
					   seq_output_array[i],
					   amount_to_move_gpu);
    }
 }
    printf("Unmap input buffers...\n");
// make data accessable by GPU
    for (i = 0; i < num_gpus * 2; i++) {
        pim_unmap(parallel_mapped_input_array[i]);
    }


// launch the kernel
   printf("Parallel Ax + By...\n");

// START CLOCK COUNTING

	pim_emu_begin();

	  gpu_kernel.func_name = (void*)kernel_nm;

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
	  
        args[nargs] =  ocl_source;

	}
	   sizes[nargs] = sizeof(char*);
	   nargs++;
       args[nargs] =  " -D __GPU__";
	   sizes[nargs] = sizeof(char*);
	   nargs++;

// kernel launch arguments
       args[nargs] =  &dim;
	   sizes[nargs] = sizeof(int);
	   nargs++;


	   glbl[0] = ((amount_to_move_gpu / lcl[0]) + 1) * lcl[0];

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

       args[nargs] = &loops;
	   sizes[nargs] = sizeof(int);
	   nargs++;

       args[nargs] = &dummy0;
	   sizes[nargs] = sizeof(float);
	   nargs++;


       args[nargs] = &amount_to_move_gpu;
	   sizes[nargs] = sizeof(int);
	   nargs++;

       args[nargs] = &alpha;
	   sizes[nargs] = sizeof(TYPE);
	   nargs++;

       args[nargs] = &beta;
	   sizes[nargs] = sizeof(TYPE);
	   nargs++;




    for (i = 0, k = 0; i < num_threads && k < num_gpus; i++) {
		if ( gpus_per_pim[i] > 0 )
		{
        size_t tnargs = nargs;
// insert compeletion event

			args[OPENCL_ARG_POSTEVENT] = pim_completion + k;
			sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

			args[tnargs] = parallel_input_array_gpu[k*2];
	        sizes[tnargs++] = sizeof(void*);
			args[tnargs] = parallel_input_array_gpu[k*2 + 1];
	        sizes[tnargs++] = sizeof(void*);
			args[tnargs] = parallel_output_array_gpu[k];
	        sizes[tnargs++] = sizeof(void*);

// insert compeletion event

			args[OPENCL_ARG_POSTEVENT] = pim_completion + k;
			sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

			pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, list_of_pims[i], PIM_PLATFORM_OPENCL_GPU);

			k++;
		}
    }

// wait for PIM gpus to finish

	clerr = clWaitForEvents(num_gpus, pim_completion) ;

	ASSERT_CL_RETURN(clerr);

	for (i = 1; i < num_threads; i++) {
		pthread_join(pim_threads[i], NULL);
	}


	free(ocl_source);

 if ( verification )
 {
    printf("Checking results.\n");


	verification_fails = 0;


    for (i = 0; i < num_gpus &&  !verification_fails; i++) {
	int j;
    
         parallel_mapped_output_array[i] = (float*)pim_map(parallel_output_array_gpu[i],PIM_PLATFORM_OPENCL_GPU);

        for (j = 0; j < amount_to_move_gpu; j++)
		{

 		    double h = (double)seq_output_array[i][j];
			double g = (double)parallel_mapped_output_array[i][j];

           verification_fails = (fabs(h - g) /g) > 0.0001 ;
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





//	printf("Checking results.\n");
	for (i = 0; i < num_threads; i++) {
		for (j = 0; j < amount_to_move; j++) {
			if (output_array[(amount_to_move*i)+j] != parallel_output_array[i][j]) {
				fprintf(stderr,
					"Parallel Prefix Sum First differs at Index %d,%d. %"
					PRIu64 " != %" PRIu64 "\n", i, j,
					output_array[(amount_to_move*i)+j],
					parallel_output_array[i][j]);
				exit(-1);
			}
		}
	}

	if (!verification_fails )
	{
	    printf("Success!\n");
	}
 }


// STOP CLOCK COUNTING 
       pim_emu_end();


// delete buffers
	printf("Deleting buffers...\n");


	free(pim_threads);
	pim_threads = 0;

	for (i = 0; i < num_threads; i++) {
		free(parallel_output_array[i]);
		parallel_output_array[i] = 0;
	}
	free(parallel_output_array);
	parallel_output_array = 0;
	free(output_array);
	output_array = 0;

    for (i = 0; i < num_gpus * 2; i++) {
		if ( parallel_input_array_gpu[i] )
		{
            pim_free(parallel_input_array_gpu[i]);
		}
    }
    for (i = 0; i < num_gpus; i++) {
		if ( parallel_output_array_gpu[i])
		{
            pim_free(parallel_output_array_gpu[i]);
		}
		if (seq_output_array[i])
		{
           free(seq_output_array[i]);
		}
    }

	if ( seq_output_array)
	{
		free(seq_output_array);
		seq_output_array= 0;
	}

	if ( parallel_input_array_gpu )
	{
       free(parallel_input_array_gpu);
	   parallel_input_array_gpu = 0;
	}
	if (parallel_mapped_input_array)
	{
        free(parallel_mapped_input_array);
		parallel_mapped_input_array = 0;
	}
	if ( parallel_output_array_gpu )
	{
        free(parallel_output_array_gpu);
		parallel_output_array_gpu = 0;
	}
	if ( parallel_mapped_output_array )
	{
       free(parallel_mapped_output_array);
	   parallel_mapped_output_array = 0;
	}
	return 0;
}
