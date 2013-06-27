// Use multiple PIMs on one single pair of graphs
// Each PIM can have only one GPU for now
// each GPU calculate the full vertex kernel(redundant computation)
// each GPU taks some edged from one graph and compair with all edges in another graph
void SPGK_mult_PIM_one_pair_2()
{
    int num_gpus;
    int num_cpus;
    int num_pims;
    int num_threads;
    pim_device_id *target_gpu;

    pim_device_id* list_of_pims;
    //pim_device_id* list_of_cpus;
    //pim_device_id* list_of_gpus;
    int * gpus_per_pim;
    int * cpus_per_pim;

    void **pim_feat_g1;
    void **pim_feat_g2;
    void **pim_edge_w1;
    void **pim_edge_w2;
    void **pim_edge_x1;
    void **pim_edge_x2;
    void **pim_edge_y1;
    void **pim_edge_y2;

    double **pim_mapped_feat_g1;
    double **pim_mapped_feat_g2;
    double **pim_mapped_edge_w1;
    double **pim_mapped_edge_w2;
    int **pim_mapped_edge_x1;
    int **pim_mapped_edge_x2;
    int **pim_mapped_edge_y1;
    int **pim_mapped_edge_y2;

    void **pim_vertex;
    void **pim_edge;
    void **pim_reduce_output;
    double **pim_mapped_reduce_output;

    int *outputsize;

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
    
    

    pim_feat_g1=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge_w1=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge_x1=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge_y1=(void **)malloc(sizeof(void *)*num_gpus); 
    pim_feat_g2=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge_w2=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge_x2=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge_y2=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_feat_g1=(double **)malloc(sizeof(double *)*num_gpus);
    pim_mapped_feat_g2=(double **)malloc(sizeof(double *)*num_gpus);
    pim_mapped_edge_w1=(double **)malloc(sizeof(double *)*num_gpus);
    pim_mapped_edge_w2=(double **)malloc(sizeof(double *)*num_gpus);
    pim_mapped_edge_x1=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_edge_x2=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_edge_y1=(int **)malloc(sizeof(int *)*num_gpus); 
    pim_mapped_edge_y2=(int **)malloc(sizeof(int *)*num_gpus);

    pim_vertex=(void **)malloc(sizeof(void *)*num_gpus);
    pim_edge=(void **)malloc(sizeof(void *)*num_gpus);
    pim_reduce_output=(void **)malloc(sizeof(void *)*num_gpus);
    pim_mapped_reduce_output=(double **)malloc(sizeof(double *)*num_gpus);

    outputsize=(int *)malloc(sizeof(int)*num_gpus);  
    // **** PIM emulation Start Mark  *********


    pim_emu_begin();
    
    for(int g1_tmp=0;g1_tmp<num_graph;g1_tmp++){
    for(int g2_tmp=g1_tmp;g2_tmp<num_graph;g2_tmp++){

        int g1 = graph[g1_tmp].n_sp_edge>=graph[g2_tmp].n_sp_edge?g1_tmp:g2_tmp;
        int g2 = graph[g1_tmp].n_sp_edge>=graph[g2_tmp].n_sp_edge?g2_tmp:g1_tmp;
        //printf("%d %d %d %d %d\n",i,j,num_comparison,g1,g2);
        n_node1=graph[g1].n_node;
        n_edge1=graph[g1].n_sp_edge;

        n_node2=graph[g2].n_node;
        n_edge2=graph[g2].n_sp_edge;

        int edges_per_gpu=(n_edge1+num_gpus-1)/num_gpus;
        printf("Each GPU is calculating %d paris of edges for graph %d and graph %d\n",edges_per_gpu,g1,g2);
    
        cl_event *complete_vert=(cl_event *)calloc(num_gpus,sizeof(cl_event));
        cl_event *complete_edge=(cl_event *)calloc(num_gpus,sizeof(cl_event));
        cl_event *complete_reduce=(cl_event *)calloc(num_gpus,sizeof(cl_event));

        double sum=0;
        // issue pim_spawn to different GPUs
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

            int start_edge= cur_gpu*edges_per_gpu;
            int end_edge= start_edge+edges_per_gpu;
            if(end_edge>n_edge1) end_edge=n_edge1;
            int own_num_edges = end_edge-start_edge;

            pim_feat_g1[cur_gpu] = pim_malloc(sizeof(double) * n_node1*n_feat, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_w1[cur_gpu] = pim_malloc(sizeof(double) * n_edge1, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_x1[cur_gpu] = pim_malloc(sizeof(int) * n_edge1, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_y1[cur_gpu] = pim_malloc(sizeof(int) * n_edge1, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

            pim_mapped_feat_g1[cur_gpu] = (double *)pim_map(pim_feat_g1[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_w1[cur_gpu] = (double *)pim_map(pim_edge_w1[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_x1[cur_gpu] = (int *)pim_map(pim_edge_x1[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_y1[cur_gpu] = (int *)pim_map(pim_edge_y1[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        
            memcpy(pim_mapped_feat_g1[cur_gpu],graph[g1].feat[0],sizeof(double)*n_node1*n_feat);
            memcpy(pim_mapped_edge_w1[cur_gpu],graph[g1].sp_edge_w,sizeof(double)*n_edge1);
            memcpy(pim_mapped_edge_x1[cur_gpu],graph[g1].sp_edge_x,sizeof(int)*n_edge1);
            memcpy(pim_mapped_edge_y1[cur_gpu],graph[g1].sp_edge_y,sizeof(int)*n_edge1);

            pim_unmap(pim_mapped_feat_g1[cur_gpu]);
            pim_unmap(pim_mapped_edge_w1[cur_gpu]);
            pim_unmap(pim_mapped_edge_x1[cur_gpu]);
            pim_unmap(pim_mapped_edge_y1[cur_gpu]);


            //printf("%d %d %d %d\n",g1,g2,n_edge1,n_edge2);
            pim_feat_g2[cur_gpu] = pim_malloc(sizeof(double) * n_node2*n_feat, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_w2[cur_gpu] = pim_malloc(sizeof(double) * n_edge2, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_x2[cur_gpu] = pim_malloc(sizeof(int) * n_edge2, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_y2[cur_gpu] = pim_malloc(sizeof(int) * n_edge2, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

            pim_mapped_feat_g2[cur_gpu] = (double *)pim_map(pim_feat_g2[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_w2[cur_gpu] = (double *)pim_map(pim_edge_w2[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_x2[cur_gpu] = (int *)pim_map(pim_edge_x2[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_y2[cur_gpu] = (int *)pim_map(pim_edge_y2[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

            memcpy(pim_mapped_feat_g2[cur_gpu],graph[g2].feat[0],sizeof(double)*n_node2*n_feat);
            memcpy(pim_mapped_edge_w2[cur_gpu],graph[g2].sp_edge_w,sizeof(double)*n_edge2);
            memcpy(pim_mapped_edge_x2[cur_gpu],graph[g2].sp_edge_x,sizeof(int)*n_edge2);
            memcpy(pim_mapped_edge_y2[cur_gpu],graph[g2].sp_edge_y,sizeof(int)*n_edge2);

            pim_unmap(pim_mapped_feat_g2[cur_gpu]);
            pim_unmap(pim_mapped_edge_w2[cur_gpu]);
            pim_unmap(pim_mapped_edge_x2[cur_gpu]);
            pim_unmap(pim_mapped_edge_y2[cur_gpu]);

            double paramx = vk_params[0];
            double paramy = vk_params[1];
            
            pim_vertex[cur_gpu] = pim_malloc(sizeof(double) *n_node1*n_node2, target_gpu[cur_gpu], PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
            pim_edge[cur_gpu] = pim_malloc(sizeof(double) *own_num_edges, target_gpu[cur_gpu], PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
        
            pim_launch_vert_gauss_kernel(pim_vertex[cur_gpu],pim_feat_g1[cur_gpu],pim_feat_g2[cur_gpu],n_node1,n_node2,n_feat,paramy,target_gpu[cur_gpu], &complete_vert[cur_gpu]);
            pim_launch_edge_kernel_multipim_2(pim_edge[cur_gpu], pim_vertex[cur_gpu], pim_edge_w1[cur_gpu], pim_edge_w2[cur_gpu], pim_edge_x1[cur_gpu], pim_edge_x2[cur_gpu], pim_edge_y1[cur_gpu], pim_edge_y2[cur_gpu], n_edge1, n_edge2, n_node1, n_node2, paramx, start_edge, end_edge, own_num_edges, target_gpu[cur_gpu], &complete_edge[cur_gpu]);
            //pim_launch_edge_kernel_multipim_1(pim_edge[cur_gpu], pim_feat_g1[cur_gpu], pim_feat_g2[cur_gpu], pim_edge_w1[cur_gpu], pim_edge_w2[cur_gpu], pim_edge_x1[cur_gpu], pim_edge_x2[cur_gpu], pim_edge_y1[cur_gpu], pim_edge_y2[cur_gpu], n_edge1, n_edge2, n_node1, n_node2, n_feat, paramx, paramy, start_edge, end_edge, own_num_edges, target_gpu[cur_gpu], &complete_edge[cur_gpu]);
            int num=own_num_edges;
            
            outputsize[cur_gpu]=(num%BLOCK_SIZE_1D==0)?num/BLOCK_SIZE_1D:(( num/BLOCK_SIZE_1D )+ 1);
            pim_reduce_output[cur_gpu] = pim_malloc(sizeof(double)*outputsize[cur_gpu], target_gpu[cur_gpu], PIM_MEM_PIM_WRITE | PIM_MEM_HOST_READ,PIM_PLATFORM_OPENCL_GPU);

            pim_launch_reduce_kernel(pim_edge[cur_gpu], pim_reduce_output[cur_gpu], num, target_gpu[cur_gpu], &complete_reduce[cur_gpu]);
        }

        // collect results from different GPUs
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
 
            
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_vert[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
            clerr = clWaitForEvents(1, &complete_edge[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
            clerr = clWaitForEvents(1, &complete_reduce[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);

            
            pim_mapped_reduce_output[cur_gpu]=(double *)pim_map(pim_reduce_output[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            for(int l=0;l<outputsize[cur_gpu];l++) sum+=pim_mapped_reduce_output[cur_gpu][l];
            
            pim_unmap(pim_mapped_reduce_output[cur_gpu]);

            pim_free(pim_vertex[cur_gpu]);
            pim_free(pim_edge[cur_gpu]);
            pim_free(pim_reduce_output[cur_gpu]);


            
            //printf("graph %d and graph %d from GPU %d with results %lf\n",g1,g2,j,sum);
            pim_free(pim_feat_g2[cur_gpu]);
            pim_free(pim_edge_w2[cur_gpu]);
            pim_free(pim_edge_x2[cur_gpu]);
            pim_free(pim_edge_y2[cur_gpu]);

            pim_free(pim_feat_g1[cur_gpu]);
            pim_free(pim_edge_w1[cur_gpu]);
            pim_free(pim_edge_x1[cur_gpu]);
            pim_free(pim_edge_y1[cur_gpu]);

            
            
        }
        K_Matrix[g1][g2]=sum;
        K_Matrix[g2][g1]=sum;

        free(complete_vert);
        free(complete_edge);
        free(complete_reduce);

}
}

    // **** PIM emulation End Mark  *********  
    pim_emu_end();

    free(pim_feat_g1);
    free(pim_edge_w1);
    free(pim_edge_x1);
    free(pim_edge_y1);
    free(pim_feat_g2);
    free(pim_edge_w2);
    free(pim_edge_x2);
    free(pim_edge_y2);

    free(pim_mapped_feat_g1);
    free(pim_mapped_edge_w1);
    free(pim_mapped_edge_x1);
    free(pim_mapped_edge_y1);
    free(pim_mapped_feat_g2);
    free(pim_mapped_edge_w2);
    free(pim_mapped_edge_x2);
    free(pim_mapped_edge_y2);

    free(pim_vertex);
    free(pim_edge);
    free(pim_reduce_output);
    free(pim_mapped_reduce_output);

    free(outputsize);

    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);

}

void pim_launch_edge_kernel_multipim_2(void *edge, void *vert, void *w1, void *w2, void *x1, void *x2, void *y1, void *y2, int edge1, int edge2, int node1, int node2, double param, int start_edge, int end_edge, int own_num_edge, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"edge_kernel_multipim_2";
    pim_f gpu_kernel;

    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(own_num_edge%localItemSize==0)?own_num_edge:localItemSize*((own_num_edge/localItemSize)+1);
	

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

    args[tnargs] = &start_edge;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &end_edge;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &own_num_edge;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

    
}
