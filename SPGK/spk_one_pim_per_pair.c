// Use only one PIM with one GPUs
// The other PIMs are ignored
// One GPU calculates one pair of graphs
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
        int temp=gpus_per_pim[i];
        if(temp>1){
            printf("PIM%d has multiple (%d) GPUs, not supported for now!\n",i,temp);
            exit(-1);
        }
        else if (temp > 0 ){
            target_gpu=list_of_pims[i];
            printf("Target GPU is %d\n",target_gpu);
            break;
        }
    }
    

    // **** PIM emulation Start Mark  *********
    pim_emu_begin();

    //for(int i=num_graph-1;i>=0;i--){      
    for(int i=0;i<num_graph;i++){    
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

        //for(int j=0;j<=i;j++){
        for(int j=i;j<num_graph;j++){
    
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
            
            double sum=pim_launch_SPGK(i, j, pim_feat_g1, pim_edge_w1, pim_edge_x1, pim_edge_y1, pim_feat_g2, pim_edge_w2, pim_edge_x2, pim_edge_y2, target_gpu);

            K_Matrix[i][j]=sum;
            K_Matrix[j][i]=sum;
            
            pim_free(pim_feat_g2);
            pim_free(pim_edge_w2);
            pim_free(pim_edge_x2);
            pim_free(pim_edge_y2);

            
            
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


// Use multiple PIMs
// Each PIM can have only one GPU for now
// One GPU calculates one pair of graphs
void SPGK_mult_PIM()
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
    
    int pair_per_gpu=(num_comparison+num_gpus-1)/num_gpus;
    printf("Each GPU is calculating %d paris of graphs\n",pair_per_gpu);

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

    for(int i=0;i<pair_per_gpu;i++){
        cl_event *complete_vert=(cl_event *)calloc(num_gpus,sizeof(cl_event));
        cl_event *complete_edge=(cl_event *)calloc(num_gpus,sizeof(cl_event));
        cl_event *complete_reduce=(cl_event *)calloc(num_gpus,sizeof(cl_event));

        // issue pim_spawn to different GPUs
        for(int j=0;j<num_gpus;j++){
            
            int cur_pair=i*num_gpus+j;
            if(cur_pair>=num_comparison) break;

            int g1 = graph1_num[cur_pair];
            int g2 = graph2_num[cur_pair];
            //printf("%d %d %d %d %d\n",i,j,num_comparison,g1,g2);
            n_node1=graph[g1].n_node;
            n_edge1=graph[g1].n_sp_edge;

            pim_feat_g1[j] = pim_malloc(sizeof(double) * n_node1*n_feat, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_w1[j] = pim_malloc(sizeof(double) * n_edge1, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_x1[j] = pim_malloc(sizeof(int) * n_edge1, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_y1[j] = pim_malloc(sizeof(int) * n_edge1, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

            pim_mapped_feat_g1[j] = (double *)pim_map(pim_feat_g1[j],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_w1[j] = (double *)pim_map(pim_edge_w1[j],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_x1[j] = (int *)pim_map(pim_edge_x1[j],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_y1[j] = (int *)pim_map(pim_edge_y1[j],PIM_PLATFORM_OPENCL_GPU);
        
            memcpy(pim_mapped_feat_g1[j],graph[g1].feat[0],sizeof(double)*n_node1*n_feat);
            memcpy(pim_mapped_edge_w1[j],graph[g1].sp_edge_w,sizeof(double)*n_edge1);
            memcpy(pim_mapped_edge_x1[j],graph[g1].sp_edge_x,sizeof(int)*n_edge1);
            memcpy(pim_mapped_edge_y1[j],graph[g1].sp_edge_y,sizeof(int)*n_edge1);

            pim_unmap(pim_mapped_feat_g1[j]);
            pim_unmap(pim_mapped_edge_w1[j]);
            pim_unmap(pim_mapped_edge_x1[j]);
            pim_unmap(pim_mapped_edge_y1[j]);

    
            n_node2=graph[g2].n_node;
            n_edge2=graph[g2].n_sp_edge;
            //printf("%d %d %d %d\n",g1,g2,n_edge1,n_edge2);
            pim_feat_g2[j] = pim_malloc(sizeof(double) * n_node2*n_feat, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_w2[j] = pim_malloc(sizeof(double) * n_edge2, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_x2[j] = pim_malloc(sizeof(int) * n_edge2, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_edge_y2[j] = pim_malloc(sizeof(int) * n_edge2, target_gpu[j], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

            pim_mapped_feat_g2[j] = (double *)pim_map(pim_feat_g2[j],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_w2[j] = (double *)pim_map(pim_edge_w2[j],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_x2[j] = (int *)pim_map(pim_edge_x2[j],PIM_PLATFORM_OPENCL_GPU);
            pim_mapped_edge_y2[j] = (int *)pim_map(pim_edge_y2[j],PIM_PLATFORM_OPENCL_GPU);

            memcpy(pim_mapped_feat_g2[j],graph[g2].feat[0],sizeof(double)*n_node2*n_feat);
            memcpy(pim_mapped_edge_w2[j],graph[g2].sp_edge_w,sizeof(double)*n_edge2);
            memcpy(pim_mapped_edge_x2[j],graph[g2].sp_edge_x,sizeof(int)*n_edge2);
            memcpy(pim_mapped_edge_y2[j],graph[g2].sp_edge_y,sizeof(int)*n_edge2);

            pim_unmap(pim_mapped_feat_g2[j]);
            pim_unmap(pim_mapped_edge_w2[j]);
            pim_unmap(pim_mapped_edge_x2[j]);
            pim_unmap(pim_mapped_edge_y2[j]);
      
            double paramx = vk_params[0];
            double paramy = vk_params[1];
      
            pim_vertex[j] = pim_malloc(sizeof(double) *n_node1*n_node2, target_gpu[j], PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);

            if(n_edge1>n_edge2) pim_edge[j] = pim_malloc(sizeof(double) *n_edge1, target_gpu[j], PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
	        else pim_edge[j] = pim_malloc(sizeof(double) *n_edge2, target_gpu[j], PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);

            // launch vertex kernel and edge kernel	                
            if(n_edge1>=n_edge2){
                pim_launch_vert_gauss_kernel(pim_vertex[j],pim_feat_g1[j],pim_feat_g2[j],n_node1,n_node2,n_feat,paramy,target_gpu[j], &complete_vert[j]);
                pim_launch_edge_kernel(pim_edge[j], pim_vertex[j], pim_edge_w1[j], pim_edge_w2[j], pim_edge_x1[j], pim_edge_x2[j], pim_edge_y1[j], pim_edge_y2[j], n_edge1, n_edge2, n_node1, n_node2, paramx, target_gpu[j], &complete_edge[j]);
                        
            }
            else{
                pim_launch_vert_gauss_kernel(pim_vertex[j],pim_feat_g2[j],pim_feat_g1[j],n_node2,n_node1,n_feat,paramy,target_gpu[j], &complete_vert[j]);
                pim_launch_edge_kernel(pim_edge[j], pim_vertex[j], pim_edge_w2[j], pim_edge_w1[j], pim_edge_x2[j], pim_edge_x1[j], pim_edge_y2[j], pim_edge_y1[j], n_edge2, n_edge1, n_node2, n_node1, paramx, target_gpu[j], &complete_edge[j]);
                        
            }

            int num;
            if(n_edge1>n_edge2) num=n_edge1;
            else num=n_edge2;
            outputsize[j]=(num%BLOCK_SIZE_1D==0)?num/BLOCK_SIZE_1D:(( num/BLOCK_SIZE_1D )+ 1);
            pim_reduce_output[j] = pim_malloc(sizeof(double)*outputsize[j], target_gpu[j], PIM_MEM_PIM_WRITE | PIM_MEM_HOST_READ,PIM_PLATFORM_OPENCL_GPU);

            // launch reduction kernel
            pim_launch_reduce_kernel(pim_edge[j], pim_reduce_output[j], num, target_gpu[j], &complete_reduce[j]);
        }

        // collect results from different GPUs
        for(int j=0;j<num_gpus;j++){  
 
            int cur_pair=i*num_gpus+j;
            if(cur_pair>=num_comparison) break;

            int g1 = graph1_num[cur_pair];
            int g2 = graph2_num[cur_pair];         
            
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_vert[j]) ;
            ASSERT_CL_RETURN(clerr);
            clerr = clWaitForEvents(1, &complete_edge[j]) ;
            ASSERT_CL_RETURN(clerr);
            clerr = clWaitForEvents(1, &complete_reduce[j]) ;
            ASSERT_CL_RETURN(clerr);

            double sum = 0;
            pim_mapped_reduce_output[j]=(double *)pim_map(pim_reduce_output[j],PIM_PLATFORM_OPENCL_GPU);
            for(int l=0;l<outputsize[j];l++) sum+=pim_mapped_reduce_output[j][l];
            
            pim_unmap(pim_mapped_reduce_output[j]);

            pim_free(pim_vertex[j]);
            pim_free(pim_edge[j]);
            pim_free(pim_reduce_output[j]);


            K_Matrix[g1][g2]=sum;
            K_Matrix[g2][g1]=sum;
            //printf("graph %d and graph %d from GPU %d with results %lf\n",g1,g2,j,sum);
            pim_free(pim_feat_g2[j]);
            pim_free(pim_edge_w2[j]);
            pim_free(pim_edge_x2[j]);
            pim_free(pim_edge_y2[j]);

            pim_free(pim_feat_g1[j]);
            pim_free(pim_edge_w1[j]);
            pim_free(pim_edge_x1[j]);
            pim_free(pim_edge_y1[j]);
      
        }
        
        free(complete_vert);
        free(complete_edge);
        free(complete_reduce);

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

// apply SPGK on a pair of graphs on one single PIM
double pim_launch_SPGK(int g1, int g2, void *pim_feat_g1, void *pim_edge_w1, void *pim_edge_x1, void *pim_edge_y1,
    void *pim_feat_g2, void *pim_edge_w2, void *pim_edge_x2, void *pim_edge_y2, pim_device_id target_gpu)
{
    
    int n_node1=graph[g1].n_node;
    int n_edge1=graph[g1].n_sp_edge;

    int n_node2=graph[g2].n_node;
    int n_edge2=graph[g2].n_sp_edge;

    double paramx = vk_params[0];
    double paramy = vk_params[1];

    void *pim_vertex;
    void *pim_edge;

    cl_event complete_vert=0;
    cl_event complete_edge=0;
    cl_event complete_reduce=0;
    cl_int clerr;
    
    pim_vertex = pim_malloc(sizeof(double) *n_node1*n_node2, target_gpu, PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);

    if(n_edge1>n_edge2) pim_edge = pim_malloc(sizeof(double) *n_edge1, target_gpu, PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
	else pim_edge = pim_malloc(sizeof(double) *n_edge2, target_gpu, PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
	
    // launch vertex kernel and edge kernel        
    if(n_edge1>=n_edge2){
        pim_launch_vert_gauss_kernel(pim_vertex,pim_feat_g1,pim_feat_g2,n_node1,n_node2,n_feat,paramy,target_gpu, &complete_vert);
        // wait for PIM gpus to finish
        clerr = clWaitForEvents(1, &complete_vert) ;
        ASSERT_CL_RETURN(clerr);
        pim_launch_edge_kernel(pim_edge, pim_vertex, pim_edge_w1, pim_edge_w2, pim_edge_x1, pim_edge_x2, pim_edge_y1, pim_edge_y2, n_edge1, n_edge2, n_node1, n_node2, paramx, target_gpu, &complete_edge);
        // wait for PIM gpus to finish
        clerr = clWaitForEvents(1, &complete_edge) ;
        ASSERT_CL_RETURN(clerr);
                
    }
    else{
        pim_launch_vert_gauss_kernel(pim_vertex,pim_feat_g2,pim_feat_g1,n_node2,n_node1,n_feat,paramy,target_gpu, &complete_vert);
        // wait for PIM gpus to finish
        clerr = clWaitForEvents(1, &complete_vert) ;
        ASSERT_CL_RETURN(clerr);
        pim_launch_edge_kernel(pim_edge, pim_vertex, pim_edge_w2, pim_edge_w1, pim_edge_x2, pim_edge_x1, pim_edge_y2, pim_edge_y1, n_edge2, n_edge1, n_node2, n_node1, paramx, target_gpu, &complete_edge);
        // wait for PIM gpus to finish
        clerr = clWaitForEvents(1, &complete_edge) ;
        ASSERT_CL_RETURN(clerr);
                
    }

    int num;
    if(n_edge1>n_edge2) num=n_edge1;
    else num=n_edge2;
    int outputsize=(num%BLOCK_SIZE_1D==0)?num/BLOCK_SIZE_1D:(( num/BLOCK_SIZE_1D )+ 1);
    void *pim_reduce_output = pim_malloc(sizeof(double)*outputsize, target_gpu, PIM_MEM_PIM_WRITE | PIM_MEM_HOST_READ,PIM_PLATFORM_OPENCL_GPU);

    // launch reduction kernel
    pim_launch_reduce_kernel(pim_edge, pim_reduce_output, num, target_gpu, &complete_reduce);
    // wait for PIM gpus to finish
    clerr = clWaitForEvents(1, &complete_reduce) ;
    ASSERT_CL_RETURN(clerr);

    double sum = 0;
    double *pim_mapped_reduce_output=(double *)pim_map(pim_reduce_output,PIM_PLATFORM_OPENCL_GPU);
    for(int l=0;l<outputsize;l++) sum+=pim_mapped_reduce_output[l];
    
    pim_unmap(pim_mapped_reduce_output);

    pim_free(pim_vertex);
    pim_free(pim_edge);
    pim_free(pim_reduce_output);
    //printf("%d %d %f\n",g1,g2,sum);
    return sum;
}

// reduction kernel, sum input[] into a small array result[]
void pim_launch_reduce_kernel(void *input, void *result, int num, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"reduce";
    pim_f gpu_kernel;
    
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

    args[tnargs] = NULL;
    sizes[tnargs] = localItemSize*sizeof(double);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}

// vertex kernel
// apply gaussian kernel on the vertices
void pim_launch_vert_gauss_kernel(void *vert, void *feat1, void *feat2, int n1, int n2, int nfeat, double param, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"vertex_gauss_kernel";
    pim_f gpu_kernel;
    
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

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);


}

// edge kernel
// each GPU thread is in charge of one edge from g1
// each GPU thread loops through all edges in g2
// vert[][] stored the similarities for vertices from the vertex kernel
void pim_launch_edge_kernel(void *edge, void *vert, void *w1, void *w2, void *x1, void *x2, void *y1, void *y2, int edge1, int edge2, int node1, int node2, double param, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"graphkernels.cl";
    char * kernel_nm = (char *)"edge_kernel_1d";
    pim_f gpu_kernel;

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

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

    
}
