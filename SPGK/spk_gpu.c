

void SPGK_GPU()
{
    double t_start,t_end,t_start1,t_end1;
    //for(int i=0;i<num_graph;i++) printf("%d %d %d\n",i,graph[i].n_node,graph[i].n_edge);

    t_start=rtclock();
	//for(int i=0;i<num_graph;i++){
	for(int i=num_graph-1;i>=0;i--){    
	    n_node1=graph[i].n_node;
	    n_edge1=graph[i].n_sp_edge;
	    //printf("%d %d\n",n_node1,n_edge1);
	    //print2d(graph[i].feat,n_node1,n_feat);
        //print2d(graph[i].adj,n_node1,n_node1);
	    t_start1=rtclock();
        cl_feat_g1 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_node1*n_feat, NULL, &errcode);	    
	    cl_edge_g1 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_edge1, NULL, &errcode);
	    cl_edge_x1 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(int)*n_edge1, NULL, &errcode);
	    cl_edge_y1 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(int)*n_edge1, NULL, &errcode);
	
	    errcode |= clEnqueueWriteBuffer(clCommandQue, cl_feat_g1, CL_TRUE, 0, sizeof(double)*n_node1*n_feat, graph[i].feat[0], 0, NULL, NULL);
	    errcode |= clEnqueueWriteBuffer(clCommandQue, cl_edge_g1, CL_TRUE, 0, sizeof(double)*n_edge1,graph[i].sp_edge_w, 0, NULL, NULL);
	    errcode |= clEnqueueWriteBuffer(clCommandQue, cl_edge_x1, CL_TRUE, 0, sizeof(int)*n_edge1, graph[i].sp_edge_x, 0, NULL, NULL);
	    errcode |= clEnqueueWriteBuffer(clCommandQue, cl_edge_y1, CL_TRUE, 0, sizeof(int)*n_edge1, graph[i].sp_edge_y, 0, NULL, NULL);
	    ASSERT_CL_RETURN(errcode);
	    clFinish(clCommandQue);
	    t_end1=rtclock();
	    mem_init_time += t_end1-t_start1;
        //for(int j=i;j<num_graph;j++){
        for(int j=0;j<=i;j++){
            
            n_node2=graph[j].n_node;
            n_edge2=graph[j].n_sp_edge;
            t_start1=rtclock();
	        cl_feat_g2 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_node2*n_feat, NULL, &errcode);
	        cl_edge_g2 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_edge2, NULL, &errcode);
	        cl_edge_x2 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(int)*n_edge2, NULL, &errcode);
	        cl_edge_y2 = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(int)*n_edge2, NULL, &errcode);
	        
	        errcode |= clEnqueueWriteBuffer(clCommandQue, cl_feat_g2, CL_TRUE, 0, sizeof(double)*n_node2*n_feat, graph[j].feat[0], 0, NULL, NULL);
	        errcode |= clEnqueueWriteBuffer(clCommandQue, cl_edge_g2, CL_TRUE, 0, sizeof(double)*n_edge2,graph[j].sp_edge_w, 0, NULL, NULL);
	        errcode |= clEnqueueWriteBuffer(clCommandQue, cl_edge_x2, CL_TRUE, 0, sizeof(int)*n_edge2, graph[j].sp_edge_x, 0, NULL, NULL);
	        errcode |= clEnqueueWriteBuffer(clCommandQue, cl_edge_y2, CL_TRUE, 0, sizeof(int)*n_edge2, graph[j].sp_edge_y, 0, NULL, NULL);
	        ASSERT_CL_RETURN(errcode);
            clFinish(clCommandQue);
	        t_end1=rtclock();
	        mem_init_time += t_end1-t_start1;
	        
	        K_Matrix[i][j]=execute_spgk_gpu(i,j);
            K_Matrix[j][i]=K_Matrix[i][j];
            
                        
            errcode |= clReleaseMemObject(cl_feat_g2);
	        errcode |= clReleaseMemObject(cl_edge_g2);
	        errcode |= clReleaseMemObject(cl_edge_x2);
	        errcode |= clReleaseMemObject(cl_edge_y2);
	        ASSERT_CL_RETURN(errcode);
                        
        }
        errcode |= clReleaseMemObject(cl_feat_g1);
	    errcode |= clReleaseMemObject(cl_edge_g1);
	    errcode |= clReleaseMemObject(cl_edge_x1);
	    errcode |= clReleaseMemObject(cl_edge_y1);
        ASSERT_CL_RETURN(errcode);
    }
    
    t_end=rtclock();
    gpu3_total_time=t_end-t_start;

    
    
}


double execute_spgk_gpu(int g1, int g2)
{
    double paramx = vk_params[0];
    double paramy = vk_params[1];

   
    cl_vertex = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_node1*n_node2, NULL, &errcode);
    
	if(n_edge1>n_edge2) cl_edge = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_edge1, NULL, &errcode);
	else cl_edge = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*n_edge2, NULL, &errcode);
	ASSERT_CL_RETURN(errcode);
	
	
    if(n_edge1>=n_edge2){
        cl_launch_vert_gauss_kernel(cl_vertex,cl_feat_g1,cl_feat_g2,n_node1,n_node2,n_feat,paramy);
        cl_launch_edge_kernel_1d(cl_edge, cl_vertex, cl_edge_g1, cl_edge_g2, cl_edge_x1, cl_edge_x2, cl_edge_y1, cl_edge_y2, n_edge1, n_edge2, n_node1, n_node2, paramx);
    }
    else{
        cl_launch_vert_gauss_kernel(cl_vertex,cl_feat_g2,cl_feat_g1,n_node2,n_node1,n_feat,paramy);
        cl_launch_edge_kernel_1d(cl_edge, cl_vertex, cl_edge_g2, cl_edge_g1, cl_edge_x2, cl_edge_x1, cl_edge_y2, cl_edge_y1, n_edge2, n_edge1, n_node2, n_node1, paramx);
    }
	
	size_t local=BLOCK_SIZE_1D;
	int num;
    if(n_edge1>n_edge2) num=n_edge1;
    else num=n_edge2;
    int outputsize=(num%local==0)?num/local:(( num/local )+ 1);
    cl_mem cl_reduce_output = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, sizeof(double)*outputsize, NULL, &errcode);
    ASSERT_CL_RETURN(errcode);
	
    cl_launch_reduce(cl_edge, cl_reduce_output, num);

    double sum = 0;
    gpu_mem_sum(cl_reduce_output,outputsize,&sum);

	errcode = clFlush(clCommandQue);
	errcode = clReleaseMemObject(cl_vertex);
	errcode |= clReleaseMemObject(cl_edge);
	errcode |= clReleaseMemObject(cl_reduce_output);
	ASSERT_CL_RETURN(errcode);
	
	//printf("finish kernel execution\n");
	
	return sum;
	
}


template <typename T> void gpu_mem_sum(cl_mem a,int n, T *sum)
{
    double t_start,t_end;
    T *agg_output = (T *)calloc(n,sizeof(T));
    
    t_start = rtclock();
    //cl_double* agg_output = (cl_double*)clEnqueueMapBuffer(clCommandQue,a,CL_FALSE,CL_MAP_READ,0,sizeof(double)*n, 0,NULL,NULL, &errcode);
	errcode = clEnqueueReadBuffer(clCommandQue, a, CL_TRUE, 0, sizeof(T)*n, agg_output, 0, NULL, NULL);
    OpenCLErrorCheck(errcode,"Agg Memory reading");
    errcode = clFinish(clCommandQue);
	t_end = rtclock();
	gpu_sum_mem_cpy_time +=t_end - t_start;

    *sum=0;
    t_start = rtclock();
	for(int i=0;i<n;i++) *sum+=agg_output[i];
    t_end = rtclock();
    gpu_sum_time +=t_end - t_start;
	//printf("k is %.1lf\n",sum);
    //errcode = clEnqueueUnmapMemObject(clCommandQue,a,(void*)agg_output,0,NULL,NULL);
    free(agg_output);

    return;
}

/* 
** reduction kernel
** sum up input[] into result[]
** result = sum(input)
*/
void cl_launch_reduce(cl_mem input, cl_mem result, int num)
{
    double t_start,t_end;
    size_t global, local;
    local=BLOCK_SIZE_1D;
    global=(num%local==0)?num:local*((num/local)+1);

    t_start = rtclock();
    errcode =  clSetKernelArg(reduce_kernel, 0, sizeof(cl_mem), (void *)&input);
	errcode |=  clSetKernelArg(reduce_kernel, 1, sizeof(cl_mem), (void *)&result);
	errcode |=  clSetKernelArg(reduce_kernel, 2, sizeof(int), (void *)&num);
	errcode |=  clSetKernelArg(reduce_kernel, 3, local * sizeof(double), NULL);
		
	// Execute the OpenCL kernel
	errcode |= clEnqueueNDRangeKernel(clCommandQue, reduce_kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	errcode |= clFinish(clCommandQue); 
    OpenCLErrorCheck(errcode,"Launching reduce_kernel");
	t_end = rtclock();
	reduce_time +=t_end - t_start;
}

/*
** Gaussian kernel for vertex
** 
*/
void cl_launch_vert_gauss_kernel(cl_mem vert, cl_mem feat1, cl_mem feat2, int n1, int n2, int nfeat, double y)
{
    double t_start, t_end;
    size_t globalItemSize[2],localItemSize[2];

	localItemSize[0]=128;
	localItemSize[1]=2;
	
	globalItemSize[0]=(n1%localItemSize[0]==0)?n1:localItemSize[0]*((n1/localItemSize[0])+1);
	globalItemSize[1]=(n2%localItemSize[1]==0)?n2:localItemSize[1]*((n2/localItemSize[1])+1);
	
	t_start = rtclock();
	errcode =  clSetKernelArg(vertex_gauss_kernel, 0, sizeof(cl_mem), (void *)&vert);
	errcode |=  clSetKernelArg(vertex_gauss_kernel, 1, sizeof(cl_mem), (void *)&feat1);
	errcode |=  clSetKernelArg(vertex_gauss_kernel, 2, sizeof(cl_mem), (void *)&feat2);
    errcode |=  clSetKernelArg(vertex_gauss_kernel, 3, sizeof(int), (void *)&n_feat);
	errcode |=  clSetKernelArg(vertex_gauss_kernel, 4, sizeof(int), (void *)&n1);
	errcode |=  clSetKernelArg(vertex_gauss_kernel, 5, sizeof(int), (void *)&n2);
    errcode |=  clSetKernelArg(vertex_gauss_kernel, 6, sizeof(double), (void *)&y);
    // Execute the OpenCL kernel
	errcode |= clEnqueueNDRangeKernel(clCommandQue, vertex_gauss_kernel, 2, NULL, globalItemSize, localItemSize, 0, NULL, NULL);
	errcode |= clFinish(clCommandQue); 
    OpenCLErrorCheck(errcode,"Launching vertex_gauss_kernel");
	t_end = rtclock();
	vertex_kernel_time+=t_end-t_start;

}


void cl_launch_edge_kernel_1d(cl_mem edge, cl_mem vert, cl_mem w1, cl_mem w2, cl_mem x1, cl_mem x2, cl_mem y1, cl_mem y2, int edge1, int edge2, int node1, int node2, double para)
{
    size_t global,local;
    double t_start,t_end;
    int num=edge1;
    t_start=rtclock();
    local=BLOCK_SIZE_1D;
    global=(num%local==0)?num:local*((num/local) + 1);

    t_start = rtclock();
	errcode =  clSetKernelArg(edge_kernel_1d, 0, sizeof(cl_mem), (void *)&edge);
	errcode |=  clSetKernelArg(edge_kernel_1d, 1, sizeof(cl_mem), (void *)&vert);
	errcode |=  clSetKernelArg(edge_kernel_1d, 2, sizeof(cl_mem), (void *)&w1);
	errcode |=  clSetKernelArg(edge_kernel_1d, 3, sizeof(cl_mem), (void *)&w2);
	errcode |=  clSetKernelArg(edge_kernel_1d, 4, sizeof(cl_mem), (void *)&x1);
	errcode |=  clSetKernelArg(edge_kernel_1d, 5, sizeof(cl_mem), (void *)&x2);
	errcode |=  clSetKernelArg(edge_kernel_1d, 6, sizeof(cl_mem), (void *)&y1);
	errcode |=  clSetKernelArg(edge_kernel_1d, 7, sizeof(cl_mem), (void *)&y2);
	errcode |=  clSetKernelArg(edge_kernel_1d, 8, sizeof(int), (void *)&edge1);
	errcode |=  clSetKernelArg(edge_kernel_1d, 9, sizeof(int), (void *)&edge2);
	errcode |=  clSetKernelArg(edge_kernel_1d, 10, sizeof(int), (void *)&node1);
	errcode |=  clSetKernelArg(edge_kernel_1d, 11, sizeof(int), (void *)&node2);
	errcode |=  clSetKernelArg(edge_kernel_1d, 12, sizeof(double), (void *)&para);
	// Execute the OpenCL kernel
	errcode = clEnqueueNDRangeKernel(clCommandQue, edge_kernel_1d, 1, NULL, &global, &local, 0, NULL, NULL);
	errcode |= clFinish(clCommandQue); 
	OpenCLErrorCheck(errcode,"Launching edge_kernel_1d");
    t_end = rtclock();
	edge_kernel_time +=t_end - t_start;
	
}
