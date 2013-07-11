
void run_bfs_pim(int no_of_nodes, Node *h_graph_nodes, int edge_list_size, \
		int *h_graph_edges, char *h_graph_mask, char *h_updating_graph_mask, \
		char *h_graph_visited, int *h_cost) 
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


    int points_per_gpu=(no_of_nodes+num_gpus-1)/num_gpus;
    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));
    
    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_graph_nodes;
    void **pim_graph_edges;
    void **pim_graph_mask;
    void **pim_updating_graph_mask;
    void **pim_graph_visited;
    void **pim_cost;
    void **pim_over;

    Node **pim_mapped_graph_nodes;
    int **pim_mapped_graph_edges;
    char **pim_mapped_graph_mask;
    char **pim_mapped_updating_graph_mask;
    char **pim_mapped_graph_visited;
    int **pim_mapped_cost;
    char **pim_mapped_over;


/********************************************************************/
/********************************************************************/


// **** PIM emulation Start Mark  *********
    pim_emu_begin();


/********************************************************************/
/******************** PIM meory allocate ****************************/
    pim_graph_nodes=(void **)malloc(sizeof(void *)*num_gpus);
    pim_graph_edges=(void **)malloc(sizeof(void *)*num_gpus);
    pim_graph_mask=(void **)malloc(sizeof(void *)*num_gpus);
    pim_updating_graph_mask=(void **)malloc(sizeof(void *)*num_gpus);
    pim_graph_visited=(void **)malloc(sizeof(void *)*num_gpus);
    pim_cost=(void **)malloc(sizeof(void *)*num_gpus);
    pim_over=(void **)malloc(sizeof(void *)*num_gpus);
    
    pim_mapped_graph_nodes=(Node **)malloc(sizeof(Node *)*num_gpus);
    pim_mapped_graph_edges=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_graph_mask=(char **)malloc(sizeof(char *)*num_gpus);
    pim_mapped_updating_graph_mask=(char **)malloc(sizeof(char *)*num_gpus);
    pim_mapped_graph_visited=(char **)malloc(sizeof(char *)*num_gpus);
    pim_mapped_cost=(int **)malloc(sizeof(int*)*num_gpus);
    pim_mapped_over=(char **)malloc(sizeof(char *)*num_gpus);

/********************************************************************/
/********************************************************************/




    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        start_point[cur_gpu]= cur_gpu*points_per_gpu;
        end_point[cur_gpu]= start_point[cur_gpu]+points_per_gpu;
        if(end_point[cur_gpu]>no_of_nodes) end_point[cur_gpu]=no_of_nodes;
        own_num_points[cur_gpu] = end_point[cur_gpu]-start_point[cur_gpu];
        printf("GPU %d is calculating %d nodes from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);

        /* allocate memory for PIM */
        pim_graph_nodes[cur_gpu] = pim_malloc(sizeof(Node) * no_of_nodes, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_graph_edges[cur_gpu] = pim_malloc(sizeof(int) * edge_list_size, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_graph_mask[cur_gpu] = pim_malloc(sizeof(char) * no_of_nodes, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_updating_graph_mask[cur_gpu] = pim_malloc(sizeof(char) * no_of_nodes, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_graph_visited[cur_gpu] = pim_malloc(sizeof(char) * no_of_nodes, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_cost[cur_gpu] = pim_malloc(sizeof(int) * no_of_nodes, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);

        pim_over[cur_gpu] = pim_malloc(sizeof(char), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        

        /* copy input information to PIM */
        pim_mapped_graph_nodes[cur_gpu] = (Node *)pim_map(pim_graph_nodes[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_graph_edges[cur_gpu] = (int *)pim_map(pim_graph_edges[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_graph_mask[cur_gpu] = (char *)pim_map(pim_graph_mask[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_updating_graph_mask[cur_gpu] = (char *)pim_map(pim_updating_graph_mask[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_graph_visited[cur_gpu] = (char *)pim_map(pim_graph_visited[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_cost[cur_gpu] = (int *)pim_map(pim_cost[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

        memcpy(pim_mapped_graph_nodes[cur_gpu],h_graph_nodes,sizeof(Node) *no_of_nodes);
        memcpy(pim_mapped_graph_edges[cur_gpu],h_graph_edges,sizeof(int) *edge_list_size);
        memcpy(pim_mapped_graph_mask[cur_gpu],h_graph_mask,sizeof(char) *no_of_nodes);
        memcpy(pim_mapped_updating_graph_mask[cur_gpu],h_updating_graph_mask,sizeof(char) *no_of_nodes);
        memcpy(pim_mapped_graph_visited[cur_gpu],h_graph_visited,sizeof(char) *no_of_nodes);
        memcpy(pim_mapped_cost[cur_gpu],h_cost,sizeof(int) *no_of_nodes); 

        pim_unmap(pim_mapped_graph_nodes[cur_gpu]);
        pim_unmap(pim_mapped_graph_edges[cur_gpu]);
        pim_unmap(pim_mapped_graph_mask[cur_gpu]);
        pim_unmap(pim_mapped_updating_graph_mask[cur_gpu]);
        pim_unmap(pim_mapped_graph_visited[cur_gpu]);
        pim_unmap(pim_mapped_cost[cur_gpu]);

        
    }

    int level=0;
    char h_over;
    do{
        level+=1;
        h_over = false;

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_mapped_over[cur_gpu]=(char *)pim_map(pim_over[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            memcpy(pim_mapped_over[cur_gpu],&h_over,sizeof(char));
            pim_unmap(pim_mapped_over[cur_gpu]);

            // launch the BFS1 kernel
            pim_launch_bfs1_kernel(pim_graph_nodes[cur_gpu],pim_graph_edges[cur_gpu],pim_graph_mask[cur_gpu],pim_updating_graph_mask[cur_gpu],pim_graph_visited[cur_gpu],pim_cost[cur_gpu],no_of_nodes,start_point[cur_gpu],end_point[cur_gpu],own_num_points[cur_gpu],target_gpu[cur_gpu], &complete_event[cur_gpu]);
 
        }


        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
        }
        
        /* synchronization */
        pim_array_sync(h_updating_graph_mask, pim_mapped_updating_graph_mask, pim_updating_graph_mask, no_of_nodes, num_gpus);
        pim_array_sync(h_graph_mask, pim_mapped_graph_mask, pim_graph_mask, no_of_nodes, num_gpus);
        

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  

            // launch the BFS2 kernel
            pim_launch_bfs2_kernel(pim_graph_mask[cur_gpu],pim_updating_graph_mask[cur_gpu],pim_graph_visited[cur_gpu],pim_over[cur_gpu],pim_cost[cur_gpu],no_of_nodes,start_point[cur_gpu],end_point[cur_gpu],own_num_points[cur_gpu],level,target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }


        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
        }

        /* synchronization */
        pim_array_sync(h_cost, pim_mapped_cost, pim_cost, no_of_nodes, num_gpus);
        pim_array_sync(h_graph_visited, pim_mapped_graph_visited, pim_graph_visited, no_of_nodes, num_gpus);
        pim_array_sync(h_graph_mask, pim_mapped_graph_mask, pim_graph_mask, no_of_nodes, num_gpus);
        pim_array_sync(h_updating_graph_mask, pim_mapped_updating_graph_mask, pim_updating_graph_mask, no_of_nodes, num_gpus);

        /* check if the traversal is over */
        char over_temp=false;
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){ 
            pim_mapped_over[cur_gpu]=(char *)pim_map(pim_over[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            memcpy(&over_temp,pim_mapped_over[cur_gpu],sizeof(char));
            pim_unmap(pim_mapped_over[cur_gpu]);
            if(over_temp==true){
                h_over=true;
                break;
            }
        }

        //for(int i=0;i<no_of_nodes;i++) printf("%d ",h_cost[i]);
        //printf("\n");

    }while(h_over);

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_free(pim_graph_nodes[cur_gpu]);
        pim_free(pim_graph_edges[cur_gpu]);
        pim_free(pim_graph_mask[cur_gpu]);
        pim_free(pim_updating_graph_mask[cur_gpu]);
        pim_free(pim_graph_visited[cur_gpu]);
        pim_free(pim_cost[cur_gpu]);

        pim_free(pim_over[cur_gpu]);
    }

    free(pim_graph_nodes);
    free(pim_graph_edges);
    free(pim_graph_mask);
    free(pim_updating_graph_mask);
    free(pim_graph_visited);
    free(pim_cost);
    free(pim_over);
    
    free(pim_mapped_graph_nodes);
    free(pim_mapped_graph_edges);
    free(pim_mapped_graph_mask);
    free(pim_mapped_updating_graph_mask);
    free(pim_mapped_graph_visited);
    free(pim_mapped_cost);
    free(pim_mapped_over);

    free(target_gpu);    
    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);

    // **** PIM emulation End Mark  *********  
    pim_emu_end();

	return ;
}


// merge the array souce[] with original[]
// and generating new array result[]
template <typename T> void array_merge(T *original, T *source, T *result, int n)
{
    for(int i=0;i<n;i++){
        T temp=source[i];
        if(original[i]!=temp) result[i]=temp;
    }
    return;
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
// launch the PIM kernel for BFS 1 kernel
// mark the nodes for next level to be updated
void pim_launch_bfs1_kernel(void *g_graph_nodes, void *g_graph_edges, void *g_graph_mask, void *g_updating_graph_mask, void *g_graph_visited, void *g_cost, int no_of_nodes, int start_node, int end_node, int own_num_nodes, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"Kernels.cl";
    char * kernel_nm = (char *)"BFS_1_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=own_num_nodes;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(num%localItemSize==0)?num:localItemSize*((num/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = g_graph_nodes;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_graph_edges;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_graph_mask;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_updating_graph_mask;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_graph_visited;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_cost;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &no_of_nodes;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &start_node;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &end_node;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &own_num_nodes;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}



// launch the PIM kernel for BFS 2 kernel
// update the nodes for the next level
void pim_launch_bfs2_kernel(void *g_graph_mask, void *g_updating_graph_mask, void *g_graph_visited, void *g_over, void *g_cost, int no_of_nodes, int start_node, int end_node, int own_num_nodes, int level, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"Kernels.cl";
    char * kernel_nm = (char *)"BFS_2_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=own_num_nodes;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(num%localItemSize==0)?num:localItemSize*((num/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments
    
    args[tnargs] = g_graph_mask;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_updating_graph_mask;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_graph_visited;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_over;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = g_cost;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &no_of_nodes;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &start_node;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &end_node;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &own_num_nodes;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &level;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}
