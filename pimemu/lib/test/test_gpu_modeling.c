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
#ifdef EMULATE_PTHREADS
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include <pim.h>
#include <string.h>



#define ARRAY_DIM_SIZE 200
#define ARRAY_SIZE (ARRAY_DIM_SIZE*ARRAY_DIM_SIZE*ARRAY_DIM_SIZE)
#define __DEFAULT_DIR__ "./test/"
#define __DEFUALT_CONF__ "1.xml"
#define TYPE       float 


#ifdef _NO_CPU_COUNTERS_
#define DEVELOPMENT 1
#if DEVELOPMENT
#define PRINT_DEBUG(...) fprintf(stdout,"DEBUG : ");\
                         fprintf(stdout,## __VA_ARGS__); \
                         fprintf(stdout,"\n");  \
                         fflush(stdout);
#define DEBUG(...) __VA_ARGS
#else
#define PRINT_DEBUG(...)
#define DEBUG(...)
#endif

#define PRINT_ERROR(...) fprintf(stderr, "ERROR in file %s line %d func %s : ",__FILE__,__LINE__,__func__ ); \
                         fprintf(stderr,## __VA_ARGS__);\
                         fprintf(stderr,"\n");\
                         exit(-1);
#define PRINT_INFO(...)  fprintf(stdout," INFO: "); \
                         fprintf(stdout,## __VA_ARGS__); \
                         fprintf(stdout, "\n");\
                         fflush(stdout);
#define PRINT_LOG(__logfile, ...) fprintf(__logfile,## __VA_ARGS__);    \
                                  fprintf(__logfile,"\n"); \
                                  fflush(__logfile);

#endif

// The map from the list of threads (sequential list) to PIM device ID.
static pim_device_id* list_of_pims;
static int * gpus_per_pim = 0;

#ifndef _NO_CPU_COUNTERS_

#include <cpu_model.h>
/***************************************************************************/
/*******  PAPI utilities ***************************************************/
/***************************************************************************/
void init_papi(int *EventSet, int *events) {

  if ( (retval=PAPI_create_eventset( EventSet ))!=PAPI_OK){
        exit(-1);
  }

  if( (retval=PAPI_event_name_to_code("RETIRED_INSTRUCTIONS:u:c=1:i=0",&events[0])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 1");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("RETIRED_INSTRUCTIONS:u:c=0",&events[1])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 2");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("CPU_CLK_UNHALTED:u",&events[2])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 3");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("L2_CACHE_MISS:ALL:u",&events[3])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 4");
        exit(-1);
  }
  if( (retval=PAPI_event_name_to_code("L2_CACHE_FILL_WRITEBACK:L2_WRITEBACKS:u",&events[4])) != PAPI_OK)
  {
        PAPI_perror("Error PAPI_event_name_to_code 5");
        exit(-1);
  }

  if ( (retval=PAPI_add_events(*EventSet, events, NUM_EVENTS))!=PAPI_OK){
        PAPI_perror("PAPI_add_events in pthread");
        exit(-1);
  }


}
void print_counters(int exit_point, int id, long long elapsed_us, long long elapsed_cyc, int *events, long long *values, char *event_name ) {




  printf("%d, 0x%x, %lld, %lld",exit_point,id , elapsed_us,elapsed_cyc);
   
  for(i=0;i<NUM_EVENTS;i++) {
        
        printf(", %lld", values[i]);
  }
  printf("\n");

}
void start_counters(long long *elapsed_us, long long *elapsed_cyc, int EventSet) {

  *elapsed_us = PAPI_get_real_usec(  );
  *elapsed_cyc = PAPI_get_real_cyc(  );
  
  if ( (retval = PAPI_start(EventSet)) != PAPI_OK ){
        PAPI_perror("PAPI_start");
        exit(-1);
  }

  return;
}

void stop_counters(long long *elapsed_us, long long *elapsed_cyc, int EventSet, long long *values) {

  if( (retval = PAPI_stop(EventSet, values) ) != PAPI_OK ){
        PAPI_perror("PAPI_stop");
        exit(-1);
    }
  
    *elapsed_us = PAPI_get_real_usec(  ) - *elapsed_us;
    *elapsed_cyc = PAPI_get_real_cyc(  ) - *elapsed_cyc;

    return;
}

#endif


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

void initialize_pim(char *filename)
{
    int failure = PIM_SUCCESS;
    failure = pim_init(filename);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): Failed to initialize PIM emulator.\n",
                __func__, __LINE__);
        if (failure == PIM_INIT_FILE_ACCESS_ERROR) {
            fprintf(stderr, "XML file access error.\n");
        }
        else if (failure == PIM_INIT_FILE_SYNTAX_ERROR) {
            fprintf(stderr, "XML file syntax error\n");
        }
        else { // failure == PIM_GENERAL_FAILURE
            fprintf(stderr, "General error\n");
        }
        exit(-1);
    }
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

int main(int argc, char **argv)
{
    int i = 0, k;

	size_t proble_size = ARRAY_SIZE;
	int num_gpus;
	int num_threads;
    int verification_fails = 0;

    // Parallel vector linear combination arrays
    void **parallel_input_array;
    void **parallel_output_array;
    TYPE **seq_output_array;

    TYPE **parallel_mapped_input_array;
    TYPE **parallel_mapped_output_array;

    // amote of work per GPU
    size_t amount_to_move = 0;



// kernel compiler arguments
const char * source_nm = "minife.cl";
const char * kernel_nm = "waxpby";
pim_f gpu_kernel;

char* ocl_source;
const void * args[1024];
size_t sizes[1024];
size_t nargs = 0;
int dim = 1;
size_t lcl[3] = {64};
size_t glbl[3]; 
int num_pre_event = 0;


float alpha = 0.3f;
float beta  = 0.5f;

// arguments
int cpu_counters = 0;
int ehp_only = 0;
int verification = 1;
int a;

    printf("**************************************************\n");
    printf("Beginning GPU PIM Spawn Test\n");

    /* Get PIM information */
    if (argc < 2) {
        fprintf(stderr, "Requires a directory to search for XML file as its argument.\n");
        exit(-1);
    }

	for(a = 2; a < argc; a++)
	{
// CPU counters?
	    if ( !strcmp(argv[a], (char*)"-ccounters") )
     	{
		     cpu_counters = 1;
	    }
	   else if ( !strcmp(argv[a], (char*)"-noverify") )
	   {
		      verification = 0;
	   }

	   else if ( !strcmp(argv[a], (char*)"-ehponly") )
	   {
		     ehp_only = 1;
	   }
	}
    printf("Arguments:\n");
	for(a = 1; a < argc; a++)
	{
		printf(argv[a]);
		printf("\n");
	}
// PIM PER MODEL
    initialize_pim(argv[1]);
    num_threads = find_and_map_pims(&num_gpus);
  //  printf("Number of threads: %d\n", num_threads);



    // FIXME: I believe this will not quite work if the array size and num_threads don't
    // divide evently. We will leave the remainder uselessly dangling at the end of the last
    // partial array. FIXME LATER
    if (num_threads != 0 && num_gpus != 0)
	{
		num_gpus = (( ehp_only) ? 1 : num_gpus);
        amount_to_move = proble_size/num_gpus;
	}
    else {
        fprintf(stderr, "This test must be run with at least one PIM-GPU.\n");
        exit(-1);
    }
   
#ifndef _NO_CPU_COUNTERS_
// CPU PERF COUNTERS
	if ( cpu_counters )
	{
    //*** Init Papi Library and register it to support pthreads **********
         if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ){
               PAPI_perror("PAPI_library_init");
       	       exit(-1);
         }
         if ( ( retval =PAPI_thread_init( ( unsigned long ( * )( void ) ) ( pthread_self ) ) ) != PAPI_OK )
		 {
               PAPI_perror("PAPI_thread_init");
               exit(-1);
         }

         init_papi(&EventSet, &events[0]);
         printf("\n");
         printf("Exec_Order, ThreadId, elapsed_us, elapsed_uc");
         printf(", COMMIT_CYC, INSTRUCTIONS, CLOCK_CYC, L2_CACHE_MISS, L2_WRITEBACKS");
         printf("\n");

    // **** Start Mark 1 *********
        start_counters(&elapsed_us, &elapsed_cyc, EventSet);
	}

#endif


#ifdef _PROPERTIES_
	// Properties
	printf("\nProperies:\n\n");

    for (i = 0, k = 0; i < num_threads; i++) {
 

		if ( gpus_per_pim[i] > 0 )
		{
         short num_cus, num_channels;
		 short eng_clock;
		 short mem_mbps;
		 size_t ret_size;
			pim_get_device_info(i, PIM_GPU_NUM_CUS, sizeof(int), &num_cus, &ret_size);
			pim_get_device_info(i, PIM_GPU_FREQ, sizeof(short), &eng_clock, &ret_size);
			pim_get_device_info(i, PIM_MEM_MBPS, sizeof(short), &mem_mbps, &ret_size);
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
// 2 inputs per kernel
    parallel_input_array = (void**)malloc(sizeof(void *) * num_gpus * 2);
	parallel_mapped_input_array = (TYPE**)malloc(sizeof(TYPE *) * num_gpus * 2);
// 1 output
    parallel_output_array = (void**)malloc(sizeof(void *) * num_gpus);
	parallel_mapped_output_array = (TYPE**)malloc(sizeof(TYPE *) * num_gpus);

// verification arrays
    seq_output_array = (TYPE**)malloc(sizeof(TYPE *) * num_gpus);


// FIXME: CHECK SUCCESS
    for (i = 0, k = 0; i < num_threads && k < num_gpus; i++) {
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
    for (i = 0; i < num_gpus * 2; i++) {
         parallel_mapped_input_array[i] = (TYPE*)pim_map(parallel_input_array[i],PIM_PLATFORM_OPENCL_GPU);
	     assert(parallel_mapped_input_array[i]);
       	 fillWithRand(parallel_mapped_input_array[i], amount_to_move, 1, 0., 255., 0);
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
					   amount_to_move);
    }
 }
    printf("Unmap input buffers...\n");
// make data accessable by GPU
    for (i = 0; i < num_gpus * 2; i++) {
        pim_unmap(parallel_mapped_input_array[i],parallel_input_array[i]);
    }


// launch the kernel
   printf("Parallel Ax + By...\n");

	  gpu_kernel.func_name = (void*)kernel_nm;

	{
    char *directory;
	    directory = find_directory(argv[1]);
	    ocl_source = concat_directory_and_name(directory, source_nm);
	  
       args[nargs] =  ocl_source;
	   free(directory);
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

#ifndef _NO_CPU_COUNTERS_
	if ( cpu_counters )
	{
    //**** Exit Mark 1 *****
         stop_counters(&elapsed_us, &elapsed_cyc,EventSet, &values[0]);
         print_counters(1, 0 ,elapsed_us,elapsed_cyc,&events[0],&values[0], &event_name[0]);

    // **** Start Mark 2 *********
         start_counters(&elapsed_us, &elapsed_cyc, EventSet);
	}
#endif

    for (i = 0, k = 0; i < num_threads && k < num_gpus; i++) {
		if ( gpus_per_pim[i] > 0 )
		{
        size_t tnargs = nargs;
			args[tnargs] = parallel_input_array[k*2];
	        sizes[tnargs++] = sizeof(void*);
			args[tnargs] = parallel_input_array[k*2 + 1];
	        sizes[tnargs++] = sizeof(void*);
			args[tnargs] = parallel_output_array[k];
	        sizes[tnargs++] = sizeof(void*);

			pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, list_of_pims[i], PIM_PLATFORM_OPENCL_GPU);

			k++;
		}
    }

#ifndef _NO_CPU_COUNTERS_
	if ( cpu_counters )
	{
    //**** Exit Mark 2 *****
         stop_counters(&elapsed_us, &elapsed_cyc,EventSet, &values[0]);
         print_counters(2, 0 ,elapsed_us,elapsed_cyc,&events[0],&values[0], &event_name[0]);

    // **** Start Mark 3 *********
         start_counters(&elapsed_us, &elapsed_cyc, EventSet);
	}
#endif

	free(ocl_source);

 if ( verification )
 {
    printf("Checking results.\n");


	verification_fails = 0;


    for (i = 0; i < num_gpus &&  !verification_fails; i++) {
	int j;
    
         parallel_mapped_output_array[i] = (float*)pim_map(parallel_output_array[i],PIM_PLATFORM_OPENCL_GPU);

        for (j = 0; j < amount_to_move; j++)
		{

 
 
		    unsigned int h = *(unsigned int*)&seq_output_array[i][j];
			unsigned int g = *(unsigned int*)&parallel_mapped_output_array[i][j];

            verification_fails = (abs((int)h - (int)g) > 3);
            if ( verification_fails ) {
                fprintf(stderr,
                        "Parallel Ax + By. First differs at Index %d,%d. %f != %f\n",
                        i, j,
						seq_output_array[i][j],
                        parallel_mapped_output_array[i][j]);
                break;
            }
        }
		pim_unmap(parallel_mapped_output_array[i],parallel_output_array[i]);

    }

	if (!verification_fails )
	{
	    printf("Success!\n");
	}
 }

// delete buffers
	printf("Deleting buffers...\n");


    for (i = 0; i < num_gpus * 2; i++) {
        pim_free(parallel_input_array[i]);
    }
    for (i = 0; i < num_gpus; i++) {
        pim_free(parallel_output_array[i]);
        free(seq_output_array[i]);
    }

    free(seq_output_array);
    free(parallel_input_array);
    free(parallel_mapped_input_array);
    free(parallel_output_array);
    free(parallel_mapped_output_array);

#ifndef _NO_CPU_COUNTERS_
	if ( cpu_counters )
	{
    //**** Exit Mark 3 *****
         stop_counters(&elapsed_us, &elapsed_cyc,EventSet, &values[0]);
         print_counters(3, 0 ,elapsed_us,elapsed_cyc,&events[0],&values[0], &event_name[0]);


	    PAPI_shutdown();
	}
#endif
    return 0;
}
