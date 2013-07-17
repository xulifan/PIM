void 
kernel_pim_wrapper(	par_str par_cpu,
							dim_str dim_cpu,
							box_str* box_cpu,
							FOUR_VECTOR* rv_cpu,
							fp* qv_cpu,
							FOUR_VECTOR* fv_cpu)
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

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_box;
    void **pim_rv;
    void **pim_qv;
    void **pim_fv;

    box_str **pim_mapped_box;
    FOUR_VECTOR **pim_mapped_rv;
    fp **pim_mapped_qv;
    FOUR_VECTOR **pim_mapped_fv;
    
/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_box=(void **)malloc(sizeof(void *)*num_gpus);
    pim_rv=(void **)malloc(sizeof(void *)*num_gpus);
    pim_qv=(void **)malloc(sizeof(void *)*num_gpus);
    pim_fv=(void **)malloc(sizeof(void *)*num_gpus);
    
    pim_mapped_box=(box_str **)malloc(sizeof(box_str *)*num_gpus);
    pim_mapped_rv=(FOUR_VECTOR **)malloc(sizeof(FOUR_VECTOR *)*num_gpus);
    pim_mapped_qv=(fp **)malloc(sizeof(fp *)*num_gpus);
    pim_mapped_fv=(FOUR_VECTOR **)malloc(sizeof(FOUR_VECTOR *)*num_gpus);
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    //kernel Fan1
    
    pim_domain_decomposition(start_point, end_point, own_num_points, num_gpus, dim_cpu.number_boxes);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d records from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);
    }


/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_box[cur_gpu] = pim_malloc(dim_cpu.box_mem, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_rv[cur_gpu] = pim_malloc(dim_cpu.space_mem, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_qv[cur_gpu] = pim_malloc(dim_cpu.space_mem2, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_fv[cur_gpu] = pim_malloc(dim_cpu.space_mem, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_memcpyHtoD(box_cpu,pim_box[cur_gpu],dim_cpu.box_mem/sizeof(box_str));
        pim_memcpyHtoD(rv_cpu,pim_rv[cur_gpu],dim_cpu.space_mem/sizeof(FOUR_VECTOR));
        pim_memcpyHtoD(qv_cpu,pim_qv[cur_gpu],dim_cpu.space_mem2/sizeof(fp));
        pim_memcpyHtoD(fv_cpu,pim_fv[cur_gpu],dim_cpu.space_mem/sizeof(FOUR_VECTOR));

    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        // launch the PIM kernel
        pim_launch_lavaMD_kernel(par_cpu, dim_cpu, pim_box[cur_gpu], pim_rv[cur_gpu], pim_qv[cur_gpu], pim_fv[cur_gpu], start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
    }

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        // wait for PIM gpus to finish
        cl_int clerr;
        clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
        ASSERT_CL_RETURN(clerr);
    }
    
    // synchronize array fv
    //pim_array_sync(fv_cpu,pim_mapped_fv,pim_fv,dim_cpu.space_mem/sizeof(FOUR_VECTOR),num_gpus);
    FOUR_VECTOR *array_sync=(FOUR_VECTOR *)malloc(dim_cpu.space_mem);
    memcpy(array_sync,fv_cpu,dim_cpu.space_mem);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_mapped_fv[cur_gpu]=(FOUR_VECTOR *)pim_map(pim_fv[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        for(int i=0;i<dim_cpu.space_mem/sizeof(FOUR_VECTOR);i++){
            FOUR_VECTOR temp1=pim_mapped_fv[cur_gpu][i];
            FOUR_VECTOR temp2=fv_cpu[i];
            if(temp1.x!=temp2.x || temp1.y!=temp2.y || temp1.z!=temp2.z || temp1.v!=temp2.v) {
                //printf("%d %d %d\n",i,temp,original[i]);
                array_sync[i].x=temp1.x;
                array_sync[i].y=temp1.y;
                array_sync[i].z=temp1.z;
                array_sync[i].v=temp1.v;
            }
        }


    }
    memcpy(fv_cpu,array_sync,dim_cpu.space_mem);
    
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/******************** collect resutls from PIMs *********************/  

    
	FILE *result_fp=fopen("result.txt","w");
	 
	 for(int g=0; g<dim_cpu.space_mem/sizeof(FOUR_VECTOR); g++){
		 fprintf(result_fp,"%f, %f, %f, %f\n", fv_cpu[g].v, fv_cpu[g].x, fv_cpu[g].y, fv_cpu[g].z);
	 }
    fclose(result_fp);

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_box[cur_gpu]);
        pim_free(pim_rv[cur_gpu]);
        pim_free(pim_qv[cur_gpu]);
        pim_free(pim_fv[cur_gpu]);
        
    }

    free(pim_box);
    free(pim_rv);
    free(pim_qv);
    free(pim_fv);
    
    free(pim_mapped_box);
    free(pim_mapped_rv);
    free(pim_mapped_qv);
    free(pim_mapped_fv);
    
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

#define BLOCK_SIZE_1D 128
// launch the PIM kernel for FAN1
void pim_launch_lavaMD_kernel(par_str d_par, dim_str d_dim, void *d_box, void *d_rv, void *d_qv, void *d_fv, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"./kernel/kernel_gpu_opencl.cl";
    char * kernel_nm = (char *)"kernel_gpu_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 1;
    int num_pre_event = 0;
    int number=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=number*localItemSize;

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = &d_par;
    sizes[tnargs] = sizeof(par_str);
    tnargs++;

    args[tnargs] = &d_dim;
    sizes[tnargs] = sizeof(dim_str);
    tnargs++;

    args[tnargs] = d_box;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = d_rv;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = d_qv;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = d_fv;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &start_point;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}
