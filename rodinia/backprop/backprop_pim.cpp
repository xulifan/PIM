int bpnn_train_kernel_pim(BPNN *net, float *eo, float *eh)
{
	int in, hid, out;
	float out_err, hid_err;
  
	in = net->input_n;
	hid = net->hidden_n;
	out = net->output_n;   
   
	
	float *input_weights_one_dim;
    float *input_weights_prev_one_dim;
	float * partial_sum;
	float sum;
	int num_blocks = in / BLOCK_SIZE;
	if(in%BLOCK_SIZE!=0) {printf("Error input size\n"); exit(-1);}
	input_weights_one_dim = (float *) malloc((in + 1)* (hid + 1) * sizeof(float));
	input_weights_prev_one_dim = (float *) malloc((in + 1)* (hid + 1) * sizeof(float));
	partial_sum = (float *) malloc(num_blocks * WIDTH * sizeof(float));
	
	// this preprocessing stage is temporarily added to correct the bug of wrong memcopy using two-dimensional net->inputweights
	// todo: fix mem allocation
	int m = 0;
	for (int k = 0; k <= in; k++) {	
		for (int j = 0; j <= hid; j++) {
		input_weights_one_dim[m] = net->input_weights[k][j];
		input_weights_prev_one_dim[m] = net-> input_prev_weights[k][j];
	    m++;
		}
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

    void **pim_input_hidden;
    void **pim_input;
    void **pim_output_hidden;
    void **pim_hidden_partial_sum;
    void **pim_hidden_delta;
    void **pim_input_prev_weights;

    float **pim_mapped_input_hidden;
    float **pim_mapped_input;
    float **pim_mapped_output_hidden;
    float **pim_mapped_hidden_partial_sum;
    float **pim_mapped_hidden_delta;
    float **pim_mapped_input_prev_weights;
    
/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_input_hidden=(void **)malloc(sizeof(void *)*num_gpus);
    pim_input=(void **)malloc(sizeof(void *)*num_gpus);
    pim_output_hidden=(void **)malloc(sizeof(void *)*num_gpus);
    pim_hidden_partial_sum=(void **)malloc(sizeof(void *)*num_gpus);
    pim_hidden_delta=(void **)malloc(sizeof(void *)*num_gpus);
    pim_input_prev_weights=(void **)malloc(sizeof(void *)*num_gpus);

    pim_mapped_input_hidden=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_input=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_output_hidden=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_hidden_partial_sum=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_hidden_delta=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_input_prev_weights=(float **)malloc(sizeof(float *)*num_gpus);
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    //pim_domain_decomposition(start_point_x, end_point_x, own_num_points_x, num_gpus, blockCols);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        own_num_points_x[cur_gpu]=1;
        start_point_x[cur_gpu]=0;
        end_point_x[cur_gpu]=1;        
        printf("GPU %d is calculating %d records from %d to %d for X\n",cur_gpu,own_num_points_x[cur_gpu],start_point_x[cur_gpu],end_point_x[cur_gpu]);
    }

    pim_domain_decomposition(start_point_y, end_point_y, own_num_points_y, num_gpus, num_blocks);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d records from %d to %d for Y\n",cur_gpu,own_num_points_y[cur_gpu],start_point_y[cur_gpu],end_point_y[cur_gpu]);
    }


/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_input[cur_gpu] = pim_malloc((in + 1) * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_input_hidden[cur_gpu] = pim_malloc((in + 1) * (hid + 1) * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_output_hidden[cur_gpu] = pim_malloc((hid + 1) * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_hidden_partial_sum[cur_gpu] = pim_malloc(num_blocks * WIDTH * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_hidden_delta[cur_gpu] = pim_malloc((hid + 1) * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_input_prev_weights[cur_gpu] = pim_malloc((in + 1) * (hid + 1) * sizeof(float), target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/

	
    float *hidden_partial_sum=(float *)calloc(num_blocks * WIDTH,sizeof(float));
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_memcpyHtoD(net->input_units,pim_input[cur_gpu],in + 1);
        pim_memcpyHtoD(input_weights_one_dim,pim_input_hidden[cur_gpu],(in + 1) * (hid + 1));
        pim_memcpyHtoD(hidden_partial_sum,pim_hidden_partial_sum[cur_gpu],num_blocks * WIDTH);

    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/



		for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

            // launch the PIM kernel
            pim_launch_layerforward_kernel(pim_input[cur_gpu], pim_output_hidden[cur_gpu], pim_input_hidden[cur_gpu], pim_hidden_partial_sum[cur_gpu], in, hid, start_point_y[cur_gpu], own_num_points_y[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);            
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
        pim_array_sync(hidden_partial_sum,pim_mapped_hidden_partial_sum,pim_hidden_partial_sum,num_blocks * WIDTH,num_gpus);
        //pim_array_sync(input_weights_one_dim,pim_mapped_input_hidden,pim_input_hidden,(in + 1) * (hid + 1),num_gpus);
        
        for (int j = 1; j <= hid; j++) {
		    sum = 0.0;
		    for (int k = 0; k < num_blocks; k++) {	
		        sum += partial_sum[k * hid + j-1] ;
            }
		    sum += net->input_weights[0][j];
		    net-> hidden_units[j] = float(1.0 / (1.0 + exp(-sum)));
	    }

	
	    bpnn_layerforward(net->hidden_units, net->output_units, net->hidden_weights, hid, out);
	    bpnn_output_error(net->output_delta, net->target, net->output_units, out, &out_err);
	    bpnn_hidden_error(net->hidden_delta, hid, net->output_delta, out, net->hidden_weights, net->hidden_units, &hid_err);  
	    bpnn_adjust_weights(net->output_delta, out, net->hidden_units, hid, net->hidden_weights, net->hidden_prev_weights);

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_memcpyHtoD(net->hidden_delta,pim_hidden_delta[cur_gpu],hid + 1);
        pim_memcpyHtoD(input_weights_prev_one_dim,pim_input_prev_weights[cur_gpu],(in + 1) * (hid + 1));
        pim_memcpyHtoD(input_weights_one_dim,pim_input_hidden[cur_gpu],(in + 1) * (hid + 1));

    }        


        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

            // launch the PIM kernel
            pim_launch_adjust_weights_kernel(pim_hidden_delta[cur_gpu], pim_input[cur_gpu], pim_input_hidden[cur_gpu], pim_input_prev_weights[cur_gpu], in, hid, start_point_y[cur_gpu], own_num_points_y[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);           
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
    
    pim_array_sync(net->input_units,pim_mapped_input,pim_input,in + 1,num_gpus);
    pim_array_sync(input_weights_one_dim,pim_mapped_input_hidden,pim_input_hidden,(in + 1) * (hid + 1),num_gpus);

    FILE *fp=fopen("result.txt","w");
    for(int i=0;i<in+1;i++) fprintf(fp,"%f\n ",net->input_units[i]);
    fclose(fp);
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/




    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_input_hidden[cur_gpu]);
        pim_free(pim_input[cur_gpu]);
        pim_free(pim_output_hidden[cur_gpu]);
        pim_free(pim_hidden_partial_sum[cur_gpu]);
        pim_free(pim_hidden_delta[cur_gpu]);
        pim_free(pim_input_prev_weights[cur_gpu]);
        
    }

    free(pim_input_hidden);
    free(pim_input);
    free(pim_output_hidden);
    free(pim_hidden_partial_sum);
    free(pim_hidden_delta);
    free(pim_input_prev_weights);

    free(pim_mapped_input_hidden);
    free(pim_mapped_input);
    free(pim_mapped_output_hidden);
    free(pim_mapped_hidden_partial_sum);
    free(pim_mapped_hidden_delta);
    free(*pim_mapped_input_prev_weights);
    
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

    free(input_weights_prev_one_dim);
	free(partial_sum);
	free(input_weights_one_dim);



}



#define BLOCK_SIZE_2D 16
// launch the PIM kernel for FAN1
void pim_launch_layerforward_kernel(void *input, void *output_hidden, void *input_hidden, void *hidden_partial_sum,int in, int hid, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"backprop_kernel.cl";
    char * kernel_nm = (char *)"bpnn_layerforward_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 2;
    int num_pre_event = 0;
    size_t globalItemSize[2],localItemSize[2];

    localItemSize[0]=BLOCK_SIZE_2D;
    localItemSize[1]=BLOCK_SIZE_2D;
	
    globalItemSize[0]=localItemSize[0];
    globalItemSize[1]=own_num_points*localItemSize[1];

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, globalItemSize, localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = input;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = output_hidden;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = input_hidden;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = hidden_partial_sum;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = sizeof(float) *  HEIGHT;
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = sizeof(float) *  HEIGHT * WIDTH;
    tnargs++;

    args[tnargs] = &in;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &hid;
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

 
void pim_launch_adjust_weights_kernel(void *delta, void *ly, void *w, void *oldw, int in, int hid, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"backprop_kernel.cl";
    char * kernel_nm = (char *)"bpnn_adjust_weights_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 2;
    int num_pre_event = 0;
    size_t globalItemSize[2],localItemSize[2];

    localItemSize[0]=BLOCK_SIZE_2D;
    localItemSize[1]=BLOCK_SIZE_2D;
	
    globalItemSize[0]=localItemSize[0];
    globalItemSize[1]=own_num_points*localItemSize[1];

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, globalItemSize, localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = delta;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = ly;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = w;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = oldw;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &in;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &hid;
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
