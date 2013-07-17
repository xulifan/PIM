/**
 * The implementation of the particle filter using OpenMP for many frames
 * @see http://openmp.org/wp/
 * @note This function is designed to work with a video of several frames. In addition, it references a provided MATLAB function which takes the video, the objxy matrix and the x and y arrays as arguments and returns the likelihoods
 * @param I The video to be run
 * @param IszX The x dimension of the video
 * @param IszY The y dimension of the video
 * @param Nfr The number of frames
 * @param seed The seed array used for random number generation
 * @param Nparticles The number of particles to be used
 */
int particleFilter_pim(unsigned char * I, int IszX, int IszY, int Nfr, int * seed, int Nparticles) {
    int max_size = IszX * IszY*Nfr;
    //original particle centroid
    float xe = roundFloat(IszY / 2.0);
    float ye = roundFloat(IszX / 2.0);

    //expected object locations, compared to center
    int radius = 5;
    int diameter = radius * 2 - 1;
    int * disk = (int*) calloc(diameter * diameter, sizeof (int));
    strelDisk(disk, radius);
    int countOnes = 0;
    int x, y;
    for (x = 0; x < diameter; x++) {
        for (y = 0; y < diameter; y++) {
            if (disk[x * diameter + y] == 1)
                countOnes++;
        }
    }
    int * objxy = (int *) calloc(countOnes * 2, sizeof(int));
    getneighbors(disk, countOnes, objxy, radius);
    //initial weights are all equal (1/Nparticles)
    float * weights = (float *) calloc(Nparticles, sizeof(float));
    for (x = 0; x < Nparticles; x++) {
        weights[x] = 1 / ((float) (Nparticles));
    }


    //initial likelihood to 0.0
    float * likelihood = (float *) calloc(Nparticles + 1, sizeof (float));
    float * arrayX = (float *) calloc(Nparticles, sizeof (float));
    float * arrayY = (float *) calloc(Nparticles, sizeof (float));
    float * xj = (float *) calloc(Nparticles, sizeof (float));
    float * yj = (float *) calloc(Nparticles, sizeof (float));
    float * CDF = (float *) calloc(Nparticles, sizeof(float));

    //GPU copies of arrays
    cl_mem arrayX_GPU;
    cl_mem arrayY_GPU;
    cl_mem xj_GPU;
    cl_mem yj_GPU;
    cl_mem CDF_GPU;
    cl_mem likelihood_GPU;
    cl_mem I_GPU;
    cl_mem weights_GPU;
    cl_mem objxy_GPU;

    int * ind = (int*) calloc(countOnes, sizeof(int));
    cl_mem ind_GPU;
    float * u = (float *) calloc(Nparticles, sizeof(float));
    cl_mem u_GPU;
    cl_mem seed_GPU;
    cl_mem partial_sums;

    
	//Donnie - this loop is different because in this kernel, arrayX and arrayY
    //  are set equal to xj before every iteration, so effectively, arrayX and
    //  arrayY will be set to xe and ye before the first iteration.
    for (x = 0; x < Nparticles; x++) {

        xj[x] = xe;
        yj[x] = ye;
    }

    int k;
    //float * Ik = (float *)calloc(IszX*IszY, sizeof(float));
    int indX, indY;
    //start send
    long long send_start = get_time();



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

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_arrayX;
    void **pim_arrayY;
    void **pim_xj;
    void **pim_yj;
    void **pim_CDF;
    void **pim_u;
    void **pim_likelihood;
    void **pim_weights;
    void **pim_I;
    void **pim_objxy;
    void **pim_ind;
    void **pim_seed;
    void **pim_partial_sums;

    float **pim_mapped_arrayX;
    float **pim_mapped_arrayY;
    float **pim_mapped_xj;
    float **pim_mapped_yj;
    float **pim_mapped_CDF;
    float **pim_mapped_u;
    float **pim_mapped_likelihood;
    float **pim_mapped_weights;
    unsigned char **pim_mapped_I;
    int **pim_mapped_objxy;
    int **pim_mapped_ind;
    int **pim_mapped_seed;
    float **pim_mapped_partial_sums;
    
/********************************************************************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();

/********************************************************************/
/******************** PIM meory allocate ****************************/

    pim_arrayX=(void **)malloc(sizeof(void *)*num_gpus);
    pim_arrayY=(void **)malloc(sizeof(void *)*num_gpus);
    pim_xj=(void **)malloc(sizeof(void *)*num_gpus);
    pim_yj=(void **)malloc(sizeof(void *)*num_gpus);
    pim_CDF=(void **)malloc(sizeof(void *)*num_gpus);
    pim_u=(void **)malloc(sizeof(void *)*num_gpus);
    pim_likelihood=(void **)malloc(sizeof(void *)*num_gpus);
    pim_weights=(void **)malloc(sizeof(void *)*num_gpus);
    pim_I=(void **)malloc(sizeof(void *)*num_gpus);
    pim_objxy=(void **)malloc(sizeof(void *)*num_gpus);
    pim_ind=(void **)malloc(sizeof(void *)*num_gpus);
    pim_seed=(void **)malloc(sizeof(void *)*num_gpus);
    pim_partial_sums=(void **)malloc(sizeof(void *)*num_gpus);
    
    pim_mapped_arrayX=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_arrayY=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_xj=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_yj=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_CDF=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_u=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_likelihood=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_weights=(float **)malloc(sizeof(float *)*num_gpus);
    pim_mapped_I=(unsigned char **)malloc(sizeof(unsigned char *)*num_gpus);
    pim_mapped_objxy=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_ind=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_seed=(int **)malloc(sizeof(int *)*num_gpus);
    pim_mapped_partial_sums=(float **)malloc(sizeof(float *)*num_gpus);
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Domain Decomposition ****************************/

    pim_domain_decomposition(start_point, end_point, own_num_points, num_gpus, Nparticles);
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        printf("GPU %d is calculating %d points from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);
    }    

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Allocate Memory on each PIM *********************/

    printf("Allocating Memory on each PIM\n");
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        pim_arrayX[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_arrayY[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_xj[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_yj[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_CDF[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_u[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_likelihood[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_weights[cur_gpu] = pim_malloc(sizeof (float) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_I[cur_gpu] = pim_malloc(sizeof (unsigned char) *IszX * IszY * Nfr, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_objxy[cur_gpu] = pim_malloc(2*sizeof (int) *countOnes, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_ind[cur_gpu] = pim_malloc(sizeof (int) *countOnes * Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_seed[cur_gpu] = pim_malloc(sizeof (int) *Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
        pim_partial_sums[cur_gpu] = pim_malloc(sizeof (float) * Nparticles, target_gpu[cur_gpu], PIM_MEM_PIM_RW | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Copy Memory to each PIM *************************/
    
    float *partial_sums_cpu=(float *)calloc(Nparticles,sizeof (float));
    
    printf("Copying Memory to each PIM\n");
    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
        /* copy features of points to PIM */
        pim_memcpyHtoD(I,pim_I[cur_gpu],IszX * IszY*Nfr);
        pim_memcpyHtoD(objxy,pim_objxy[cur_gpu],countOnes);
        pim_memcpyHtoD(weights,pim_weights[cur_gpu],Nparticles);
        pim_memcpyHtoD(xj,pim_xj[cur_gpu],Nparticles);
        pim_memcpyHtoD(yj,pim_yj[cur_gpu],Nparticles);
        pim_memcpyHtoD(seed,pim_seed[cur_gpu],Nparticles);

        pim_memcpyHtoD(arrayX,pim_arrayX[cur_gpu],Nparticles);
        pim_memcpyHtoD(arrayY,pim_arrayY[cur_gpu],Nparticles);
        pim_memcpyHtoD(CDF,pim_CDF[cur_gpu],Nparticles);
        pim_memcpyHtoD(u,pim_u[cur_gpu],Nparticles);
        pim_memcpyHtoD(likelihood,pim_likelihood[cur_gpu],Nparticles);
        pim_memcpyHtoD(ind,pim_ind[cur_gpu],countOnes);
        pim_memcpyHtoD(partial_sums_cpu,pim_partial_sums[cur_gpu],Nparticles);
    }

/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** Launch kernels on each PIM **********************/

    printf("Launching Kernels\n");
    for (k = 1; k < Nfr; k++) {
        //printf("K %d\n",k);
        // set partial sum to be all 0s        
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_memcpyHtoD(partial_sums_cpu,pim_partial_sums[cur_gpu],sizeof (float) *Nparticles);
        }

        /****************** L I K E L I H O O D ************************************/
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_launch_likelihood_kernel(pim_arrayX[cur_gpu], pim_arrayY[cur_gpu], pim_xj[cur_gpu], pim_yj[cur_gpu], pim_CDF[cur_gpu], pim_ind[cur_gpu], pim_objxy[cur_gpu], pim_likelihood[cur_gpu], pim_I[cur_gpu], pim_u[cur_gpu], pim_weights[cur_gpu], Nparticles, countOnes, max_size, k, IszY, Nfr, pim_seed[cur_gpu], pim_partial_sums[cur_gpu], start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }
        
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
        }

        
        
        /****************** E N D    L I K E L I H O O D **********************/
        /*************************** S U M ************************************/
        float sum=0;
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_mapped_partial_sums[cur_gpu]=(float *)pim_map(pim_partial_sums[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
            for(int i=0;i<Nparticles;i++) sum+=pim_mapped_partial_sums[cur_gpu][i];
            pim_unmap(pim_mapped_partial_sums[cur_gpu]);
        }
        /*************************** E N D   S U M ****************************/

        // synchronize array weights
        pim_array_sync(weights,pim_mapped_weights,pim_weights,Nparticles,num_gpus);

        //printf("normalize weights\n");
        /**************** N O R M A L I Z E     W E I G H T S *****************/
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_launch_normalize_weights_kernel(pim_weights[cur_gpu], Nparticles, pim_CDF[cur_gpu], pim_u[cur_gpu], pim_seed[cur_gpu], sum, start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
        }

        /************* E N D    N O R M A L I Z E     W E I G H T S ***********/

        // synchronize array m
        pim_array_sync(CDF,pim_mapped_CDF,pim_CDF,Nparticles,num_gpus);

        // /********* I N T E R M E D I A T E     R E S U L T S ***************/

        // synchronize array weights
        pim_array_sync(weights,pim_mapped_weights,pim_weights,Nparticles,num_gpus);
        pim_array_sync(arrayX,pim_mapped_arrayX,pim_arrayX,Nparticles,num_gpus);
        pim_array_sync(arrayY,pim_mapped_arrayY,pim_arrayY,Nparticles,num_gpus);

         xe = 0;
         ye = 0;
         float total=0.0;
         // estimate the object location by expected values
        for (x = 0; x < Nparticles; x++) {
             if( 0.0000000 < arrayX[x]*weights[x]) printf("   arrayX[%d]:%f, arrayY[%d]:%f, weights[%d]:%0.10f\n",x,arrayX[x], x, arrayY[x], x, weights[x]);
             printf("arrayX[%d]:%f | arrayY[%d]:%f | weights[%d]:%f\n", x, arrayX[x], x, arrayY[x], x, weights[x]); 
             xe += arrayX[x] * weights[x];
             ye += arrayY[x] * weights[x];
             total+= weights[x];
         }
         printf("total weight: %f\n", total);
         printf("XE: %f\n", xe);
         printf("YE: %f\n", ye);
         float distance = sqrt(pow((float) (xe - (int) roundFloat(IszY / 2.0)), 2) + pow((float) (ye - (int) roundFloat(IszX / 2.0)), 2));
         printf("%f\n", distance);
        // /********* E N D    I N T E R M E D I A T E     R E S U L T S ***************/


        /******************** F I N D    I N D E X ****************************/
        printf("find index\n");
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_launch_find_index_kernel(pim_arrayX[cur_gpu], pim_arrayY[cur_gpu], pim_CDF[cur_gpu], pim_u[cur_gpu], pim_xj[cur_gpu], pim_yj[cur_gpu], pim_weights[cur_gpu], Nparticles, start_point[cur_gpu], own_num_points[cur_gpu], target_gpu[cur_gpu], &complete_event[cur_gpu]);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
        }
        break;
        /******************* E N D    F I N D    I N D E X ********************/

    }//end loop

    
    
/********************************************************************/
/********************************************************************/

/********************************************************************/
/******************** collect resutls from PIMs *********************/  

    

    xe = 0;
    ye = 0;
    // estimate the object location by expected values
    for (x = 0; x < Nparticles; x++) {
        xe += arrayX[x] * weights[x];
        ye += arrayY[x] * weights[x];
    }
    float distance = sqrt(pow((float) (xe - (int) roundFloat(IszY / 2.0)), 2) + pow((float) (ye - (int) roundFloat(IszX / 2.0)), 2));

    //Output results
    FILE *fid;
    fid=fopen("output.txt", "w+");
    if( fid == NULL ){
      printf( "The file was not opened for writing\n" );
      return -1;
    }
    fprintf(fid, "XE: %lf\n", xe);
    fprintf(fid, "YE: %lf\n", ye);
    fprintf(fid, "distance: %lf\n", distance);
    fclose(fid);

    //free regular memory
    free(likelihood);
    free(arrayX);
    free(arrayY);
    free(xj);
    free(yj);
    free(CDF);
    free(ind);
    free(u);
    free(partial_sums_cpu);
/********************************************************************/
/********************************************************************/

/********************************************************************/
/****************** free memory *************************************/

    for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
        pim_free(pim_arrayX[cur_gpu]);
        pim_free(pim_arrayY[cur_gpu]);
        pim_free(pim_xj[cur_gpu]);
        pim_free(pim_yj[cur_gpu]);
        pim_free(pim_CDF[cur_gpu]);
        pim_free(pim_u[cur_gpu]);
        pim_free(pim_likelihood[cur_gpu]);
        pim_free(pim_weights[cur_gpu]);
        pim_free(pim_I[cur_gpu]);
        pim_free(pim_objxy[cur_gpu]);
        pim_free(pim_ind[cur_gpu]);
        pim_free(pim_seed[cur_gpu]);
        pim_free(pim_partial_sums[cur_gpu]);

    }

    free(pim_arrayX);
    free(pim_arrayY);
    free(pim_xj);
    free(pim_yj);
    free(pim_CDF);
    free(pim_u);
    free(pim_likelihood);
    free(pim_weights);
    free(pim_I);
    free(pim_objxy);
    free(pim_ind);
    free(pim_seed);
    free(pim_partial_sums);

    free(pim_mapped_arrayX);
    free(pim_mapped_arrayY);
    free(pim_mapped_xj);
    free(pim_mapped_yj);
    free(pim_mapped_CDF);
    free(pim_mapped_u);
    free(pim_mapped_likelihood);
    free(pim_mapped_weights);
    free(pim_mapped_I);
    free(pim_mapped_objxy);
    free(pim_mapped_ind);
    free(pim_mapped_seed);
    free(pim_mapped_partial_sums);
    
    free(complete_event);
    free(start_point);
    free(end_point);
    free(own_num_points);

    free(target_gpu);    
    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);

/********************************************************************/
/********************************************************************/

    // **** PIM emulation End Mark  *********  
    pim_emu_end();
    
}


#define BLOCK_SIZE_1D 256
// launch the PIM kernel for likelihood
void pim_launch_likelihood_kernel(void *x, void *y, void *xj, void *yj, void *CDF, void * ind, void *objxy, void *likelihood, void *I, void * u, void *weights, int Nparticles, int countOnes, int max_size, int k, int IszY, int Nfr, void *seed, void *partial_sums, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"particle_single.cl";
    char * kernel_nm = (char *)"likelihood_kernel_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 1;
    int num_pre_event = 0;
    int number=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(number%localItemSize==0)?number:localItemSize*((number/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = x;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = y;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = xj;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = yj;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = CDF;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = ind;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = objxy;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = likelihood;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = I;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = u;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = weights;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &Nparticles;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &countOnes;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &max_size;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &k;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &IszY;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = &Nfr;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = seed;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = partial_sums;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = NULL;
    sizes[tnargs] = localItemSize*sizeof(float);
    tnargs++;

    args[tnargs] = &start_point;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}


// launch the PIM kernel for normalize weights
void pim_launch_normalize_weights_kernel(void *weights, int Nparticles, void *CDF, void *u, void *seed, float partial_sum, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"particle_single.cl";
    char * kernel_nm = (char *)"normalize_weights_kernel_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 1;
    int num_pre_event = 0;
    int number=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(number%localItemSize==0)?number:localItemSize*((number/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = weights;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &Nparticles;
    sizes[tnargs] = sizeof(int);
    tnargs++;

    args[tnargs] = CDF;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = u;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = seed;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &partial_sum;
    sizes[tnargs] = sizeof(float);
    tnargs++;

    args[tnargs] = &start_point;
    sizes[tnargs] = sizeof(int);
    tnargs++;

// insert compeletion event

    args[OPENCL_ARG_POSTEVENT] = complete;
    sizes[OPENCL_ARG_POSTEVENT] = sizeof(cl_event);

    pim_spawn(gpu_kernel, (void**)args, sizes, tnargs, target, PIM_PLATFORM_OPENCL_GPU);

}


// launch the PIM kernel for normalize weights
void pim_launch_find_index_kernel(void *x, void *y, void *CDF, void *u, void *xj, void *yj, void *weights, int Nparticles, int start_point, int own_num_points, pim_device_id target, cl_event *complete)
{
    char * source_nm = (char *)"particle_single.cl";
    char * kernel_nm = (char *)"find_index_kernel_pim";
    pim_f gpu_kernel;
    
    void * args[1024];
    size_t sizes[1024];
    size_t nargs = 0;
    int dimension = 1;
    int num_pre_event = 0;
    int number=own_num_points;
    size_t globalItemSize,localItemSize;

    localItemSize=BLOCK_SIZE_1D;
	
    globalItemSize=(number%localItemSize==0)?number:localItemSize*((number/localItemSize)+1);

    gpu_kernel.func_name = (void*)kernel_nm;

    pim_spawn_args(args,sizes, &nargs, source_nm, (char *)" -D __GPU__", dimension, &globalItemSize, &localItemSize, &num_pre_event, NULL, NULL);

    size_t tnargs = nargs;

    // kernel arguments

    args[tnargs] = x;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = y;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = CDF;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = u;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = xj;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = yj;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = weights;
    sizes[tnargs] = sizeof(void *);
    tnargs++;

    args[tnargs] = &Nparticles;
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
