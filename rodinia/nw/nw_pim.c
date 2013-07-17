
#define BLOCK_SIZE_1D 16
// launch the PIM kernel for nw1
void pim_launch_nw1_kernel(void *reference, void *input_itermsets, void *output_itemsets, int cols, int penalty, int blk, int block_width, int worksize, int offset_r, int offset_c, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"nw.cl";
    char * kernel_nm = (char *)"nw_kernel1_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=num*localItemSize;

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = reference;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = input_itermsets;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = output_itemsets;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = sizeof(int)*(localItemSize+1)*(localItemSize+1);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = sizeof(int)*localItemSize*localItemSize;
    tnargs++;

    args[tnargs] = &cols;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &penalty;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &blk;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &block_width;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &worksize;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &offset_r;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &offset_c;
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


void pim_launch_nw2_kernel(void *reference, void *input_itermsets, void *output_itemsets, int cols, int penalty, int blk, int block_width, int worksize, int offset_r, int offset_c, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"nw.cl";
    char * kernel_nm = (char *)"nw_kernel2_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=num*localItemSize;

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = reference;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = input_itermsets;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = output_itemsets;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = sizeof(int)*(localItemSize+1)*(localItemSize+1);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = sizeof(int)*localItemSize*localItemSize;
    tnargs++;

    args[tnargs] = &cols;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &penalty;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &blk;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &block_width;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &worksize;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &offset_r;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &offset_c;
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
