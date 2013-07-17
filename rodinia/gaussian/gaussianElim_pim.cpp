/*------------------------------------------------------
 ** ForwardSub() -- Forward substitution of Gaussian
 ** elimination.
 **------------------------------------------------------
 */
void ForwardSub_pim(cl_context context, float *a, float *b, float *m, int size,int timing)
{


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
            //printf("Target GPU is %d\n",target_gpu[gpu_temp]);
            gpu_temp+=1;
        }
    }

/********************************************************************/
/********************************************************************/

    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));

    int *start_point_1=(int *)calloc(num_gpus,sizeof(int));
    int *end_point_1=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points_1=(int *)calloc(num_gpus,sizeof(int));

    int *start_point_2=(int *)calloc(num_gpus,sizeof(int));
    int *end_point_2=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points_2=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_m;
    void **pim_a;
    void **pim_b;

    float **pim_mapped_m;
    float **pim_mapped_a;
    float **pim_mapped_b;
    
/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_m=(void **)malloc(sizeof(void *)*num_gpus);
    pim_a=(void **)malloc(sizeof(void *)*num_gpus);
    pim_b=(void **)malloc(sizeof(void *)*num_gpus);
    
    pim_mapped_m=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_a=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_b=(float **)malloc(sizeof(float *)*num_gpus);
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    //kernel Fan1
    
    pim_domain_decomposition(start_point_1, end_point_1, own_num_points_1, num_gpus, size);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d records from %d to %d for Fan1\n",cur_gpu,own_num_points_1[cur_gpu],start_point_1[cur_gpu],end_point_1[cur_gpu]);
    }

    //kernel Fan2
    pim_domain_decomposition(start_point_2, end_point_2, own_num_points_2, num_gpus, size*size);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d records from %d to %d for Fan2\n",cur_gpu,own_num_points_2[cur_gpu],start_point_2[cur_gpu],end_point_2[cur_gpu]);
    }


/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_m[cur_gpu] = pim_malloc(sizeof(float)*size*size, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_a[cur_gpu] = pim_malloc(sizeof(float)*size*size, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_b[cur_gpu] = pim_malloc(sizeof(float)*size, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_memcpyHtoD(m,pim_m[cur_gpu],size*size);
        pim_memcpyHtoD(a,pim_a[cur_gpu],size*size);
        pim_memcpyHtoD(b,pim_b[cur_gpu],size);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/
int t;
for (t=0; t<(size-1); t++) {
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        // launch the PIM kernel
        pim_launch_Fan1_kernel(pim_m[cur_gpu], pim_a[cur_gpu], pim_b[cur_gpu], size, t, start_point_1[cur_gpu], own_num_points_1[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
    }

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        // wait for PIM gpus to finish
        cl_int clerr;
        clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
        ASSERT_CL_RETURN(clerr);
    }
    
    // synchronize array m
    pim_array_sync(m,pim_mapped_m,pim_m,size*size,num_gpus);

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        // launch the PIM kernel
        pim_launch_Fan2_kernel(pim_m[cur_gpu], pim_a[cur_gpu], pim_b[cur_gpu], size, t, start_point_2[cur_gpu], own_num_points_2[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
    }

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        // wait for PIM gpus to finish
        cl_int clerr;
        clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
        ASSERT_CL_RETURN(clerr);
    }

    // syncrhonize array a and b
    pim_array_sync(a,pim_mapped_a,pim_a,size*size,num_gpus);
    pim_array_sync(b,pim_mapped_b,pim_b,size,num_gpus);

    
}
    
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/******************** collect resutls from PIMs *********************/  

    
        //printf("The result of matrix m is: \n");
        //PrintMat(m, size, size, size);
        //printf("The result of matrix a is: \n");
        //PrintMat(a, size, size, size);
        //printf("The result of array b is: \n");
        //PrintAry(b, size);

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_m[cur_gpu]);
        pim_free(pim_a[cur_gpu]);
        pim_free(pim_b[cur_gpu]);
        
    }

    free(pim_m);
    free(pim_a);
    free(pim_b);
    
    free(pim_mapped_m);
    free(pim_mapped_a);
    free(pim_mapped_b);
    
    free(complete_event);
    free(start_point_1);
    free(end_point_1);
    free(own_num_points_1);
    free(start_point_2);
    free(end_point_2);
    free(own_num_points_2);

    free(target_gpu);    
    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);

/********************************************************************/
/********************************************************************/

    // **** PIM emulation End Mark  *********  
    pim_emu_end();
    
}






#define BLOCK_SIZE_1D 256
// launch the PIM kernel for FAN1
void pim_launch_Fan1_kernel(void *m, void *a, void *b, int size, int t, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"gaussianElim_kernels.cl";
    char * kernel_nm = (char *)"Fan1_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 1;
    int num_pre_event = 0;
    int number=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(number%localItemSize==0)?number:localItemSize*((number/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = m;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = a;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = b;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &size;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &t;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &start_point;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}

// launch the PIM kernel for FAN1
void pim_launch_Fan2_kernel(void *m, void *a, void *b, int size, int t, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"gaussianElim_kernels.cl";
    char * kernel_nm = (char *)"Fan2_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 1;
    int num_pre_event = 0;
    int number=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(number%localItemSize==0)?number:localItemSize*((number/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = m;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = a;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = b;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &size;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &t;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &start_point;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}
