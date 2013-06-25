void OpenCL_init()
{
    read_cl_file();
    
    cl_initialization();
        
    cl_load_prog();
}

void read_cl_file()
{
    FILE *fp;
	// Load the kernel source code into the array source_str
    fp = fopen("graphkernels.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(EXIT_FAILURE);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );
}  

void cl_load_prog()
{
    //printf("loading OpenCL kernels\n");
	// Create a program from the kernel source
	clProgram = clCreateProgramWithSource(clGPUContext, 1, (const char **)&source_str, (const size_t *)&source_size, &errcode);

	ASSERT_CL_RETURN(errcode);

	// Build the program
	errcode = clBuildProgram(clProgram, 1, &device_id, NULL, NULL, NULL);
	ASSERT_CL_RETURN(errcode);
		
	// Create the OpenCL kernel
	vertex_gauss_kernel = clCreateKernel(clProgram, "vertex_gauss_kernel", &errcode);
	edge_kernel_1d = clCreateKernel(clProgram, "edge_kernel_1d", &errcode);

	reduce_kernel = clCreateKernel(clProgram, "reduce", &errcode);
    
    ASSERT_CL_RETURN(errcode);
	clFinish(clCommandQue);
}
 
void cl_initialization()
{
	
	// Get platform and device information
    errcode = clGetPlatformIDs(0, NULL, &num_platforms);
    ASSERT_CL_RETURN(errcode);
    char platform_str[1024];
    if(platform_opt==0) strcpy(platform_str,"NVIDIA Corporation");
    else if(platform_opt==1) strcpy(platform_str,"Advanced Micro Devices, Inc.");
    else if(platform_opt==2) strcpy(platform_str,"Intel(R) Corporation");
    else strcpy(platform_str,"NVIDIA Corporation");
    cout<<"Looking for platform: "<<platform_str<<endl;
    if (0 < num_platforms) 
    {
        platform_id = (cl_platform_id *)malloc(sizeof(cl_platform_id)*num_platforms);
        errcode = clGetPlatformIDs(num_platforms, platform_id, NULL);
        ASSERT_CL_RETURN(errcode);

        for (unsigned int i = 0; i < num_platforms; ++i) 
        {
            errcode = clGetPlatformInfo(platform_id[i],CL_PLATFORM_VENDOR,sizeof(str_temp), str_temp, NULL);

           ASSERT_CL_RETURN(errcode);
            //if(errcode == CL_SUCCESS) printf("Platform %d Vendor: %s\n",i,str_temp);
			
            if (strcmp(str_temp, platform_str)!=0) 
            {       
                printf("Platform %s is not the desired, check next platform\n",str_temp);
            }
            else{
                errcode = clGetPlatformInfo(platform_id[i],CL_PLATFORM_NAME, sizeof(str_temp), str_temp,NULL);
                if(errcode == CL_SUCCESS) printf("Platform %d Name: %s\n",i,str_temp);
                if(platform_opt==2) errcode = clGetDeviceIDs( platform_id[i], CL_DEVICE_TYPE_CPU, 1, &device_id, &num_devices);
                else errcode = clGetDeviceIDs( platform_id[i], CL_DEVICE_TYPE_GPU, 1, &device_id, &num_devices);
                break;
            }
            if(i == num_platforms - 1){
                cout<<"Cannot find the desired platform, quit!"<<endl;
                exit(EXIT_FAILURE);
            }
        }


        free(platform_id);
    }

    errcode = clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(str_temp), str_temp,NULL);
    errcode|= clGetDeviceInfo(device_id, CL_DRIVER_VERSION, sizeof(driver_version), driver_version,NULL);
    errcode|= clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(device_version), device_version,NULL);
    errcode|= clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, sizeof(device_extension), device_extension,NULL);
    errcode|= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
    errcode|= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
	errcode|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
	errcode|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(global_mem_cacheline_size), &global_mem_cacheline_size, NULL);
    errcode|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(global_mem_cache_size), &global_mem_cache_size, NULL);
	errcode|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
    //errcode|= clGetDeviceInfo(device_id, CL_DEVICE_PRINTF_BUFFER_SIZE, sizeof(printf_buffer_size), &printf_buffer_size, NULL);
	printf("Device Name: %s\n",str_temp);

    if(errcode == CL_SUCCESS && DEVICE_QUERY == 1){
    
    printf("Device Version: %s\n",device_version);
    printf("Device Extension: %s\n",device_extension);
    printf("OpenCL Driver Version: %s\n",driver_version);
    printf("Local Mem Type (Local=1, Global=2): %d\n",(int)local_mem_type);
    printf("Local Mem Size(KB): %ld\n",local_mem_size/1024);
	printf("Global Mem Size (MB): %ld\n",global_mem_size/(1024*1024));
    printf("Global Mem Cache Size (KB): %d\n",(int)global_mem_cache_size/1024);
	printf("Global Mem Cacheline Size (Bytes): %d\n",(int)global_mem_cacheline_size);
	printf("Max Mem Alloc Size Per Mem Object (MB): %ld\n",(long int)max_mem_alloc_size/(1024*1024));
    //printf("Max Printf Buffer Size (MB): %ld\n",(long int)printf_buffer_size/(1024*1024));
	}
	
    // Create an OpenCL context
    clGPUContext = clCreateContext( NULL, 1, &device_id, NULL, NULL, &errcode);
    ASSERT_CL_RETURN(errcode);

    //Create a command-queue
    clCommandQue = clCreateCommandQueue(clGPUContext, device_id, 0, &errcode);
    ASSERT_CL_RETURN(errcode);
}


void cl_clean_up()
{
	// Clean up
	errcode = clFlush(clCommandQue);
	errcode |= clFinish(clCommandQue);

	errcode |= clReleaseKernel(vertex_gauss_kernel);
	errcode |= clReleaseKernel(edge_kernel_1d);
	errcode |= clReleaseKernel(reduce_kernel);

	errcode |= clReleaseProgram(clProgram);
	errcode |= clReleaseCommandQueue(clCommandQue);
	errcode |= clReleaseContext(clGPUContext);
	ASSERT_CL_RETURN(errcode);
}



