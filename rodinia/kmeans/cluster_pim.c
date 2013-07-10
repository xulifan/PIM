

	

int cluster_pim(int      npoints,				/* number of data points */
            int      nfeatures,				/* number of attributes for each point */
            float  **features,			/* array: [npoints][nfeatures] */                  
            int      min_nclusters,			/* range of min to max number of clusters */
			int		 max_nclusters,
            float    threshold,				/* loop terminating factor */
            int     *best_nclusters,		/* out: number between min and max with lowest RMSE */
            float ***cluster_centres,		/* out: [best_nclusters][nfeatures] */
			float	*min_rmse,				/* out: minimum RMSE */
			int		 isRMSE,				/* calculate RMSE */
			int		 nloops					/* number of iteration for each number of clusters */
			)
{    
	int		nclusters;						/* number of clusters k */	
	int		index =0;						/* number of iteration to reach the best RMSE */
	int		rmse;							/* RMSE for each clustering */
    int    *membership;						/* which cluster a data point belongs to */
    //float **tmp_cluster_centers;			/* hold coordinates of cluster centers */
	int     *new_centers_len;	/* [nclusters]: no. of points in each cluster */
    float  **new_centers;		/* [nclusters][nfeatures] */


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

    
    int points_per_gpu=(npoints+num_gpus-1)/num_gpus;
    cl_event *complete_event=(cl_event *)calloc(num_gpus,sizeof(cl_event));

    int *start_point=(int *)calloc(num_gpus,sizeof(int));
    int *end_point=(int *)calloc(num_gpus,sizeof(int));
    int *own_num_points=(int *)calloc(num_gpus,sizeof(int));

/********************************************************************/
/******************** PIM meory variable ****************************/

    void **pim_feature;
    void **pim_feature_swap;
    void **pim_cluster;
    void **pim_membership;


    float **pim_mapped_feature;
    float **pim_mapped_feature_swap;
    float **pim_mapped_cluster;
    int **pim_mapped_membership;

/******************** PIM memory variable end ***********************/
/********************************************************************/

// **** PIM emulation Start Mark  *********
    pim_emu_begin();


	/* allocate memory for membership */
    membership = (int*) malloc(npoints * sizeof(int));

	/* sweep k from min to max_nclusters to find the best number of clusters */
    for(nclusters = min_nclusters; nclusters <= max_nclusters; nclusters++)
    {
        if (nclusters > npoints) break;	/* cannot have more clusters than points */

        // allocate for each PIM
        pim_feature=(void **)malloc(sizeof(void *)*num_gpus);
        pim_feature_swap=(void **)malloc(sizeof(void *)*num_gpus);
        pim_cluster=(void **)malloc(sizeof(void *)*num_gpus);
        pim_membership=(void **)malloc(sizeof(void *)*num_gpus);

        pim_mapped_feature=(float **)malloc(sizeof(float *)*num_gpus);
        pim_mapped_feature_swap=(float **)malloc(sizeof(float *)*num_gpus);
        pim_mapped_cluster=(float **)malloc(sizeof(float *)*num_gpus);
        pim_mapped_membership=(int **)malloc(sizeof(int *)*num_gpus);

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

            start_point[cur_gpu]= cur_gpu*points_per_gpu;
            end_point[cur_gpu]= start_point[cur_gpu]+points_per_gpu;
            if(end_point[cur_gpu]>npoints) end_point[cur_gpu]=npoints;
            own_num_points[cur_gpu] = end_point[cur_gpu]-start_point[cur_gpu];
            printf("GPU %d is calculating %d points from %d to %d\n",cur_gpu,own_num_points[cur_gpu],start_point[cur_gpu],end_point[cur_gpu]);

            /* allocate memory for PIM */
            pim_feature[cur_gpu] = pim_malloc(sizeof(float) * own_num_points[cur_gpu]*nfeatures, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_feature_swap[cur_gpu] = pim_malloc(sizeof(float) * own_num_points[cur_gpu]*nfeatures, target_gpu[cur_gpu], PIM_MEM_PIM_RW, PIM_PLATFORM_OPENCL_GPU);
            pim_cluster[cur_gpu] = pim_malloc(sizeof(float) * nclusters*nfeatures, target_gpu[cur_gpu], PIM_MEM_PIM_READ | PIM_MEM_HOST_WRITE, PIM_PLATFORM_OPENCL_GPU);
            pim_membership[cur_gpu] = pim_malloc(sizeof(int) * own_num_points[cur_gpu], target_gpu[cur_gpu], PIM_MEM_PIM_WRITE | PIM_MEM_HOST_RW, PIM_PLATFORM_OPENCL_GPU);

            /* copy features of points to PIM */
            pim_mapped_feature[cur_gpu] = (float *)pim_map(pim_feature[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

            memcpy(pim_mapped_feature[cur_gpu],features[0]+start_point[cur_gpu]*nfeatures,sizeof(float)*own_num_points[cur_gpu]*nfeatures);

            pim_unmap(pim_mapped_feature[cur_gpu]);

            /* launch the matrix transpose kernel */
            pim_launch_swap_kernel(pim_feature[cur_gpu], pim_feature_swap[cur_gpu], own_num_points[cur_gpu], nfeatures, target_gpu[cur_gpu], &complete_event[cur_gpu]);

        }

        
        
        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
            // wait for PIM gpus to finish
            cl_int clerr;
            clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
            ASSERT_CL_RETURN(clerr);
 
        }
        
        float **clusters;
        int *initial;
        
        for(int cur_loop=0;cur_loop<nloops;cur_loop++){
            clusters    = (float**) malloc(nclusters *             sizeof(float*));
            clusters[0] = (float*)  malloc(nclusters * nfeatures * sizeof(float));
            for (int i=1; i<nclusters; i++) clusters[i] = clusters[i-1] + nfeatures;

	        /* initialize the random clusters */
	        initial = (int *) malloc (npoints * sizeof(int));
	        for (int i = 0; i < npoints; i++)
	        {
		        initial[i] = i;
	        }
	        int initial_points = npoints;
            int n=0;
            int temp;
            /* randomly pick cluster centers */
            for (int i=0; i<nclusters && initial_points >= 0; i++) {
		        //n = (int)rand() % initial_points;		
		
                for (int j=0; j<nfeatures; j++) clusters[i][j] = features[initial[n]][j];	// remapped

		        /* swap the selected index to the end (not really necessary,
		           could just move the end up) */
		        temp = initial[n];
		        initial[n] = initial[initial_points-1];
		        initial[initial_points-1] = temp;
		        initial_points--;
		        n++;
            }

	        /* initialize the membership to -1 for all */
            for (int i=0; i < npoints; i++) membership[i] = -1;

            for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

                /* copy cluster features and membership of the points to PIM */
                pim_mapped_cluster[cur_gpu] = (float *)pim_map(pim_cluster[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
                pim_mapped_membership[cur_gpu] = (int *)pim_map(pim_membership[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

                memcpy(pim_mapped_cluster[cur_gpu],clusters[0],sizeof(float)*nclusters*nfeatures);
                memcpy(pim_mapped_membership[cur_gpu],&membership[start_point[cur_gpu]],sizeof(int)*own_num_points[cur_gpu]);

                pim_unmap(pim_mapped_cluster[cur_gpu]);
                pim_unmap(pim_mapped_membership[cur_gpu]);

            }


            /* allocate space for and initialize new_centers_len and new_centers */
            new_centers_len = (int*) calloc(nclusters, sizeof(int));

            new_centers    = (float**) malloc(nclusters *            sizeof(float*));
            new_centers[0] = (float*)  calloc(nclusters * nfeatures, sizeof(float));

            for (int i=1; i<nclusters; i++) new_centers[i] = new_centers[i-1] + nfeatures;

            int c=0;
            int loop=0;
            float delta;
            do {
                delta = 0.0;
		        
                /* launch kmeans kernel for each PIM */
                for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
                    int size=0;
                    int offset=0;
                    pim_launch_kmeans_kernel(pim_feature_swap[cur_gpu], pim_cluster[cur_gpu], pim_membership[cur_gpu], own_num_points[cur_gpu], nclusters, nfeatures, offset, size, target_gpu[cur_gpu], &complete_event[cur_gpu]);
                }

                for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){  
                    // wait for PIM gpus to finish
                    cl_int clerr;
                    clerr = clWaitForEvents(1, &complete_event[cur_gpu]) ;
                    ASSERT_CL_RETURN(clerr);
                    
                }
                
                for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
                    // copy membership information to host and collect misclassification
                    pim_mapped_membership[cur_gpu] = (int *)pim_map(pim_membership[cur_gpu],PIM_PLATFORM_OPENCL_GPU);
                    
                    int mis_classify = 0;
	                for (int i = 0; i < own_num_points[cur_gpu]; i++)
	                {
		                int cluster_id = pim_mapped_membership[cur_gpu][i];
		                new_centers_len[cluster_id]++;
		                if (pim_mapped_membership[cur_gpu][i] != membership[start_point[cur_gpu]+i])
		                {
			                mis_classify+=1;
			                membership[start_point[cur_gpu]+i] = pim_mapped_membership[cur_gpu][i];
		                }
		                for (int j = 0; j < nfeatures; j++)
		                {
			                new_centers[cluster_id][j] += features[start_point[cur_gpu]+i][j];
		                }
	                }
                    pim_unmap(pim_mapped_membership[cur_gpu]);
                    delta+=(float) mis_classify;
                }

                
		        /* replace old cluster centers with new_centers */
		        /* CPU side of reduction */
		        for (int i=0; i<nclusters; i++) {
			        for (int j=0; j<nfeatures; j++) {
				        if (new_centers_len[i] > 0)
					        clusters[i][j] = new_centers[i][j] / new_centers_len[i];	/* take average i.e. sum/n */
				        new_centers[i][j] = 0.0;	/* set back to 0 */
			        }
			        new_centers_len[i] = 0;			/* set back to 0 */
		        }	 

                for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){

                    /* update cluster features in PIM */
                    pim_mapped_cluster[cur_gpu] = (float *)pim_map(pim_cluster[cur_gpu],PIM_PLATFORM_OPENCL_GPU);

                    memcpy(pim_mapped_cluster[cur_gpu],clusters[0],sizeof(float)*nclusters*nfeatures);

                    pim_unmap(pim_mapped_cluster[cur_gpu]);

                }
		        c++;
                printf("loop %d error %f\n",c,delta);
            } while ((delta > threshold) && (loop++ < 500));	/* makes sure loop terminates */
	        printf("iterated %d times\n", c);
            
            if (*cluster_centres) {
				free((*cluster_centres)[0]);
				free(*cluster_centres);
			}
            *cluster_centres = clusters;
            
            if(isRMSE)
			{
				rmse = rms_err(features,
							   nfeatures,
							   npoints,
							   clusters,
							   nclusters);
				
				if(rmse < min_rmse_ref){
					min_rmse_ref = rmse;			//update reference min RMSE
					*min_rmse = min_rmse_ref;		//update return min RMSE
					*best_nclusters = nclusters;	//update optimum number of clusters
					index = cur_loop;						//update number of iteration to reach best RMSE
				}
			} 

            

            free(new_centers_len);
            free(new_centers[0]);
            free(new_centers);           
            free(initial);
        }

        for(int cur_gpu=0;cur_gpu<num_gpus;cur_gpu++){
            pim_free(pim_feature[cur_gpu]);
            pim_free(pim_feature_swap[cur_gpu]);
            pim_free(pim_cluster[cur_gpu]);
            pim_free(pim_membership[cur_gpu]);
        }

        free(pim_feature);
        free(pim_feature_swap);
        free(pim_cluster);
        free(pim_membership);

        free(pim_mapped_feature);
        free(pim_mapped_feature_swap);
        free(pim_mapped_cluster);
        free(pim_mapped_membership);

            	
	}

    free(membership);

    // **** PIM emulation End Mark  *********  
    pim_emu_end();

    free(target_gpu);    
    free(list_of_pims);
    free(gpus_per_pim);
    free(cpus_per_pim);


    return index;
}
