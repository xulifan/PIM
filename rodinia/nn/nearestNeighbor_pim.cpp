float *FindNearestNeighbors_pim(
	cl_context context,
	int numRecords,
	std::vector<LatLong> &locations,float lat,float lng,
	int timing) {

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

    void **pim_locations;
    void **pim_distances;

    LatLong **pim_mapped_locations;
    float **pim_mapped_distances;

/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_locations=(void **)malloc(sizeof(void *)*num_gpus);
    pim_distances=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_locations=(LatLong **)malloc(sizeof(LatLong *)*num_gpus);
    pim_mapped_distances=(float **)malloc(sizeof(float *)*num_gpus);

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    pim_domain_decomposition(start_point, end_point, own_num_points, num_gpus, numRecords);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d records from %d to %d for Fan1\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_locations[cur_gpu] = pim_malloc(sizeof(LatLong) * own_num_points[cur_gpu], target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_distances[cur_gpu] = pim_malloc(sizeof(float) * own_num_points[cur_gpu], target_gpu[cur_gpu], PIM_MEM_PIM_WRITE | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        //pim_mapped_locations[cur_gpu] = (LatLong *)pim_map(pim_locations[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        //memcpy(pim_mapped_locations[cur_gpu],&locations[start_point[cur_gpu]],sizeof(LatLong) *own_num_points[cur_gpu]);
        //pim_unmap(pim_mapped_locations[cur_gpu]);
        pim_memcpyHtoD(&locations[start_point[cur_gpu]],pim_locations[cur_gpu],own_num_points[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        // launch the PIM kernel
        pim_launch_nn_kernel(pim_locations[cur_gpu], pim_distances[cur_gpu], own_num_points[cur_gpu], lat, lng, target_gpu[cur_gpu], &complete_event[cur_gpu]);
    }

    
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        // wait for PIM gpus to finish
        cl_int clerr;
        clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
        ASSERT_CL_RETURN(clerr);
            
    }

/********************************************************************/
/********************************************************************/

    // create distances std::vector
    float *distances = (float *)malloc(sizeof(float) * numRecords);

/********************************************************************/
/******************** collect resutls from PIMs *********************/  
   
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_mapped_distances[cur_gpu] = (float *)pim_map(pim_distances[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        memcpy(&distances[start_point[cur_gpu]],pim_mapped_distances[cur_gpu],sizeof(float)*own_num_points[cur_gpu]);
        pim_unmap(pim_mapped_distances[cur_gpu]);        
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_locations[cur_gpu]);
        pim_free(pim_distances[cur_gpu]);
    }

    free(pim_locations);
    free(pim_distances);

    free(pim_mapped_locations);
    free(pim_mapped_distances);

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
	
    return distances;
}





#define BLOCK_SIZE_1D 256
// launch the PIM kernel for nearest neighbors
// locations stores the coordinates for numRecords points
// The kernel calculates the distances for all points in locations from a certain point with coordinate (lat,lng)
// the distances are stored in the array distances[]
void pim_launch_nn_kernel(void *locations, void *distances, int numRecords, float lat, float lng, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"nearestNeighbor_kernel.cl";
    char * kernel_nm = (char *)"NearestNeighbor";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=numRecords;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(num%localItemSize==0)?num:localItemSize*((num/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = locations;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = distances;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &numRecords;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &lat;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &lng;
    sizes[tnargs] = sizeof(float);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}

