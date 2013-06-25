void output(int argc, char *argv[])
{
    char output[1024];
    string temp=input_dirc;
    if(*temp.rbegin() != '/') temp+="/";
    strcpy(output,temp.c_str());
    
    if(vert_option==0) strcat(output,"GAUSS");
    if(vert_option==1) strcat(output,"INTERSECT");
    strcat(output,"-");
    strcat(output,argv[6]);
    strcat(output,"-");
    if(gk_opt==0) strcat(output,"SP.kernel");
    if(gk_opt==1) strcat(output,"RW.kernel");
    if(gk_opt==2) strcat(output,"UNORD.kernel");

    FILE *fp;
    fp=fopen(output,"w");
	if(fp ==0)
    {
        printf("error in opening output file %s\n",output);
        exit(EXIT_FAILURE);
    }
    
    double **k_temp=malloc2ddouble(num_graph,num_graph);
    for(int i=0;i<num_graph;i++){
        for(int j=0;j<num_graph;j++){
            k_temp[i][j] = K_Matrix[i][j]/sqrtf(K_Matrix[i][i]*K_Matrix[j][j]);
            fprintf(fp,"%lf ",k_temp[i][j]);
        }
        fprintf(fp,"\n");
    }
    free2d(k_temp);

    fclose(fp);

    char output_label[1024];
    char output_fname[1024];

    strcpy(output_label,output);
    strcpy(output_fname,output);
    
    strcat(output_label,".labels");
    strcat(output_fname,".fnames");

    fp=fopen(output_label,"w");
	if(fp ==0)
    {
        printf("error in opening output label file %s\n",output_label);
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<num_graph;i++){
        fprintf(fp,"%s\n",graph[i].label);
    }
    fclose(fp);

    fp=fopen(output_fname,"w");
	if(fp ==0)
    {
        printf("error in opening output fname file %s\n",output_fname);
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<num_graph;i++){
        fprintf(fp,"%s\n",graph[i].graph_fname);
    }
    fclose(fp);


}

void convert_to_sp(int n)
{
    double val;
	int n_nodes = graph[n].n_node;
	// copy the values

	for (int i = 0 ; i < n_nodes ; i ++){
		for (int j = 0 ; j < n_nodes ; j ++) {
			if (i == j)
				val = 0;
			else {
				if (graph[n].adj[i][j] != 0)
					val = graph[n].adj[i][j];
				else
					val = INFINITY;
			}
			graph[n].sp_adj[i][j] = val;
		}
	}
	// apply floyd warshall
	for (int k = 0 ; k < n_nodes ; k ++){
		for (int i = 0 ; i < n_nodes ; i ++){
			for (int j = 0 ; j < n_nodes ; j ++){
				graph[n].sp_adj[i][j] = fmin(graph[n].sp_adj[i][j], graph[n].sp_adj[i][k]+graph[n].sp_adj[k][j]);
				}}}
	

    //print2d(graph[n].sp_adj,n_nodes,n_nodes);
    return;
}

void count_sp_edge(int k)
{
    int count=0;
    int n = graph[k].n_node;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(graph[k].sp_adj[i][j]!= infinity && i !=j ) count++;
        }
    }
    graph[k].n_sp_edge=count;
    if(k==0){
        max_sp = count;
        min_sp = count;
    }    
    if(count>max_sp) max_sp=count;
    if(count<min_sp) min_sp=count;
    
    total_sp+=count;

    //printf("Number of edge for G%d is %d\n",k,graph[k].n_sp_edge);
    return;
}

void pack_sp_edge()
{
    sp_edge_offset=(int *)calloc(num_graph,sizeof(int));
    graph_sp_edge_num=(int *)calloc(num_graph,sizeof(int));
    edge_offset=(int *)calloc(num_graph,sizeof(int));
    graph_edge_num=(int *)calloc(num_graph,sizeof(int));
    
    int cur_sp_edge_offset=0;
    int cur_edge_offset=0;
    for(int i=0;i<num_graph;i++){
        sp_edge_offset[i]=cur_sp_edge_offset;
        graph_sp_edge_num[i]=graph[i].n_sp_edge;
        cur_sp_edge_offset += graph[i].n_sp_edge;

        edge_offset[i]=cur_edge_offset;
        graph_edge_num[i]=graph[i].n_edge;
        cur_edge_offset += graph[i].n_edge;
    }

    //print1d(edge_offset,num_graph);
    //print1d(graph_edge_num,num_graph);


    sp_edge_w_all = (double *)calloc(total_sp,sizeof(double));
    sp_edge_x_all = (int *)calloc(total_sp,sizeof(int));
    sp_edge_y_all = (int *)calloc(total_sp,sizeof(int));

    edge_w_all = (double *)calloc(total_edge,sizeof(double));
    edge_x_all = (int *)calloc(total_edge,sizeof(int));
    edge_y_all = (int *)calloc(total_edge,sizeof(int));

    int sp_count=0;
    int edge_count=0;
    for(int i =0;i < num_graph;i++){
        for(int j=0;j<graph[i].n_sp_edge;j++){
            sp_edge_w_all[sp_count]=graph[i].sp_edge_w[j];
            sp_edge_x_all[sp_count]=graph[i].sp_edge_x[j];
            sp_edge_y_all[sp_count]=graph[i].sp_edge_y[j];
            sp_count++;
        }

        for(int j=0;j<graph[i].n_edge;j++){
            edge_w_all[edge_count]=graph[i].edge_w[j];
            edge_x_all[edge_count]=graph[i].edge_x[j];
            edge_y_all[edge_count]=graph[i].edge_y[j];
            edge_count++;
        }
        //memcpy(sp_edge_w_all + sp_edge_offset[i], graph[i].sp_edge_w,sizeof(double)*graph[i].n_sp_edge);
        //memcpy(sp_edge_x_all + sp_edge_offset[i], graph[i].sp_edge_x,sizeof(int)*graph[i].n_sp_edge);
        //memcpy(sp_edge_y_all + sp_edge_offset[i], graph[i].sp_edge_y,sizeof(int)*graph[i].n_sp_edge);        
    }

    //print1d(sp_edge_w_all,total_sp);
    //print1d(sp_edge_x_all,total_sp);
    //print1d(sp_edge_y_all,total_sp);

    //print1d(edge_w_all,total_edge);
    //print1d(edge_x_all,total_edge);
    //print1d(edge_y_all,total_edge);

}

void pack_adj_list()
{

    
    for(int i=0;i<num_graph;i++) total_neighbor+=graph[i].t_neighbor;

    adj_list_all=(int *)calloc(total_neighbor,sizeof(int));
    adj_list_all_offset=(int *)calloc(num_graph,sizeof(int));
    n_neighbor_all_offset=(int *)calloc(num_graph,sizeof(int));
    n_neighbor_all=(int *)calloc(total_node,sizeof(int));
    adj_list_offset_all=(int *)calloc(total_node,sizeof(int));
    
    int count=0;
    int node_count=0;
    for(int i=0;i<num_graph;i++){
        adj_list_all_offset[i]=count;
        for(int j=0;j<graph[i].t_neighbor;j++){
            adj_list_all[count]=graph[i].adj_list[j];
            count+=1;
        }
        n_neighbor_all_offset[i]=node_count;
        for(int k=0;k<graph[i].n_node;k++){
            adj_list_offset_all[node_count]=graph[i].adj_list_offset[k];
            n_neighbor_all[node_count]=graph[i].n_neighbor[k];
            node_count+=1;
        }
    }

    //print1d(adj_list_all,total_neighbor);
    //print1d(adj_list_all_offset,num_graph);
    //print1d(n_neighbor_all_offset,num_graph);
    //print1d(n_neighbor_all,total_node);
    //print1d(adj_list_offset_all,total_node);

}

void init_edge(int k)
{
    int num_edge = graph[k].n_sp_edge;
    int n = graph[k].n_node;
    graph[k].sp_edge_w = (double *)calloc(num_edge,sizeof(double));
    graph[k].sp_edge_x = (int *)calloc(num_edge,sizeof(int));
    graph[k].sp_edge_y = (int *)calloc(num_edge,sizeof(int));

    graph[k].edge_w = (double *)calloc(graph[k].n_edge,sizeof(double));
    graph[k].edge_x = (int *)calloc(graph[k].n_edge,sizeof(int));
    graph[k].edge_y = (int *)calloc(graph[k].n_edge,sizeof(int));
    
    int count=0;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(graph[k].sp_adj[i][j]!= infinity && i !=j ){
                graph[k].sp_edge_w[count]=graph[k].sp_adj[i][j];
                graph[k].sp_edge_x[count]=i;
                graph[k].sp_edge_y[count]=j;
                count++;
            }
        }
    }

    count=0;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(graph[k].adj[i][j]!= 0 ){
                graph[k].edge_w[count]=graph[k].adj[i][j];
                graph[k].edge_x[count]=i;
                graph[k].edge_y[count]=j;
                count++;
            }
        }
    }
    
    //print1d(graph[k].sp_edge_x,num_edge);
    //print1d(graph[k].sp_edge_y,num_edge);
    //print1d(graph[k].sp_edge_w,num_edge);   
    
    return;
}

double gaussian_k(int g1, int g2, int idx1, int idx2) 
{
	double diff;
	double vk = 0;
	for (int i = 0 ; i < n_feat ; i ++) {
		diff = graph[g1].feat[idx1][i] - graph[g2].feat[idx2][i];
		vk += (diff * diff);
	}
    double result=expf(-vk/vk_params[1]);
    //printf("%.1f %.1f %.1f %.1f %f %f\n",graph[g1].feat[idx1][0],graph[g1].feat[idx1][1],graph[g2].feat[idx2][0],graph[g2].feat[idx2][1],vk,result);
    return result;
    
}



void clean_up()
{
    free(feat_all);
    free(adj_all);
    free(sp_adj_all);
    free(graph_node);
    free(graph1_num);
    free(graph2_num);
    free(adj_offset);
    free(feat_offset);
    free(sp_edge_offset);
    free(graph_sp_edge_num);
    free(edge_offset);
    free(graph_edge_num);

    free(sp_edge_w_all);
    free(sp_edge_x_all);
    free(sp_edge_y_all);

    free(edge_w_all);
    free(edge_x_all);
    free(edge_y_all);

}



