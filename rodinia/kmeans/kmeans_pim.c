#define BLOCK_SIZE_1D 256
// PIM launch for the matrix transpose kernel
void pim_launch_swap_kernel(void *input, void *input_swap, int npoints, int nfeatures, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"kmeans.cl";
    char * kernel_nm = (char *)"kmeans_swap";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=npoints;
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

    args[tnargs] = input_swap;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &npoints;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &nfeatures;
    sizes[tnargs] = sizeof(int);
    tnargs++;


// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}

// PIM launch for the kmeans kernel
void pim_launch_kmeans_kernel(void *feature_swap, void *cluster, void *membership, int npoints, int nclusters, int nfeatures, int offset, int size, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"kmeans.cl";
    char * kernel_nm = (char *)"kmeans_kernel_c";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dim = 1;
    int num_pre_event = 0;
    int num=npoints;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(num%localItemSize==0)?num:localItemSize*((num/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dim, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = feature_swap;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = cluster;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = membership;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &npoints;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &nclusters;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &nfeatures;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &offset;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &size;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}


