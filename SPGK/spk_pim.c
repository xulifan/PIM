
void SPGK_PIM()
{
    int num_gpus;
    int num_cpus;
    int num_pims;
	int num_threads;
    pim_device_id target_gpu=0;

    pim_device_id* list_of_pims;
    //pim_device_id* list_of_cpus;
    //pim_device_id* list_of_gpus;
    int * gpus_per_pim;
    int * cpus_per_pim;

    void *pim_feat_g1;
    void *pim_feat_g2;
    void *pim_edge_w1;
    void *pim_edge_w2;
    void *pim_edge_x1;
    void *pim_edge_x2;
    void *pim_edge_y1;
    void *pim_edge_y2;

    void *pim_vertex;
    void *pim_edge;

    double *pim_mapped_feat_g1;
    double *pim_mapped_feat_g2;
    double *pim_mapped_edge_w1;
    double *pim_mapped_edge_w2;
    int *pim_mapped_edge_x1;
    int *pim_mapped_edge_x2;
    int *pim_mapped_edge_y1;
    int *pim_mapped_edge_y2;

    // PIM PER MODEL
    num_pims = find_pims();
    num_threads=num_pims;
    list_of_pims = (pim_device_id*)malloc(num_pims * sizeof(pim_device_id));
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

    
    // only use one GPU, get its id
    for (int i = 0; i < num_threads; i++) {
		if (gpus_per_pim[i] > 0 ){
            target_gpu=list_of_pims[i];
            printf("Target GPU is %d\n",target_gpu);
            break;
        }
    }
    
    // **** PIM emulation Start Mark  *********
	pim_emu_begin();

    for(int i=num_graph-1;i>=0;i--){    
	    
        n_node1=graph[i].n_node;
	    n_edge1=graph[i].n_sp_edge;

        pim_feat_g1 = pim_malloc(sizeof(double) * n_node1*n_feat, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_edge_w1 = pim_malloc(sizeof(double) * n_edge1, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_edge_x1 = pim_malloc(sizeof(int) * n_edge1, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_edge_y1 = pim_malloc(sizeof(int) * n_edge1, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

        pim_mapped_feat_g1 = (double *)pim_map(pim_feat_g1,PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_edge_w1 = (double *)pim_map(pim_edge_w1,PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_edge_x1 = (int *)pim_map(pim_edge_x1,PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_edge_y1 = (int *)pim_map(pim_edge_y1,PIM_PLATFORM_OPENCL_GPU);
    
        memcpy(pim_mapped_feat_g1,graph[i].feat[0],sizeof(double)*n_node1*n_feat);
        memcpy(pim_mapped_edge_w1,graph[i].sp_edge_w,sizeof(double)*n_edge1);
        memcpy(pim_mapped_edge_x1,graph[i].sp_edge_x,sizeof(int)*n_edge1);
        memcpy(pim_mapped_edge_y1,graph[i].sp_edge_y,sizeof(int)*n_edge1);

        pim_unmap(pim_mapped_feat_g1);
        pim_unmap(pim_mapped_edge_w1);
        pim_unmap(pim_mapped_edge_x1);
        pim_unmap(pim_mapped_edge_y1);

        for(int j=0;j<=i;j++){
            
            n_node2=graph[j].n_node;
            n_edge2=graph[j].n_sp_edge;
            //printf("%d %d %d %d\n",i,j,n_edge1,n_edge2);
            pim_feat_g2 = pim_malloc(sizeof(double) * n_node2*n_feat, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_w2 = pim_malloc(sizeof(double) * n_edge2, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_x2 = pim_malloc(sizeof(int) * n_edge2, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_y2 = pim_malloc(sizeof(int) * n_edge2, target_gpu, PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

            pim_mapped_feat_g2 = (double *)pim_map(pim_feat_g2,PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_w2 = (double *)pim_map(pim_edge_w2,PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_x2 = (int *)pim_map(pim_edge_x2,PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_y2 = (int *)pim_map(pim_edge_y2,PIM_PLATFORM_OPENCL_GPU);

            memcpy(pim_mapped_feat_g2,graph[j].feat[0],sizeof(double)*n_node2*n_feat);
            memcpy(pim_mapped_edge_w2,graph[j].sp_edge_w,sizeof(double)*n_edge2);
            memcpy(pim_mapped_edge_x2,graph[j].sp_edge_x,sizeof(int)*n_edge2);
            memcpy(pim_mapped_edge_y2,graph[j].sp_edge_y,sizeof(int)*n_edge2);

            pim_unmap(pim_mapped_feat_g2);
            pim_unmap(pim_mapped_edge_w2);
            pim_unmap(pim_mapped_edge_x2);
            pim_unmap(pim_mapped_edge_y2);

            double paramx = vk_params[0];
            double paramy = vk_params[1];

            pim_vertex = pim_malloc(sizeof(double) *n_node1*n_node2, target_gpu, PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);

            if(n_edge1>n_edge2) pim_edge = pim_malloc(sizeof(double) *n_edge1, target_gpu, PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
	        else pim_edge = pim_malloc(sizeof(double) *n_edge2, target_gpu, PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
	        
            if(n_edge1>=n_edge2){
                pim_launch_vert_gauss_kernel(pim_vertex,pim_feat_g1,pim_feat_g2,n_node1,n_node2,n_feat,paramy,target_gpu);
                pim_launch_edge_kernel(pim_edge, pim_vertex, pim_edge_w1, pim_edge_w2, pim_edge_x1, pim_edge_x2, pim_edge_y1, pim_edge_y2, n_edge1, n_edge2, n_node1, n_node2, paramx, target_gpu);
                
            }
            else{
                pim_launch_vert_gauss_kernel(pim_vertex,pim_feat_g2,pim_feat_g1,n_node2,n_node1,n_feat,paramy,target_gpu);
                pim_launch_edge_kernel(pim_edge, pim_vertex, pim_edge_w2, pim_edge_w1, pim_edge_x2, pim_edge_x1, pim_edge_y2, pim_edge_y1, n_edge2, n_edge1, n_node2, n_node1, paramx, target_gpu);
                
            }

            int num;
            if(n_edge1>n_edge2) num=n_edge1;
            else num=n_edge2;
            int outputsize=(num%BLOCK_SIZE_1D==0)?num/BLOCK_SIZE_1D:(( num/BLOCK_SIZE_1D )+ 1);
            void *pim_reduce_output = pim_malloc(sizeof(double)*outputsize, target_gpu, PIM_MEM_PIM_WRITE | PIM_MEM_HOST_READ,PIM_PLATFORM_OPENCL_GPU);
            pim_launch_reduce_kernel(pim_edge, pim_reduce_output, num, target_gpu);
            double sum = 0;
            double *pim_mapped_reduce_output=(double *)pim_map(pim_reduce_output,PIM_PLATFORM_OPENCL_GPU);
            for(int l=0;l<outputsize;l++) sum+=pim_mapped_reduce_output[l];

            pim_unmap(pim_mapped_reduce_output);
            
            pim_free(pim_vertex);
            pim_free(pim_edge);
            pim_free(pim_reduce_output);

            pim_free(pim_feat_g2);
            pim_free(pim_edge_w2);
            pim_free(pim_edge_x2);
            pim_free(pim_edge_y2);

            K_Matrix[i][j]=sum;
            K_Matrix[j][i]=sum;
            //printf("%d %d %d %d %lf\n",i,j,n_edge1,n_edge2,sum);
            
        }
        pim_free(pim_feat_g1);
        pim_free(pim_edge_w1);
        pim_free(pim_edge_x1);
        pim_free(pim_edge_y1);

    }

    // **** PIM emulation End Mark  *********  
    pim_emu_end();

    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);



}


void pim_launch_reduce_kernel(void *input, void *result, int num, pim_device_id target)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"reduce_nolocal";
    pim_f gpu_kernel;
    cl_event complete=0;

    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    size_t globalItemSize,localItemSize;

	localItemSize=BLOCK_SIZE_1D;
	
	globalItemSize=(num%localItemSize==0)?num:localItemSize*((num/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = input;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = result;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &num;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    //args[tnargs] = NULL;
    //sizes[tnargs] = localItemSize*sizeof(double);
    //tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = &complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

// wait for PIM gpus to finish

    cl_int clerr = clWaitForEvents(1, &complete) ;

    ASSERT_CL_RETURN(clerr);

}


void pim_launch_vert_gauss_kernel(void *vert, void *feat1, void *feat2, int n1, int n2, int nfeat, double param, pim_device_id target)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"vertex_gauss_kernel";
    pim_f gpu_kernel;
    cl_event complete=0;

    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 2;
    int num_pre_event = 0;
    size_t globalItemSize[2],localItemSize[2];

	localItemSize[0]=BLOCK_SIZE_2D;
	localItemSize[1]=BLOCK_SIZE_2D;
	
	globalItemSize[0]=(n1%localItemSize[0]==0)?n1:localItemSize[0]*((n1/localItemSize[0])+1);
	globalItemSize[1]=(n2%localItemSize[1]==0)?n2:localItemSize[1]*((n2/localItemSize[1])+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, globalItemSize, localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = vert;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = feat1;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = feat2;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &n_feat;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &n1;
    sizes[tnargs] = sizeof(int);
    tnargs++;   

    args[tnargs] = &n2;
    sizes[tnargs] = sizeof(int);
    tnargs++;         

    args[tnargs] = &param;
    sizes[tnargs] = sizeof(double);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = &complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

// wait for PIM gpus to finish

    cl_int clerr = clWaitForEvents(1, &complete) ;

    ASSERT_CL_RETURN(clerr);

}

void pim_launch_edge_kernel(void *edge, void *vert, void *w1, void *w2, void *x1, void *x2, void *y1, void *y2, int edge1, int edge2, int node1, int node2, double param, pim_device_id target)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"edge_kernel_1d";
    pim_f gpu_kernel;
    cl_event complete=0;

    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    size_t globalItemSize,localItemSize;

	localItemSize=BLOCK_SIZE_1D;
	
	globalItemSize=(edge1%localItemSize==0)?edge1:localItemSize*((edge1/localItemSize)+1);
	

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = edge;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = vert;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = w1;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = w2;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = x1;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = x2;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = y1;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = y2;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &edge1;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &edge2;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &node1;
    sizes[tnargs] = sizeof(int);
    tnargs++;   

    args[tnargs] = &node2;
    sizes[tnargs] = sizeof(int);
    tnargs++;         

    args[tnargs] = &param;
    sizes[tnargs] = sizeof(double);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = &complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

// wait for PIM gpus to finish

    cl_int clerr = clWaitForEvents(1, &complete) ;

    ASSERT_CL_RETURN(clerr);

}
