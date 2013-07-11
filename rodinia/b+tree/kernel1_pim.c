void 
kernel1_pim_wrapper(	record *records,
							long records_mem,
							knode *knodes,
							long knodes_elem,
							long knodes_mem,

							int order,
							long maxheight,
							int count,

							long *currKnode,
							long *offset,
							int *keys,
							record *ans)
{

	// timer
	long long time0;
	long long time1;
	long long time2;
	long long time3;
	long long time4;
	long long time5;
	long long time6;

	time0 = get_time();

	
	// common variables
	cl_int error;

	
	time1 = get_time();

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

/******************** PIM initialization end ************************/
/********************************************************************/


    int points_per_gpu=(count+num_gpus-1)/num_gpus;
    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_records;
    void **pim_knodes;
    void **pim_currKnode;
    void **pim_offset;
    void **pim_keys;
    void **pim_ans;

    record **pim_mapped_records;
    knode **pim_mapped_knodes;
    long **pim_mapped_currKnode;
    long **pim_mapped_offset;
    int **pim_mapped_keys;
    record **pim_mapped_ans;

/******************** PIM memory variable end ***********************/
/********************************************************************/


// **** PIM emulation Start Mark  *********
    pim_emu_begin();



/********************************************************************/
/******************** PIM meory allocate ****************************/
    pim_records=(void **)malloc(sizeof(void *)*num_gpus);
    pim_knodes=(void **)malloc(sizeof(void *)*num_gpus);
    pim_currKnode=(void **)malloc(sizeof(void *)*num_gpus);
    pim_offset=(void **)malloc(sizeof(void *)*num_gpus);
    pim_keys=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_records=(record **)malloc(sizeof(record *)*num_gpus);
    pim_mapped_knodes=(knode **)malloc(sizeof(knode *)*num_gpus);
    pim_mapped_currKnode=(long **)malloc(sizeof(long *)*num_gpus);
    pim_mapped_offset=(long **)malloc(sizeof(long *)*num_gpus);
    pim_mapped_keys=(int **)malloc(sizeof(int *)*num_gpus);

/******************** PIM memory variable allocate end **************/
/********************************************************************/


    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

        start_point[cur_gpu]= cur_gpu*points_per_gpu;
        end_point[cur_gpu]= start_point[cur_gpu]+points_per_gpu;
        if(end_point[cur_gpu]>count) end_point[cur_gpu]=count;
        own_num_points[cur_gpu] = end_point[cur_gpu]-start_point[cur_gpu];
        printf("GPU %d is calculating %d records from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);

        /* allocate memory for PIM */
        pim_records[cur_gpu] = pim_malloc(records_mem, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_knodes[cur_gpu] = pim_malloc(knodes_mem, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_currKnode[cur_gpu] = pim_malloc(count*sizeof(long), target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_offset[cur_gpu] = pim_malloc(count*sizeof(long), target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_keys[cur_gpu] = pim_malloc(count*sizeof(int), target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
        pim_ans[cur_gpu] = pim_malloc(count*sizeof(record), target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);

        /* copy features of points to PIM */
        pim_mapped_records[cur_gpu] = (record *)pim_map(pim_records[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_knodes[cur_gpu] = (record *)pim_map(pim_knodes[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_currKnode[cur_gpu] = (record *)pim_map(pim_currKnode[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_offset[cur_gpu] = (record *)pim_map(pim_offset[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_keys[cur_gpu] = (record *)pim_map(pim_keys[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
        pim_mapped_ans[cur_gpu] = (record *)pim_map(pim_ans[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

        memcpy(pim_mapped_records[cur_gpu],records,records_mem);
        memcpy(pim_mapped_knodes[cur_gpu],knodes,knodes_mem);
        memcpy(pim_mapped_currKnode[cur_gpu],currKnode,count*sizeof(long));
        memcpy(pim_mapped_offset[cur_gpu],offset,count*sizeof(long));
        memcpy(pim_mapped_keys[cur_gpu],keys,count*sizeof(int));
        memcpy(pim_mapped_ans[cur_gpu],ans,count*sizeof(record));

        pim_unmap(pim_mapped_records[cur_gpu]);
        pim_unmap(pim_mapped_knodes[cur_gpu]);
        pim_unmap(pim_mapped_currKnode[cur_gpu]);
        pim_unmap(pim_mapped_offset[cur_gpu]);
        pim_unmap(pim_mapped_keys[cur_gpu]);
        pim_unmap(pim_mapped_ans[cur_gpu]);













        // launch the PIM kernel
        //pim_launch_nn_kernel(pim_locations[cur_gpu], pim_distances[cur_gpu], own_num_points[cur_gpu], lat, lng, target_gpu[cur_gpu], &complete_event[cur_gpu]);

    }

	time2 = get_time();

	//==================================================50
	//	END
	//==================================================50

	//====================================================================================================100
	//	END
	//====================================================================================================100

	//======================================================================================================================================================150
	//	GPU MEMORY			COPY
	//======================================================================================================================================================150

	//====================================================================================================100
	//	GPU MEMORY				(MALLOC) COPY IN
	//====================================================================================================100

	//==================================================50
	//	recordsD
	//==================================================50

	error = clEnqueueWriteBuffer(	command_queue,			// command queue
									recordsD,				// destination
									1,						// block the source from access until this copy operation complates (1=yes, 0=no)
									0,						// offset in destination to write to
									records_mem,			// size to be copied
									records,				// source
									0,						// # of events in the list of events to wait for
									NULL,					// list of events to wait for
									NULL);					// ID of this operation to be used by waiting operations
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	//==================================================50
	//	knodesD
	//==================================================50

	error = clEnqueueWriteBuffer(	command_queue,			// command queue
									knodesD,				// destination
									1,						// block the source from access until this copy operation complates (1=yes, 0=no)
									0,						// offset in destination to write to
									knodes_mem,				// size to be copied
									knodes,					// source
									0,						// # of events in the list of events to wait for
									NULL,					// list of events to wait for
									NULL);					// ID of this operation to be used by waiting operations
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	//==================================================50
	//	currKnodeD
	//==================================================50

	error = clEnqueueWriteBuffer(	command_queue,			// command queue
									currKnodeD,				// destination
									1,						// block the source from access until this copy operation complates (1=yes, 0=no)
									0,						// offset in destination to write to
									count*sizeof(long),		// size to be copied
									currKnode,				// source
									0,						// # of events in the list of events to wait for
									NULL,					// list of events to wait for
									NULL);					// ID of this operation to be used by waiting operations
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	//==================================================50
	//	offsetD
	//==================================================50

	error = clEnqueueWriteBuffer(	command_queue,			// command queue
									offsetD,				// destination
									1,						// block the source from access until this copy operation complates (1=yes, 0=no)
									0,						// offset in destination to write to
									count*sizeof(long),		// size to be copied
									offset,					// source
									0,						// # of events in the list of events to wait for
									NULL,					// list of events to wait for
									NULL);					// ID of this operation to be used by waiting operations
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	//==================================================50
	//	keysD
	//==================================================50

	error = clEnqueueWriteBuffer(	command_queue,			// command queue
									keysD,					// destination
									1,						// block the source from access until this copy operation complates (1=yes, 0=no)
									0,						// offset in destination to write to
									count*sizeof(int),		// size to be copied
									keys,					// source
									0,						// # of events in the list of events to wait for
									NULL,					// list of events to wait for
									NULL);					// ID of this operation to be used by waiting operations
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	//==================================================50
	//	END
	//==================================================50

	//====================================================================================================100
	//	DEVICE IN/OUT
	//====================================================================================================100

	//==================================================50
	//	ansD
	//==================================================50

	error = clEnqueueWriteBuffer(	command_queue,			// command queue
									ansD,					// destination
									1,						// block the source from access until this copy operation complates (1=yes, 0=no)
									0,						// offset in destination to write to
									count*sizeof(record),	// size to be copied
									ans,					// source
									0,						// # of events in the list of events to wait for
									NULL,					// list of events to wait for
									NULL);					// ID of this operation to be used by waiting operations
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	time3 = get_time();

	//==================================================50
	//	END
	//==================================================50

	//====================================================================================================100
	//	END
	//====================================================================================================100

	//======================================================================================================================================================150
	// findK kernel
	//======================================================================================================================================================150

	//====================================================================================================100
	//	Execution Parameters
	//====================================================================================================100

	size_t local_work_size[1];
	local_work_size[0] = order < 1024 ? order : 1024;
	size_t global_work_size[1];
	global_work_size[0] = count * local_work_size[0];

	printf("# of blocks = %d, # of threads/block = %d (ensure that device can handle)\n", (int)(global_work_size[0]/local_work_size[0]), (int)local_work_size[0]);

	//====================================================================================================100
	//	Kernel Arguments
	//====================================================================================================100

	clSetKernelArg(	kernel, 
					0, 
					sizeof(long), 
					(void *) &maxheight);
	clSetKernelArg(	kernel, 
					1, 
					sizeof(cl_mem), 
					(void *) &knodesD);
	clSetKernelArg(	kernel, 
					2, 
					sizeof(long), 
					(void *) &knodes_elem);
	clSetKernelArg(	kernel, 
					3, 
					sizeof(cl_mem), 
					(void *) &recordsD);

	clSetKernelArg(	kernel, 
					4, 
					sizeof(cl_mem), 
					(void *) &currKnodeD);
	clSetKernelArg(	kernel, 
					5, 
					sizeof(cl_mem), 
					(void *) &offsetD);
	clSetKernelArg(	kernel, 
					6, 
					sizeof(cl_mem), 
					(void *) &keysD);
	clSetKernelArg(	kernel, 
					7, 
					sizeof(cl_mem), 
					(void *) &ansD);

	//====================================================================================================100
	//	Kernel
	//====================================================================================================100

	error = clEnqueueNDRangeKernel(	command_queue, 
									kernel, 
									1, 
									NULL, 
									global_work_size, 
									local_work_size, 
									0, 
									NULL, 
									NULL);
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	// Wait for all operations to finish NOT SURE WHERE THIS SHOULD GO
	error = clFinish(command_queue);
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	time4 = get_time();

	//====================================================================================================100
	//	END
	//====================================================================================================100

	//======================================================================================================================================================150
	//	GPU MEMORY			COPY (CONTD.)
	//======================================================================================================================================================150

	//====================================================================================================100
	//	DEVICE IN/OUT
	//====================================================================================================100

	//==================================================50
	//	ansD
	//==================================================50

	error = clEnqueueReadBuffer(command_queue,				// The command queue.
								ansD,						// The image on the device.
								CL_TRUE,					// Blocking? (ie. Wait at this line until read has finished?)
								0,							// Offset. None in this case.
								count*sizeof(record),		// Size to copy.
								ans,						// The pointer to the image on the host.
								0,							// Number of events in wait list. Not used.
								NULL,						// Event wait list. Not used.
								NULL);						// Event object for determining status. Not used.
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	time5 = get_time();

	//==================================================50
	//	END
	//==================================================50

	//====================================================================================================100
	//	END
	//====================================================================================================100

	//======================================================================================================================================================150
	//	GPU MEMORY DEALLOCATION
	//======================================================================================================================================================150

	// Release kernels...
	clReleaseKernel(kernel);

	// Now the program...
	clReleaseProgram(program);

	// Clean up the device memory...
	clReleaseMemObject(recordsD);
	clReleaseMemObject(knodesD);

	clReleaseMemObject(currKnodeD);
	clReleaseMemObject(offsetD);
	clReleaseMemObject(keysD);
	clReleaseMemObject(ansD);

	// Flush the queue
	error = clFlush(command_queue);
	if (error != CL_SUCCESS) 
		fatal_CL(error, __LINE__);

	// ...and finally, the queue and context.
	clReleaseCommandQueue(command_queue);

	// ???
	clReleaseContext(context);

	time6 = get_time();

	//======================================================================================================================================================150
	//	DISPLAY TIMING
	//======================================================================================================================================================150

	printf("Time spent in different stages of GPU_CUDA KERNEL:\n");

	printf("%15.12f s, %15.12f % : GPU: SET DEVICE / DRIVER INIT\n",	(float) (time1-time0) / 1000000, (float) (time1-time0) / (float) (time6-time0) * 100);
	printf("%15.12f s, %15.12f % : GPU MEM: ALO\n", 					(float) (time2-time1) / 1000000, (float) (time2-time1) / (float) (time6-time0) * 100);
	printf("%15.12f s, %15.12f % : GPU MEM: COPY IN\n",					(float) (time3-time2) / 1000000, (float) (time3-time2) / (float) (time6-time0) * 100);

	printf("%15.12f s, %15.12f % : GPU: KERNEL\n",						(float) (time4-time3) / 1000000, (float) (time4-time3) / (float) (time6-time0) * 100);

	printf("%15.12f s, %15.12f % : GPU MEM: COPY OUT\n",				(float) (time5-time4) / 1000000, (float) (time5-time4) / (float) (time6-time0) * 100);
	printf("%15.12f s, %15.12f % : GPU MEM: FRE\n", 					(float) (time6-time5) / 1000000, (float) (time6-time5) / (float) (time6-time0) * 100);

	printf("Total time:\n");
	printf("%.12f s\n", 												(float) (time6-time0) / 1000000);

	//======================================================================================================================================================150
	//	END
	//======================================================================================================================================================150

}
