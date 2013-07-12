#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#ifdef WIN32
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include <CL/cl.h>
#include <pim.h>
#include <string.h>

#include <utility.c>

#define ARRAY_DIM_SIZE 400
#define ARRAY_SIZE (ARRAY_DIM_SIZE*ARRAY_DIM_SIZE*ARRAY_DIM_SIZE)
#define __DEFAULT_DIR__ "./test/"
#define __DEFUALT_CONF__ "1.xml"
#define TYPE       float 



pim_device_id* list_of_pims;
//pim_device_id* list_of_cpus;
//pim_device_id* list_of_gpus;
int * gpus_per_pim;
int * cpus_per_pim;


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


int main(int argc, char **argv)
{
    int i,k;
    size_t problem_size = ARRAY_SIZE;
	int num_gpus;
    int num_cpus;
    int num_pims;
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
char * source_nm = "lifan_test.cl";
char * kernel_nm = "test";
pim_f gpu_kernel;

char* ocl_source;
void * args[1024];
size_t sizes[1024];
size_t nargs = 0;
int dim = 1;
size_t lcl[3] = {64};
size_t glbl[3]; 
int num_pre_event = 0;
cl_event *pim_completion = 0;
cl_int clerr = CL_SUCCESS;

int alpha = 2;


    // PIM PER MODEL
    //num_threads = find_and_map_pims(&num_gpus, &num_cpus, &gpus_per_pim,&cpus_per_pim, &list_of_pims);
    num_pims = find_pims();
    num_threads=num_pims;
    list_of_pims = malloc(num_pims * sizeof(pim_device_id));
    gpus_per_pim = (int*)malloc(num_pims * sizeof(int));
    cpus_per_pim = (int*)malloc(num_pims * sizeof(int));
    map_pims(num_pims, &num_gpus, &num_cpus, gpus_per_pim, cpus_per_pim, list_of_pims);
    //list_of_cpus = malloc(num_cpus * sizeof(pim_device_id));
    //list_of_gpus = malloc(num_gpus * sizeof(pim_device_id));
    printf("Number of threads: %d\n", num_threads);
    printf("Number of GPUs: %d\n",num_gpus);
    printf("Number of CPUs: %d\n",num_cpus);
    //for(i=0;i<num_threads;i++) printf("%d\n",list_of_pims[i]);
    pim_property(num_pims, gpus_per_pim, cpus_per_pim, list_of_pims);


    amount_to_move = problem_size/num_gpus;
    if(problem_size%num_gpus!=0){
        fprintf(stderr, "Uneven workload decompostion.\n");
        exit(-1);
    }

    // Allocates the per-PIM structures.
// alloctae cl control data
	pim_completion = (cl_event *) calloc(num_gpus, sizeof(cl_event));
	assert(pim_completion);

// 2 inputs per kernel
    parallel_input_array = (void**)malloc(sizeof(void *) * num_gpus );
	parallel_mapped_input_array = (TYPE**)malloc(sizeof(TYPE *) * num_gpus );
// 1 output
    parallel_output_array = (void**)malloc(sizeof(void *) * num_gpus);
	parallel_mapped_output_array = (TYPE**)malloc(sizeof(TYPE *) * num_gpus);

// verification arrays
    seq_output_array = (TYPE**)malloc(sizeof(TYPE *) * num_gpus);


// FIXME: CHECK SUCCESS
    for (i = 0, k = 0; i < num_threads && k < num_gpus; i++) {
		if ( gpus_per_pim[i] > 0 )
		{
             parallel_input_array[k] = pim_malloc(sizeof(TYPE) * amount_to_move, list_of_pims[i], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
		     assert(parallel_input_array[k]);
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
    for (i = 0; i < num_gpus; i++) {
         parallel_mapped_input_array[i] = (TYPE*)pim_map(parallel_input_array[i],PIM_PLATFORM_OPENCL_GPU);
	     assert(parallel_mapped_input_array[i]);
       	 fillWithRand(parallel_mapped_input_array[i], amount_to_move, 1, 0., 255., 0);
    }

//  PERF COUNTERS
	// **** Start Mark  *********
	pim_emu_begin();

    printf("Calculating A=B*x array, sequentially...\n");
    for (i = 0; i < num_gpus; i++) {
        for(int j=0;j<amount_to_move;j++){
            seq_output_array[i][j]=parallel_mapped_input_array[i][j]*alpha;
        }
    }

    printf("Unmap input buffers...\n");
// make data accessable by GPU
    for (i = 0; i < num_gpus; i++) {
        pim_unmap(parallel_mapped_input_array[i]);
    }


// launch the kernel
   printf("Parallel Ax + By...\n");

    gpu_kernel.func_name = (void*)kernel_nm;

    //glbl[0] = amount_to_move%lcl[0]==0?amount_to_move:((amount_to_move/lcl[0]) + 1) * lcl[0];
    //pim_spawn_args(args,sizes, &nargs, source_nm, " -D __GPU__", dim, glbl, lcl, num_pre_event, NULL, NULL);

    //kernel source file
    ocl_source = concat_directory_and_name("./", source_nm);
    ocl_source = source_nm;
    args[nargs] =  ocl_source;
    sizes[nargs] = sizeof(char*);
    nargs++;

    //build program flags
    args[nargs] =  " -D __GPU__";
    sizes[nargs] = sizeof(char*);
    nargs++;

    //work dim	
    args[nargs] =  &dim;
    sizes[nargs] = sizeof(int);
    nargs++;

    glbl[0] = amount_to_move%lcl[0]==0?amount_to_move:((amount_to_move/lcl[0]) + 1) * lcl[0];

    //global work size
    args[nargs] =  glbl;
    sizes[nargs] = sizeof(size_t) * dim;
    nargs++;

    //local work size
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

    args[nargs] = &alpha;
    sizes[nargs] = sizeof(int);
    nargs++;

    args[nargs] = &amount_to_move;
    sizes[nargs] = sizeof(int);
    nargs++;

for(int l=0;l<100;l++){
    for (i = 0, k = 0; i < num_threads && k < num_gpus; i++) {
        if ( gpus_per_pim[i] > 0 )
        {
            size_t tnargs = nargs;

            args[tnargs] = parallel_input_array[k];
            sizes[tnargs] = sizeof(void*);
            tnargs++;

            args[tnargs] = parallel_output_array[k];
            sizes[tnargs] = sizeof(void*);
            tnargs++;

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
}


    printf("Checking results.\n");


    verification_fails = 0;


    for (i = 0; i < num_gpus &&  !verification_fails; i++) {
        int j;
    
        parallel_mapped_output_array[i] = (float*)pim_map(parallel_output_array[i],PIM_PLATFORM_OPENCL_GPU);

        for (j = 0; j < amount_to_move; j++){

            double h = (double)seq_output_array[i][j];
            double g = (double)parallel_mapped_output_array[i][j];

            verification_fails = (abs(h - g) /g) > 0.0000001 ;
            if ( verification_fails ) {
                fprintf(stderr,"Parallel Ax + By. First differs at Index %d,%d. %f != %f\n",i, j,seq_output_array[i][j],parallel_mapped_output_array[i][j]);
                break;
            }
        }
		pim_unmap(parallel_mapped_output_array[i]);

    }

	if (!verification_fails )
	{
	    printf("Success!\n");
	}


    pim_emu_end();


// delete buffers
    printf("Deleting buffers...\n");


    
    for (i = 0; i < num_gpus; i++) {
        pim_free(parallel_input_array[i]);
        pim_free(parallel_output_array[i]);
        free(seq_output_array[i]);
//		free(parallel_mapped_output_array[i]);
    }

    free(seq_output_array);
    free(parallel_input_array);
    free(parallel_mapped_input_array);
    free(parallel_output_array);
    free(parallel_mapped_output_array);


    return 0;
}








