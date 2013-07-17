#define BLOCK_SIZE 16 
#define LIMIT -999

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/time.h>

#ifdef NV //NVIDIA
	#include <oclUtils.h>
#else 
	#include <CL/cl.h>
#endif

//global variables

int blosum62[24][24] = {
{ 4, -1, -2, -2,  0, -1, -1,  0, -2, -1, -1, -1, -1, -2, -1,  1,  0, -3, -2,  0, -2, -1,  0, -4},
{-1,  5,  0, -2, -3,  1,  0, -2,  0, -3, -2,  2, -1, -3, -2, -1, -1, -3, -2, -3, -1,  0, -1, -4},
{-2,  0,  6,  1, -3,  0,  0,  0,  1, -3, -3,  0, -2, -3, -2,  1,  0, -4, -2, -3,  3,  0, -1, -4},
{-2, -2,  1,  6, -3,  0,  2, -1, -1, -3, -4, -1, -3, -3, -1,  0, -1, -4, -3, -3,  4,  1, -1, -4},
{ 0, -3, -3, -3,  9, -3, -4, -3, -3, -1, -1, -3, -1, -2, -3, -1, -1, -2, -2, -1, -3, -3, -2, -4},
{-1,  1,  0,  0, -3,  5,  2, -2,  0, -3, -2,  1,  0, -3, -1,  0, -1, -2, -1, -2,  0,  3, -1, -4},
{-1,  0,  0,  2, -4,  2,  5, -2,  0, -3, -3,  1, -2, -3, -1,  0, -1, -3, -2, -2,  1,  4, -1, -4},
{ 0, -2,  0, -1, -3, -2, -2,  6, -2, -4, -4, -2, -3, -3, -2,  0, -2, -2, -3, -3, -1, -2, -1, -4},
{-2,  0,  1, -1, -3,  0,  0, -2,  8, -3, -3, -1, -2, -1, -2, -1, -2, -2,  2, -3,  0,  0, -1, -4},
{-1, -3, -3, -3, -1, -3, -3, -4, -3,  4,  2, -3,  1,  0, -3, -2, -1, -3, -1,  3, -3, -3, -1, -4},
{-1, -2, -3, -4, -1, -2, -3, -4, -3,  2,  4, -2,  2,  0, -3, -2, -1, -2, -1,  1, -4, -3, -1, -4},
{-1,  2,  0, -1, -3,  1,  1, -2, -1, -3, -2,  5, -1, -3, -1,  0, -1, -3, -2, -2,  0,  1, -1, -4},
{-1, -1, -2, -3, -1,  0, -2, -3, -2,  1,  2, -1,  5,  0, -2, -1, -1, -1, -1,  1, -3, -1, -1, -4},
{-2, -3, -3, -3, -2, -3, -3, -3, -1,  0,  0, -3,  0,  6, -4, -2, -2,  1,  3, -1, -3, -3, -1, -4},
{-1, -2, -2, -1, -3, -1, -1, -2, -2, -3, -3, -1, -2, -4,  7, -1, -1, -4, -3, -2, -2, -1, -2, -4},
{ 1, -1,  1,  0, -1,  0,  0,  0, -1, -2, -2,  0, -1, -2, -1,  4,  1, -3, -2, -2,  0,  0,  0, -4},
{ 0, -1,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -2, -1,  1,  5, -2, -2,  0, -1, -1,  0, -4},
{-3, -3, -4, -4, -2, -2, -3, -2, -2, -3, -2, -3, -1,  1, -4, -3, -2, 11,  2, -3, -4, -3, -2, -4},
{-2, -2, -2, -3, -2, -1, -2, -3,  2, -1, -1, -2, -1,  3, -3, -2, -2,  2,  7, -1, -3, -2, -1, -4},
{ 0, -3, -3, -3, -1, -2, -2, -3, -3,  3,  1, -2,  1, -1, -2, -2,  0, -3, -1,  4, -3, -2, -1, -4},
{-2, -1,  3,  4, -3,  0,  1, -1,  0, -3, -4,  0, -3, -3, -2,  0, -1, -4, -3, -3,  4,  1, -1, -4},
{-1,  0,  0,  1, -3,  3,  4, -2,  0, -3, -3,  1, -1, -3, -1,  0, -1, -3, -2, -2,  1,  4, -1, -4},
{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2,  0,  0, -2, -1, -1, -1, -1, -1, -4},
{-4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  1}
};

// local variables
static cl_context	    context;
static cl_command_queue cmd_queue;
static cl_device_type   device_type;
static cl_device_id   * device_list;
static cl_int           num_devices;

static int initialize(int use_gpu)
{
	cl_int result;
	size_t size;

	// create OpenCL context
	cl_platform_id platform_id;
	if (clGetPlatformIDs(1, &platform_id, NULL) != CL_SUCCESS) { printf("ERROR: clGetPlatformIDs(1,*,0) failed\n"); return -1; }
	cl_context_properties ctxprop[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0};
	device_type = use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;
	context = clCreateContextFromType( ctxprop, device_type, NULL, NULL, NULL );
	if( !context ) { printf("ERROR: clCreateContextFromType(%s) failed\n", use_gpu ? "GPU" : "CPU"); return -1; }

	// get the list of GPUs
	result = clGetContextInfo( context, CL_CONTEXT_DEVICES, 0, NULL, &size );
	num_devices = (int) (size / sizeof(cl_device_id));
	printf("num_devices = %d\n", num_devices);
	
	if( result != CL_SUCCESS || num_devices < 1 ) { printf("ERROR: clGetContextInfo() failed\n"); return -1; }
	device_list = new cl_device_id[num_devices];
	if( !device_list ) { printf("ERROR: new cl_device_id[] failed\n"); return -1; }
	result = clGetContextInfo( context, CL_CONTEXT_DEVICES, size, device_list, NULL );
	if( result != CL_SUCCESS ) { printf("ERROR: clGetContextInfo() failed\n"); return -1; }

	// create command queue for the first device
	cmd_queue = clCreateCommandQueue( context, device_list[0], 0, NULL );
	if( !cmd_queue ) { printf("ERROR: clCreateCommandQueue() failed\n"); return -1; }
	return 0;
}

static int shutdown()
{
	// release resources
	if( cmd_queue ) clReleaseCommandQueue( cmd_queue );
	if( context ) clReleaseContext( context );
	if( device_list ) delete device_list;

	// reset all variables
	cmd_queue = 0;
	context = 0;
	device_list = 0;
	num_devices = 0;
	device_type = 0;

	return 0;
}

int maximum( int a,
		 int b,
		 int c){

	int k;
	if( a <= b )
	  k = b;
	else 
	  k = a;
	if( k <=c )
	  return(c);
	else
	  return(k);
}

void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s <max_rows/max_cols> <penalty> \n", argv[0]);
	fprintf(stderr, "\t<dimension>  - x and y dimensions\n");
	fprintf(stderr, "\t<penalty> - penalty(positive integer)\n");
	fprintf(stderr, "\t<file> - filename\n");
	exit(1);
}

double gettime() {
  struct timeval t;
  gettimeofday(&t,NULL);
  return t.tv_sec+t.tv_usec*1e-6;
}

#include <pim.h>
#include "pim_utility.cpp"
#include "nw_pim.c"
int main(int argc, char **argv){

    int max_rows, max_cols, penalty;
	char * tempchar;
	// the lengths of the two sequences should be able to divided by 16.
	// And at current stage  max_rows needs to equal max_cols
	if (argc == 4)
	{
		max_rows = atoi(argv[1]);
		max_cols = atoi(argv[1]);
		penalty = atoi(argv[2]);
		tempchar = argv[3];
	}
    else{
	     usage(argc, argv);
    }
	
	if(atoi(argv[1])%16!=0){
	fprintf(stderr,"The dimension values must be a multiple of 16\n");
	exit(1);
	}

	max_rows = max_rows + 1;
	max_cols = max_cols + 1;

	int *reference;
	int *input_itemsets;
	int *output_itemsets;
	
	reference = (int *)malloc( max_rows * max_cols * sizeof(int) );
    input_itemsets = (int *)malloc( max_rows * max_cols * sizeof(int) );
	output_itemsets = (int *)malloc( max_rows * max_cols * sizeof(int) );
	
	srand(7);
	
	//initialization 
	for (int i = 0 ; i < max_cols; i++){
		for (int j = 0 ; j < max_rows; j++){
			input_itemsets[i*max_cols+j] = 0;
		}
	}

	for( int i=1; i< max_rows ; i++){    //initialize the cols
			input_itemsets[i*max_cols] = rand() % 10 + 1;
	}
	
    for( int j=1; j< max_cols ; j++){    //initialize the rows
			input_itemsets[j] = rand() % 10 + 1;
	}
	
	for (int i = 1 ; i < max_cols; i++){
		for (int j = 1 ; j < max_rows; j++){
		reference[i*max_cols+j] = blosum62[input_itemsets[i*max_cols]][input_itemsets[j]];
		}
	}

    for( int i = 1; i< max_rows ; i++)
       input_itemsets[i*max_cols] = -i * penalty;
	for( int j = 1; j< max_cols ; j++)
       input_itemsets[j] = -j * penalty;



/********************************************************************/
/******************** PIM initialization ****************************/

    int num_gpus;
    int num_cpus;
    int num_pims;
    int num_threads;
    pim_device_id *target_gpu;

    pim_device_id* list_of_pims;
    int * gpus_per_pim;
    int * cpus_per_pim;

    // PIM PER MODEL
    num_pims = find_pims();
    num_threads=num_pims;
    list_of_pims = (pim_device_id*)malloc(num_pims * sizeof(pim_device_id));
    gpus_per_pim = (int*)malloc(num_pims * sizeof(int));
    cpus_per_pim = (int*)malloc(num_pims * sizeof(int));
    map_pims(num_pims, &num_gpus, &num_cpus, gpus_per_pim, cpus_per_pim, list_of_pims);
    printf("Number of threads: %d\n", num_threads);
    printf("Number of GPUs: %d\n",num_gpus);
    printf("Number of CPUs: %d\n",num_cpus);
    //for(i=0;i<num_threads;i++) printf("%d\n",list_of_pims[i]);
    pim_property(num_pims, gpus_per_pim, cpus_per_pim, list_of_pims);
    
    // only use one GPU, get its id
    target_gpu=(pim_device_id*)malloc(num_gpus*sizeof(pim_device_id));
    int gpu_temp=0;
    for (int i = 0; i < num_threads; i++) {
        int temp=gpus_per_pim[i];
        if(temp>1){
            printf("PIM%d has multiple (%d) GPUs, not supported for now!\n",i,temp);
            exit(-1);
        }
        else if (temp > 0 ){
            target_gpu[gpu_temp]=list_of_pims[i];
            printf("Target GPU is %d\n",target_gpu[gpu_temp]);
            gpu_temp+=1;
        }
    }

/********************************************************************/
/********************************************************************/

    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

	void **pim_input_itemsets;
    void **pim_output_itemsets;
    void **pim_reference;

    int **pim_mapped_input_itemsets;
    int **pim_mapped_output_itemsets;
    int **pim_mapped_reference;

/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_input_itemsets=(void **)malloc(sizeof(void *)*num_gpus);
    pim_output_itemsets=(void **)malloc(sizeof(void *)*num_gpus);
    pim_reference=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_input_itemsets=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_output_itemsets=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_reference=(int **)malloc(sizeof(int *)*num_gpus);

    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/


/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_input_itemsets[cur_gpu] = pim_malloc(max_cols * max_rows * sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_output_itemsets[cur_gpu] = pim_malloc(max_cols * max_rows * sizeof(int), target_gpu[cur_gpu], PIM_MEM_HOST_READ, PIM_PLATFORM_OPENCL_GPU);
        pim_reference[cur_gpu] = pim_malloc(max_cols * max_rows * sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_memcpyHtoD(input_itemsets,pim_input_itemsets[cur_gpu],max_cols * max_rows);
        pim_memcpyHtoD(reference,pim_reference[cur_gpu],max_cols * max_rows);
        //pim_print1d(pim_mapped_input_itemsets[cur_gpu],pim_input_itemsets[cur_gpu],max_cols * max_rows);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/

    int worksize = max_cols - 1;
    printf("worksize = %d\n", worksize);
    int block_width = worksize/BLOCK_SIZE ;
    int offset_r=0;
    int offset_c=0;

    printf("Processing upper-left matrix\n");
    for( int blk = 1 ; blk <= worksize/BLOCK_SIZE ; blk++){
	    
        pim_domain_decomposition(start_point, end_point, own_num_points, num_gpus, blk);
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            printf("GPU %d is calculating %d records from %d to %d for blk %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu],blk);
        }
        
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            if(own_num_points[cur_gpu] ==0) continue;
            // launch the PIM kernel
            pim_launch_nw1_kernel(pim_reference[cur_gpu], pim_input_itemsets[cur_gpu], pim_output_itemsets[cur_gpu], max_cols, penalty, blk, block_width, worksize, offset_r, offset_c, start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            if(own_num_points[cur_gpu] ==0) continue;
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
                
        }

        pim_array_sync(input_itemsets,pim_mapped_input_itemsets,pim_input_itemsets,max_cols * max_rows,num_gpus);
        //print1d(input_itemsets,max_cols*max_rows);
	    //break;
    }

    printf("Processing lower-right matrix\n");
    for( int blk =  worksize/BLOCK_SIZE - 1  ; blk >= 1 ; blk--){
	
        pim_domain_decomposition(start_point, end_point, own_num_points, num_gpus, blk);
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            printf("GPU %d is calculating %d records from %d to %d for blk %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu],blk);
        }

        
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            if(own_num_points[cur_gpu] ==0) continue;
            // launch the PIM kernel
            pim_launch_nw2_kernel(pim_reference[cur_gpu], pim_input_itemsets[cur_gpu], pim_output_itemsets[cur_gpu], max_cols, penalty, blk, block_width, worksize, offset_r, offset_c, start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            if(own_num_points[cur_gpu] ==0) continue;
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
                
        }

        pim_array_sync(input_itemsets,pim_mapped_input_itemsets,pim_input_itemsets,max_cols * max_rows,num_gpus);
        //print1d(input_itemsets,max_cols*max_rows);
        //break;
	
    }
	


/********************************************************************/
/********************************************************************/



/********************************************************************/
/******************** collect resutls from PIMs *********************/  
   
    memcpy(output_itemsets,input_itemsets,max_cols * max_rows * sizeof(int));

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_input_itemsets[cur_gpu]);
        pim_free(pim_output_itemsets[cur_gpu]);
        pim_free(pim_reference[cur_gpu]);
    }

    free(pim_input_itemsets);
    free(pim_output_itemsets);
    free(pim_reference);

    free(pim_mapped_input_itemsets);
    free(pim_mapped_output_itemsets);
    free(pim_mapped_reference);

    free(complete_event);
    free(start_point);
    free(end_point);
    free(own_num_points);

    free(target_gpu);    
    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);

/********************************************************************/
/********************************************************************/

    // **** PIM emulation End Mark  *********  
    pim_emu_end();



#define TRACEBACK	
#ifdef TRACEBACK
	
	FILE *fpo = fopen("result.txt","w");
	fprintf(fpo, "print traceback value GPU:\n");
    
	for (int i = max_rows - 2,  j = max_rows - 2; i>=0, j>=0;){
		int nw, n, w, traceback;
		if ( i == max_rows - 2 && j == max_rows - 2 )
			fprintf(fpo, "%d ", output_itemsets[ i * max_cols + j]); //print the first element
		if ( i == 0 && j == 0 )
           break;
		if ( i > 0 && j > 0 ){
			nw = output_itemsets[(i - 1) * max_cols + j - 1];
		    w  = output_itemsets[ i * max_cols + j - 1 ];
            n  = output_itemsets[(i - 1) * max_cols + j];
		}
		else if ( i == 0 ){
		    nw = n = LIMIT;
		    w  = output_itemsets[ i * max_cols + j - 1 ];
		}
		else if ( j == 0 ){
		    nw = w = LIMIT;
            n  = output_itemsets[(i - 1) * max_cols + j];
		}
		else{
		}

		//traceback = maximum(nw, w, n);
		int new_nw, new_w, new_n;
		new_nw = nw + reference[i * max_cols + j];
		new_w = w - penalty;
		new_n = n - penalty;
		
		traceback = maximum(new_nw, new_w, new_n);
		if(traceback == new_nw)
			traceback = nw;
		if(traceback == new_w)
			traceback = w;
		if(traceback == new_n)
            traceback = n;
			
		fprintf(fpo, "%d ", traceback);

		if(traceback == nw )
		{i--; j--; continue;}

        else if(traceback == w )
		{j--; continue;}

        else if(traceback == n )
		{i--; continue;}

		else
		;
	}
	
	fclose(fpo);

#endif

	printf("Computation Done\n");

	free(reference);
	free(input_itemsets);
	free(output_itemsets);
	
}

