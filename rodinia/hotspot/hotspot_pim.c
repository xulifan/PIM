/*
   compute N time steps
*/

float * compute_tran_temp_pim(cl_mem MatrixPower, cl_mem MatrixTemp[2], int grid_cols, int grid_rows, \
		int total_iterations, int num_iterations, int blockCols, int blockRows, int borderCols, int borderRows,
		float *TempCPU, float *PowerCPU) 
{ 
	
	float grid_height = chip_height / grid_rows;
	float grid_width = chip_width / grid_cols;

	float Cap = FACTOR_CHIP * SPEC_HEAT_SI * t_chip * grid_width * grid_height;
	float Rx = grid_width / (2.0 * K_SI * t_chip * grid_height);
	float Ry = grid_height / (2.0 * K_SI * t_chip * grid_width);
	float Rz = t_chip / (K_SI * grid_height * grid_width);

	float max_slope = MAX_PD / (FACTOR_CHIP * t_chip * SPEC_HEAT_SI);
	float step = PRECISION / max_slope;
	int t;

	int src = 0, dst = 1;
	
	cl_int error;
	
	// Determine GPU work group grid
	size_t global_work_size[2];
	global_work_size[0] = BLOCK_SIZE * blockCols;
	global_work_size[1] = BLOCK_SIZE * blockRows;
	size_t local_work_size[2];
	local_work_size[0] = BLOCK_SIZE;
	local_work_size[1] = BLOCK_SIZE;
	

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

    int *start_point_x=(int *)calloc(num_gpus,sizeof(int));
    int *end_point_x=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points_x=(int *)calloc(num_gpus,sizeof(int));

    int *start_point_y=(int *)calloc(num_gpus,sizeof(int));
    int *end_point_y=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points_y=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_power;
    void **pim_temp_src;
    void **pim_temp_dst;

    float **pim_mapped_power;
    float **pim_mapped_temp_src;
    float **pim_mapped_temp_dst;
    
/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_power=(void **)malloc(sizeof(void *)*num_gpus);
    pim_temp_src=(void **)malloc(sizeof(void *)*num_gpus);
    pim_temp_dst=(void **)malloc(sizeof(void *)*num_gpus);
    
    pim_mapped_power=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_temp_src=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_temp_dst=(float **)malloc(sizeof(float *)*num_gpus);
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    printf("Cols :%d Rows %d\n",blockCols,blockRows);
    pim_domain_decomposition(start_point_x, end_point_x, own_num_points_x, num_gpus, blockCols);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d records from %d to %d for X\n",cur_gpu,own_num_points_x[cur_gpu],start_point_x[cur_gpu],end_point_x[cur_gpu]);
    }

    //pim_domain_decomposition(start_point_y, end_point_y, own_num_points_y, num_gpus, blockRows);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        own_num_points_y[cur_gpu]=blockRows;
        start_point_y[cur_gpu]=0;
        end_point_y[cur_gpu]=blockRows;
        printf("GPU %d is calculating %d records from %d to %d for Y\n",cur_gpu,own_num_points_y[cur_gpu],start_point_y[cur_gpu],end_point_y[cur_gpu]);
    }


/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_power[cur_gpu] = pim_malloc(sizeof(float)*grid_rows*grid_cols, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_temp_src[cur_gpu] = pim_malloc(sizeof(float)*grid_rows*grid_cols, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_temp_dst[cur_gpu] = pim_malloc(sizeof(float)*grid_rows*grid_cols, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    float *temp_dst=(float *)calloc(grid_rows*grid_cols,sizeof(float));
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_memcpyHtoD(PowerCPU,pim_power[cur_gpu],grid_rows*grid_cols);
        pim_memcpyHtoD(TempCPU,pim_temp_src[cur_gpu],grid_rows*grid_cols);
        pim_memcpyHtoD(temp_dst,pim_temp_dst[cur_gpu],grid_rows*grid_cols);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/

for (t = 0; t < total_iterations; t += num_iterations) {
        int iter = MIN(num_iterations, total_iterations - t);

		for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

            // launch the PIM kernel
            if(src ==0) pim_launch_hotspot_kernel(iter, pim_power[cur_gpu], pim_temp_src[cur_gpu], pim_temp_dst[cur_gpu], grid_cols, grid_rows, borderCols, borderRows, Cap, Rx, Ry, Rz, step, start_point_x[cur_gpu], start_point_y[cur_gpu], own_num_points_x[cur_gpu], own_num_points_y[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
            else pim_launch_hotspot_kernel(iter, pim_power[cur_gpu], pim_temp_dst[cur_gpu], pim_temp_src[cur_gpu], grid_cols, grid_rows, borderCols, borderRows, Cap, Rx, Ry, Rz, step, start_point_x[cur_gpu], start_point_y[cur_gpu], own_num_points_x[cur_gpu], own_num_points_y[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            //pim_print1d(pim_mapped_temp_dst[cur_gpu],pim_temp_dst[cur_gpu],grid_rows*grid_cols);
        }
        if(src ==0) pim_array_sync(temp_dst,pim_mapped_temp_dst,pim_temp_dst,grid_rows*grid_cols,num_gpus);
        else pim_array_sync(TempCPU,pim_mapped_temp_src,pim_temp_src,grid_rows*grid_cols,num_gpus);
        //print1d(temp_dst,grid_rows*grid_cols);
		// Swap input and output GPU matrices
		src = 1 - src;
		dst = 1 - dst;
        

}
   

    
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/******************** collect resutls from PIMs *********************/  
    
    pim_array_sync(temp_dst,pim_mapped_temp_dst,pim_temp_dst,grid_rows*grid_cols,num_gpus);
    pim_array_sync(TempCPU,pim_mapped_temp_src,pim_temp_src,grid_rows*grid_cols,num_gpus);

	if(src == 1) pim_memcpyDtoH(temp_dst,pim_temp_dst[0],grid_rows*grid_cols);
    else pim_memcpyDtoH(temp_dst,pim_temp_src[0],grid_rows*grid_cols);

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_power[cur_gpu]);
        pim_free(pim_temp_src[cur_gpu]);
        pim_free(pim_temp_dst[cur_gpu]);
        
    }

    free(pim_power);
    free(pim_temp_src);
    free(pim_temp_dst);
    
    free(pim_mapped_power);
    free(pim_mapped_temp_src);
    free(pim_mapped_temp_dst);
    
    free(complete_event);
    free(start_point_x);
    free(end_point_x);
    free(own_num_points_x);
    free(start_point_y);
    free(end_point_y);
    free(own_num_points_y);

    free(target_gpu);    
    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);

/********************************************************************/
/********************************************************************/

    // **** PIM emulation End Mark  *********  
    pim_emu_end();

	return temp_dst;
}


#define BLOCK_SIZE_2D 16
// launch the PIM kernel for FAN1
void pim_launch_hotspot_kernel(int iteration, void *power, void *temp_src, void *temp_dst, int grid_cols, int grid_rows, int border_cols, int border_rows, float Cap, float Rx, float Ry, float Rz, float step, int startx, int starty, int own_num_pointsx, int own_num_pointsy, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"hotspot_kernel.cl";
    char * kernel_nm = (char *)"hotspot_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 2;
    int num_pre_event = 0;
    size_t globalItemSize[2],localItemSize[2];

    localItemSize[0]=BLOCK_SIZE_2D;
    localItemSize[1]=BLOCK_SIZE_2D;
	
    globalItemSize[0]=own_num_pointsx*localItemSize[0];
    globalItemSize[1]=own_num_pointsy*localItemSize[1];

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, globalItemSize, localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = &iteration;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = power;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = temp_src;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = temp_dst;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &grid_cols;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &grid_rows;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &border_cols;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &border_rows;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &Cap;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &Rx;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &Ry;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &Rz;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &step;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &startx;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &starty;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}
