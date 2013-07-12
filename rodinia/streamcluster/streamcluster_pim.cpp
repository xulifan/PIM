

float pgain_pim( long x, Points *points, float z, long int *numcenters, int kmax, bool *is_center, int *center_table, char *switch_membership,
							double *serial, double *cpu_gpu_memcpy, double *memcpy_back, double *gpu_malloc, double *kernel)
{
	float gl_cost = 0;

	int K	= *numcenters ;						// number of centers
	int num    =   points->num;				// number of points
	int dim     =   points->dim;				// number of dimension
	kmax++;
	/***** build center index table 1*****/
	int count = 0;
	for( int i=0; i<num; i++){
		if( is_center[i] )
			center_table[i] = count++;
	}

	/***** initial memory allocation and preparation for transfer : execute once -1 *****/
	if( c == 0 ) {

	coord_h = (float*) malloc( num * dim * sizeof(float));								// coordinates (host)
	gl_lower = (float*) malloc( kmax * sizeof(float) );
	work_mem_h = (float*)_clMallocHost(kmax*num*sizeof(float));
	p_h = (Point_Struct*)malloc(num*sizeof(Point_Struct));	//by cambine: not compatibal with original Point
	
	// prepare mapping for point coordinates
	//--cambine: what's the use of point coordinates? for computing distance.
	for(int i=0; i<dim; i++){
		for(int j=0; j<num; j++)
			coord_h[ (num*i)+j ] = points->p[j].coord[i];
	}
    }
	for(int i=0; i<num; i++){
		p_h[i].weight = ((points->p)[i]).weight;
		p_h[i].assign = ((points->p)[i]).assign;
		p_h[i].cost = ((points->p)[i]).cost;	
	}


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
    //printf("Number of threads: %d\n", num_threads);
    //printf("Number of GPUs: %d\n",num_gpus);
    //printf("Number of CPUs: %d\n",num_cpus);
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

    int points_per_gpu=(num+num_gpus-1)/num_gpus;
    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_p;
    void **pim_coord;
    void **pim_work_mem;
    void **pim_center_table;
    void **pim_switch_membership;

    Point **pim_mapped_p;
    float **pim_mapped_coord;
    float **pim_mapped_work_mem;
    int **pim_mapped_center_table;
    char **pim_mapped_switch_membership;

/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_p=(void **)malloc(sizeof(void *)*num_gpus);
    pim_coord=(void **)malloc(sizeof(void *)*num_gpus);
    pim_work_mem=(void **)malloc(sizeof(void *)*num_gpus);
    pim_center_table=(void **)malloc(sizeof(void *)*num_gpus);
    pim_switch_membership=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_p=(Point **)malloc(sizeof(Point *)*num_gpus);
    pim_mapped_coord=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_work_mem=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_center_table=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_switch_membership=(char **)malloc(sizeof(char *)*num_gpus);

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        start_point[cur_gpu]= cur_gpu*points_per_gpu;
        end_point[cur_gpu]= start_point[cur_gpu]+points_per_gpu;
        if(end_point[cur_gpu]>num) end_point[cur_gpu]=num;
        own_num_points[cur_gpu] = end_point[cur_gpu]-start_point[cur_gpu];
        //printf("GPU %d is calculating %d records from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_p[cur_gpu] = pim_malloc(num * sizeof(Point), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_coord[cur_gpu] = pim_malloc(num * dim * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_work_mem[cur_gpu] = pim_malloc(kmax * num * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_center_table[cur_gpu] = pim_malloc(num * sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_switch_membership[cur_gpu] = pim_malloc(num * sizeof(char), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_mapped_p[cur_gpu] = (Point *)pim_map(pim_p[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_coord[cur_gpu] = (float *)pim_map(pim_coord[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_work_mem[cur_gpu] = (float *)pim_map(pim_work_mem[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_center_table[cur_gpu] = (int *)pim_map(pim_center_table[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_switch_membership[cur_gpu] = (char *)pim_map(pim_switch_membership[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        
        memcpy(pim_mapped_p[cur_gpu],p_h,num * sizeof(Point_Struct));
        memcpy(pim_mapped_coord[cur_gpu],coord_h, num*dim*sizeof(float));
        memset(pim_mapped_work_mem[cur_gpu],0,(K+1)*num*sizeof(float)); // K+1 == kmax??
        memcpy(pim_mapped_center_table[cur_gpu],center_table, num*sizeof(int));
        memset(pim_mapped_switch_membership[cur_gpu],0,num*sizeof(char));

        pim_unmap(pim_mapped_p[cur_gpu]);
        pim_unmap(pim_mapped_coord[cur_gpu]);
        pim_unmap(pim_mapped_work_mem[cur_gpu]);
        pim_unmap(pim_mapped_center_table[cur_gpu]);
        pim_unmap(pim_mapped_switch_membership[cur_gpu]);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        // launch the PIM kernel
        pim_launch_pgain_kernel(pim_p[cur_gpu], pim_coord[cur_gpu], pim_work_mem[cur_gpu], pim_center_table[cur_gpu], pim_switch_membership[cur_gpu], num, dim, x, K, start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
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
   
    pim_array_sync(switch_membership, pim_mapped_switch_membership, pim_switch_membership, num, num_gpus);
    pim_array_sync(work_mem_h,pim_mapped_work_mem,pim_work_mem, (K+1)*num, num_gpus);
    

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_p[cur_gpu]);
        pim_free(pim_coord[cur_gpu]);
        pim_free(pim_work_mem[cur_gpu]);
        pim_free(pim_center_table[cur_gpu]);
        pim_free(pim_switch_membership[cur_gpu]);
    }

    free(pim_p);
    free(pim_coord);
    free(pim_work_mem);
    free(pim_center_table);
    free(pim_switch_membership);

    free(pim_mapped_p);
    free(pim_mapped_coord);
    free(pim_mapped_work_mem);
    free(pim_mapped_center_table);
    free(pim_mapped_switch_membership);

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

	

	/****** cpu side work *****/
	int numclose = 0;
	gl_cost = z;
	
	/* compute the number of centers to close if we are to open i */
	for(int i=0; i < num; i++){	//--cambine:??
		if( is_center[i] ) {
			float low = z;
			//printf("i=%d  ", i);
		    for( int j = 0; j < num; j++ )
				low += work_mem_h[ j*(K+1) + center_table[i] ];
			//printf("low=%f\n", low);		
		    gl_lower[center_table[i]] = low;
				
		    if ( low > 0 ) {
				numclose++;				
				work_mem_h[i*(K+1)+K] -= low;
		    }
		}
		gl_cost += work_mem_h[i*(K+1)+K];
	}

	/* if opening a center at x saves cost (i.e. cost is negative) do so
		otherwise, do nothing */
	if ( gl_cost < 0 ) {
		for(int i=0; i<num; i++){
		
			bool close_center = gl_lower[center_table[points->p[i].assign]] > 0 ;
		    if ( (switch_membership[i]=='1') || close_center ) {
				points->p[i].cost = points->p[i].weight * dist(points->p[i], points->p[x], points->dim);
				points->p[i].assign = x;
		    }
	    }
		
		for(int i=0; i<num; i++){
			if( is_center[i] && gl_lower[center_table[i]] > 0 )
				is_center[i] = false;
		}
		
		is_center[x] = true;
		*numcenters = *numcenters +1 - numclose;
	}
	else
		gl_cost = 0;  // the value we'


	c++;


	/*FILE *fp = fopen("data_opencl.txt", "a");
	fprintf(fp,"%d, %f\n", c, gl_cost);
	fclose(fp);*/
	//printf("%d, %f\n", c, gl_cost);
	//exit(-1);
	return -gl_cost;
}






// merge the array souce[] with original[]
// and generating new array result[]
template <typename T> void array_merge(T *original, T *source, T *result, int n)
{
    for(int i=0;i<n;i++){
        T temp=source[i];
        if(original[i]!=temp) result[i]=temp;
    }
}

template <typename T> void pim_array_sync(T *host_array, T **pim_mapped_array, void **pim_array, int array_size, int num_gpus)
{
    T *host_array_sync=(T *)malloc(sizeof(T)*array_size);
    memcpy(host_array_sync,host_array,sizeof(T)*array_size);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){ 
        pim_mapped_array[cur_gpu] = (T *)pim_map(pim_array[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        array_merge(host_array,pim_mapped_array[cur_gpu],host_array_sync,array_size);
        
    } 
    
    memcpy(host_array,host_array_sync,sizeof(T)*array_size);
    free(host_array_sync);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){ 
        memcpy(pim_mapped_array[cur_gpu],host_array,sizeof(T) *array_size);
        pim_unmap(pim_mapped_array[cur_gpu]);
    }
    
}


#define BLOCK_SIZE_1D 256
// launch the PIM kernel for pgain
void pim_launch_pgain_kernel(void *p, void *coord, void *work_mem, void *center_table, void *switch_membership, int num, int dim, long x, int K, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"Kernels.cl";
    char * kernel_nm = (char *)"pgain_kernel_pim";
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

    args[tnargs] = p;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = coord;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = work_mem;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = center_table;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = switch_membership;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = dim*sizeof(float);
    tnargs++;

    args[tnargs] = &num;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &dim;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &x;
    sizes[tnargs] = sizeof(long);
    tnargs++;

    args[tnargs] = &K;
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
