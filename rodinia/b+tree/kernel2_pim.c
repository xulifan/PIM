void 
kernel2_pim_wrapper(        knode *knodes,
							long knodes_elem,
							long knodes_mem,

							int order,
							long maxheight,
							int count,

							long *currKnode,
							long *offset,
							long *lastKnode,
							long *offset_2,
							int *start,
							int *end,
							int *recstart,
							int *reclength)
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
            printf("Target GPU is %d\n",target_gpu[gpu_temp]);
            gpu_temp+=1;
        }
    }

/********************************************************************/
/********************************************************************/

    int points_per_gpu=(count+num_gpus-1)/num_gpus;
    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_knodes;
    void **pim_currKnode;
    void **pim_offset;
    void **pim_lastKnode;
    void **pim_offset_2;
    void **pim_start;
    void **pim_end;
    void **pim_recstart;
    void **pim_reclen;

    knode **pim_mapped_knodes;
    long **pim_mapped_currKnode;
    long **pim_mapped_offset;
    long **pim_mapped_lastKnode;
    long **pim_mapped_offset_2;
    int **pim_mapped_start;
    int **pim_mapped_end;
    int **pim_mapped_recstart;
    int **pim_mapped_reclen;

/********************************************************************/
/********************************************************************/

    // **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/
    
    pim_knodes=(void **)malloc(sizeof(void *)*num_gpus);
    pim_currKnode=(void **)malloc(sizeof(void *)*num_gpus);
    pim_offset=(void **)malloc(sizeof(void *)*num_gpus);
    pim_lastKnode=(void **)malloc(sizeof(void *)*num_gpus);
    pim_offset_2=(void **)malloc(sizeof(void *)*num_gpus);
    pim_start=(void **)malloc(sizeof(void *)*num_gpus);
    pim_end=(void **)malloc(sizeof(void *)*num_gpus);
    pim_recstart=(void **)malloc(sizeof(void *)*num_gpus);
    pim_reclen=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_knodes=(knode **)malloc(sizeof(knode *)*num_gpus);
    pim_mapped_currKnode=(long **)malloc(sizeof(long *)*num_gpus);
    pim_mapped_offset=(long **)malloc(sizeof(long *)*num_gpus);
    pim_mapped_lastKnode=(long **)malloc(sizeof(long *)*num_gpus);
    pim_mapped_offset_2=(long **)malloc(sizeof(long *)*num_gpus);
    pim_mapped_start=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_end=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_recstart=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_reclen=(int **)malloc(sizeof(int *)*num_gpus);

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        start_point[cur_gpu]= cur_gpu*points_per_gpu;
        end_point[cur_gpu]= start_point[cur_gpu]+points_per_gpu;
        if(end_point[cur_gpu]>count) end_point[cur_gpu]=count;
        own_num_points[cur_gpu] = end_point[cur_gpu]-start_point[cur_gpu];
        printf("GPU %d is calculating %d records from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* allocate memory for PIM */
        pim_knodes[cur_gpu] = pim_malloc(knodes_mem, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_currKnode[cur_gpu] = pim_malloc(count*sizeof(long), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_offset[cur_gpu] = pim_malloc(count*sizeof(long), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_lastKnode[cur_gpu] = pim_malloc(count*sizeof(long), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_offset_2[cur_gpu] = pim_malloc(count*sizeof(long), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_start[cur_gpu] = pim_malloc(count*sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_end[cur_gpu] = pim_malloc(count*sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_recstart[cur_gpu] = pim_malloc(count*sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_reclen[cur_gpu] = pim_malloc(count*sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/
        
/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_mapped_knodes[cur_gpu] = (knode *)pim_map(pim_knodes[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_currKnode[cur_gpu] = (long *)pim_map(pim_currKnode[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_offset[cur_gpu] = (long *)pim_map(pim_offset[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_lastKnode[cur_gpu] = (long *)pim_map(pim_lastKnode[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_offset_2[cur_gpu] = (long *)pim_map(pim_offset_2[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_start[cur_gpu] = (int *)pim_map(pim_start[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_end[cur_gpu] = (int *)pim_map(pim_end[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_recstart[cur_gpu] = (int *)pim_map(pim_recstart[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_reclen[cur_gpu] = (int *)pim_map(pim_reclen[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

        memcpy(pim_mapped_knodes[cur_gpu],knodes,knodes_mem);
        memcpy(pim_mapped_currKnode[cur_gpu],currKnode,count*sizeof(long));
        memcpy(pim_mapped_offset[cur_gpu],offset,count*sizeof(long));
        memcpy(pim_mapped_lastKnode[cur_gpu],lastKnode,count*sizeof(long));
        memcpy(pim_mapped_offset_2[cur_gpu],offset_2,count*sizeof(long));
        memcpy(pim_mapped_start[cur_gpu],start,count*sizeof(int));
        memcpy(pim_mapped_end[cur_gpu],end,count*sizeof(int));
        memcpy(pim_mapped_recstart[cur_gpu],recstart,count*sizeof(int));
        memcpy(pim_mapped_reclen[cur_gpu],reclength,count*sizeof(int));
        
        pim_unmap(pim_mapped_knodes[cur_gpu]);
        pim_unmap(pim_mapped_currKnode[cur_gpu]);
        pim_unmap(pim_mapped_offset[cur_gpu]);
        pim_unmap(pim_mapped_lastKnode[cur_gpu]);
        pim_unmap(pim_mapped_offset_2[cur_gpu]);
        pim_unmap(pim_mapped_start[cur_gpu]);
        pim_unmap(pim_mapped_end[cur_gpu]);
        pim_unmap(pim_mapped_recstart[cur_gpu]);
        pim_unmap(pim_mapped_reclen[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_launch_findRangeK_kernel(pim_knodes[cur_gpu], pim_currKnode[cur_gpu], pim_offset[cur_gpu], pim_lastKnode[cur_gpu], pim_offset_2[cur_gpu], pim_start[cur_gpu], pim_end[cur_gpu], pim_recstart[cur_gpu], pim_reclen[cur_gpu], maxheight, knodes_elem, start_point[cur_gpu], order, own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
    }

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        // wait for PIM gpus to finish
        cl_int clerr;
        clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
        ASSERT_CL_RETURN(clerr);
            
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/******************** collect resutls from PIMs *********************/  
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_mapped_recstart[cur_gpu] = (int *)pim_map(pim_recstart[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_reclen[cur_gpu] = (int *)pim_map(pim_reclen[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        memcpy(&recstart[start_point[cur_gpu]],&pim_mapped_recstart[cur_gpu][start_point[cur_gpu]],sizeof(int)*own_num_points[cur_gpu]);
        memcpy(&reclength[start_point[cur_gpu]],&pim_mapped_reclen[cur_gpu][start_point[cur_gpu]],sizeof(int)*own_num_points[cur_gpu]);
        pim_unmap(pim_mapped_recstart[cur_gpu]);
        pim_unmap(pim_mapped_reclen[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_free(pim_knodes[cur_gpu]);
        pim_free(pim_currKnode[cur_gpu]);
        pim_free(pim_offset[cur_gpu]);
        pim_free(pim_lastKnode[cur_gpu]);
        pim_free(pim_offset_2[cur_gpu]);
        pim_free(pim_start[cur_gpu]);
        pim_free(pim_end[cur_gpu]);
        pim_free(pim_recstart[cur_gpu]);
        pim_free(pim_reclen[cur_gpu]);
    }

    // free memory on host
    free(pim_knodes);
    free(pim_currKnode);
    free(pim_offset);
    free(pim_lastKnode);
    free(pim_offset_2);
    free(pim_start);
    free(pim_end);
    free(pim_recstart);
    free(pim_reclen);

    free(pim_mapped_knodes);
    free(pim_mapped_currKnode);
    free(pim_mapped_offset);
    free(pim_mapped_lastKnode);
    free(pim_mapped_offset_2);
    free(pim_mapped_start);
    free(pim_mapped_end);
    free(pim_mapped_recstart);
    free(pim_mapped_reclen);


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
	
}



// launch the PIM kernel for K range queries on the B+tree
void pim_launch_findRangeK_kernel(void *knodes, void *currKnode, void *offset, void *lastKnode, void * offset_2,  void *start, void *end, void *Recstart, void *Reclen, long height, long knodes_elem, int start_point, int order, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"./kernel/kernel_gpu_opencl_2.cl";
    char * kernel_nm = (char *)"findRangeK_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=order < 1024 ? order : 1024;
	
    globalItemSize=num*localItemSize;

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = knodes;
    sizes[tnargs] = sizeof(void *);
    tnargs++;
    
    args[tnargs] = currKnode;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = offset;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = lastKnode;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = offset_2;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = start;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = end;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = Recstart;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = Reclen;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &height;
    sizes[tnargs] = sizeof(long);
    tnargs++;

    args[tnargs] = &knodes_elem;
    sizes[tnargs] = sizeof(long);
    tnargs++;

    args[tnargs] = &start_point;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}
