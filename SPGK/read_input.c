void read_input_file(int argc, char *argv[])
{
    if(argc<11){
        cout<<"Graph Kernel usage:"<<endl;
        cout<<"./spk.exe -directory [directory] -platform [platform] -gk [graph_kernel] -option [implement_option] -vertext [vertex_kernel] -gauss [gaussian_param]"<<endl;
        cout<<"Example to run the code:"<<endl;
        cout<<"./spk.exe -directory ~/Data/input_directory -platform 1 -gk 0 -option 8 -vertex 0 -gauss 0.1"<<endl;
        cout<<"-directory [directory]           specify the input files directory"<<endl;
        cout<<"-platform [platform]             specify the platform"<<endl;
        cout<<"                                 0: NVIDIA"<<endl;
        cout<<"                                 1: AMD"<<endl;
        cout<<"                                 2: Intecl"<<endl;
        cout<<"-gk [graph_kernel]               specify the graph kernels"<<endl;
        cout<<"                                 0: Shortest Path"<<endl;
        cout<<"                                 1: Random Walk"<<endl;
        cout<<"                                 2: UNORD"<<endl;
        cout<<"                                 3: graphlet"<<endl;
        cout<<"-optiont [implement_option]      specify different implementaions"<<endl;
        cout<<"-vertext [vertex_kernel]         specify different vertext kernel"<<endl;
        cout<<"                                 0: Gaussian"<<endl;
        cout<<"                                 1: Intersection"<<endl;
        cout<<"-gauss [gaussian_param]          specify the input parameter for Gaussian kernel"<<endl;
        exit(-1);
    }

    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i],(char *)"-directory"))
        {
            i+=1;
            strcpy(input_dirc,argv[i]);
        }
        else if(!strcmp(argv[i],(char *)"-platform"))
        {
            i+=1;
            platform_opt = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],(char *)"-gk"))
        {
            i+=1;
            gk_opt = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],(char *)"-option"))
        {
            i+=1;
            option = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],(char *)"-vertex"))
        {
            i+=1;
            vert_option = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],(char *)"-gauss"))
        {
            i+=1;
            vk_params[1] = atof(argv[i]);
        }
        else if(!strcmp(argv[i],(char *)"-threshold"))
        {
            i+=1;
            threshold = atoi(argv[i]);
        }
	}

    printf("Arguments:\n");
	for(int i = 1; i < argc; i++)
	{
		printf(argv[i]);
		printf("\n");
	}
/*
    if(argc!=7 && argc !=8){
        cout<<"./spk.exe <input_directory> <platform> <kernel> <implement option> <vertex kernel option> <vertex kernel parameter> [threshold]"<<endl;
        cout<<"platform 0: NVIDIA GPU"<<endl;
        cout<<"platform 1: AMD GPU"<<endl;
        cout<<"platform 2: Intel CPU"<<endl;
        cout<<"kernel 0: Shortest Path Graph Kernel"<<endl;
        cout<<"option 0: CPU"<<endl;
        cout<<"option 1: OpenMP"<<endl;
        cout<<"option 2: GPU1"<<endl;
        cout<<"option 3: GPU2"<<endl;
        cout<<"option 4: GPU3"<<endl;
        cout<<"option 5: Hybrid, need 5th argument threshold, the input has to be sorted for this option"<<endl;
        cout<<"kernel 1: Random Walk Kernel"<<endl;
        cout<<"option 0: CPU"<<endl;
        cout<<"option 1: OpenMP"<<endl;
        cout<<"option 2: GPU1"<<endl;
        cout<<"option 3: GPU2"<<endl;
        cout<<"option 4: GPU3"<<endl;
        cout<<"option 5: GPU4"<<endl;
        cout<<"kernel 2: Simple Kernel"<<endl;
        cout<<"option 0: CPU"<<endl;
        cout<<"option 2: GPU1"<<endl;
        cout<<"kernel 3: 3 graphlet Kernel"<<endl;
        cout<<"option 0: CPU"<<endl;
        cout<<"option 2: GPU1"<<endl;

        exit(EXIT_FAILURE);
    }
    strcpy(input_dirc,argv[1]);
    platform_opt = atoi(argv[2]);
    gk_opt = atoi(argv[3]);
    option = atoi(argv[4]);
    vert_option = atoi(argv[5]);
    vk_params[1] = atof(argv[6]);
    if(gk_opt == 0 && option ==5) threshold = atoi(argv[7]);
*/    

    struct dirent *dp;
	DIR *dirp = opendir(input_dirc);
    int count=0;
    while ((dp = readdir(dirp)) != NULL) {
		string fname(dp->d_name);
		//if (fname.length() > 6 && ! fname.compare(fname.length()-6,6,".graph")) {
		if (fname.length() > 4 && ! fname.compare(fname.length()-4,4,".txt")) {
			count++;
        }
	}
	closedir(dirp);

    num_graph=count;
    cout<<"Number of graphs is "<<num_graph<<endl;
    graph = (Graph *)malloc(sizeof(Graph)*num_graph);
    

    dirp = opendir(input_dirc);
    count=0;
    while ((dp = readdir(dirp)) != NULL) {
		string fname(dp->d_name);
        //if (fname.length() > 6 && ! fname.compare(fname.length()-6,6,".graph")) {
        if (fname.length() > 4 && ! fname.compare(fname.length()-4,4,".txt")) {
			string temp=input_dirc;
            if(*temp.rbegin() != '/') temp+="/";
            temp+=fname;
            strcpy(graph[count].graph_fname,temp.c_str());
            string label = temp.substr(temp.find_last_of("/") + 1);
	        //label = label.substr(0, label.find_last_of("_"));
	        label = label.substr(0, label.find_first_of("_"));
            strcpy(graph[count].label, label.c_str());
            //cout<<graph[count].label<<" "<<graph[count].graph_fname<<endl;
            count++;
		}
	}
	closedir(dirp);

    K_Matrix=malloc2ddouble(num_graph,num_graph);
    num_comparison = num_graph*(num_graph+1)/2;
    //printf("Total number of graph is %d\n",num_graph);
    //printf("Length of gaussian  feature vector is %d\n",n_feat);


}

void read_graph(int i)
{
    FILE *fp;
    //char temp[2048];
	//sprintf(temp,"%d",i+1);
	//strcpy(input_file,input_dirc);	
	//strcat(input_file,temp);


	int n_node =0;
	int n_edge =0;
	    
	//fp=fopen(input_file,"r");
    fp=fopen(graph[i].graph_fname,"r");
	if(fp ==0)
    {
        printf("error in opening input file %s\n",graph[i].graph_fname);
        exit(EXIT_FAILURE);
    }
    
    //skipnlines(fp,1);
    fscanf(fp,"%d %d",&n_node,&n_feat);
    //fscanf(fp,"%d\n",&n_node);

    graph[i].n_node = n_node;
    if(i==0){
        max_node = n_node;
        min_node = n_node;
    }
    if(n_node>max_node) max_node=n_node;
    if(n_node<min_node) min_node=n_node;
    total_node+=n_node;
    //printf("Number of node for G%d is %d\n",i,n_node);
    
    graph[i].adj =malloc2ddouble(n_node,n_node);
    graph[i].sp_adj =malloc2ddouble(n_node,n_node);
    graph[i].feat=malloc2ddouble(n_node,n_feat);
    
    for(int j=0;j<n_node;j++)
    {
        for(int k=0;k<n_feat;k++){
            fscanf(fp,"%lf ",&graph[i].feat[j][k]);
        }
    }
    for(int j=0;j<n_node;j++){
        for(int k=0;k<n_node;k++){
            fscanf(fp,"%lf ",&graph[i].adj[j][k]);
            if(graph[i].adj[j][k]!=0) n_edge+=1;
        }
    }

    if(i==0){
        max_edge = n_edge;
        min_edge = n_edge;
    }    
    if(n_edge>max_edge) max_edge=n_edge;
    if(n_edge<min_edge) min_edge=n_edge;
    graph[i].n_edge = n_edge;
    total_edge+=n_edge;
    //if(i==1 ) printf("%s\n",graph[i].graph_fname);
    //if(i==1 ) print2d(graph[i].feat,n_node,n_feat);
    //if(i==1 ) print2d(graph[i].adj,n_node,n_node);
    fclose(fp);


    //output the input file into seperate feat file and adj file
    /*char output[2048];
    strcpy(output,input_dirc);
    strcat(output,"sep/");
    strcat(output,temp);

    char out_feat[2048];
    char out_adj[2048];

    strcpy(out_feat,output);
    strcpy(out_adj,output);
    strcat(out_feat,".feat");
    strcat(out_adj,".adj");
    
    fp=fopen(out_feat,"w");
	if(fp ==0)
    {
        printf("error in opening output feat file %s\n",out_feat);
        exit(EXIT_FAILURE);
    }

    for(int j=0;j<n_node;j++)
    {
        for(int k=0;k<n_feat;k++){
            fprintf(fp,"%.3f ",graph[i].feat[j][k]);
        }
        fprintf(fp,"\n");
    }

    fclose(fp);

    fp=fopen(out_adj,"w");
	if(fp ==0)
    {
        printf("error in opening output feat file %s\n",out_adj);
        exit(EXIT_FAILURE);
    }
    for(int j=0;j<n_node;j++){
        for(int k=0;k<n_node;k++){
            fprintf(fp,"%.3f ",graph[i].adj[j][k]);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);*/
        
	return;
}

void init_adj_list(int g)
{
    int t_neighbor=0;
    int n_node = graph[g].n_node;
    graph[g].n_neighbor = (int *)calloc(n_node,sizeof(int));
    graph[g].adj_list_offset=(int *)calloc(n_node,sizeof(int));
    for(int i=0;i<n_node;i++){
        int neighbors=0;
        graph[g].adj_list_offset[i]=t_neighbor;
        for(int j=0;j<n_node;j++){
            if(i!=j && graph[g].adj[i][j]!=0) neighbors+=1;
        }
        t_neighbor+=neighbors;
        graph[g].n_neighbor[i]=neighbors;
    }
    graph[g].adj_list=(int *)calloc(t_neighbor,sizeof(int));
    graph[g].t_neighbor=t_neighbor;
    int count=0;
    for(int i=0;i<n_node;i++){
        for(int j=0;j<n_node;j++){
            if(i!=j && graph[g].adj[i][j]!=0){
                graph[g].adj_list[count]=j;
                count+=1;
            }
        }
    }
    //print1dto2d(graph[g].adj[0],n_node,n_node);
    //print1d(graph[g].n_neighbor,n_node);
    //print1d(graph[g].adj_list,t_neighbor);
    //print1d(graph[g].adj_list_offset,n_node);
}
        


void init_graph_info()
{
    /*initializing graph_num1 and graph_num2
    **these two arrays store which two graphs to compare for each GPU thread
    */
    graph1_num=(int *)calloc(num_comparison,sizeof(int));
    graph2_num=(int *)calloc(num_comparison,sizeof(int));
    for(int id=0;id<num_comparison;id++){
        int G1,G2;
        int id_temp;
        id_temp=id;
        for(G1=0;G1<num_graph;G1++){
            if(id_temp-(num_graph-G1)<0) break;
            else id_temp-=num_graph-G1;
        }
        G2=id+G1-(num_graph+1+num_graph-G1)*G1/2;
        graph1_num[id]=G1;
        graph2_num[id]=G2;
    }
    //print1d(graph1_num,num_comparison);
    //print1d(graph2_num,num_comparison);
    
    
    /*initliazing graph_node to store number of nodes for each graph
    **and calculate the totally element in all feature vectors and adj matrix
    */
    feat_all_length = 0;
    adj_all_length = 0;    
    graph_node=(int *)calloc(num_graph,sizeof(int));
    for(int i=0;i<num_graph;i++){
        graph_node[i]=graph[i].n_node;
        feat_all_length += graph_node[i]*n_feat;
        adj_all_length += graph_node[i]*graph_node[i];
    }
    
    
    /*initializing offset arrays, so GPU can know where to start to fetch memory
    **
    */
    adj_offset=(int *)calloc(num_graph,sizeof(int));
    feat_offset=(int *)calloc(num_graph,sizeof(int));
    //sp_edge_offset=(int *)calloc(num_graph,sizeof(int));
    //graph_sp_edge_num=(int *)calloc(num_graph,sizeof(int));
    
    int cur_adj_offset=0;
    int cur_feat_offset=0;
    //int cur_sp_edge_offset=0;
    for(int i=0;i<num_graph;i++){
        adj_offset[i] = cur_adj_offset;
        feat_offset[i] = cur_feat_offset;
        //sp_edge_offset[i]=cur_sp_edge_offset;
        //graph_sp_edge_num[i]=graph[i].n_sp_edge;
        cur_adj_offset += graph_node[i]*graph_node[i];
        cur_feat_offset += graph_node[i]*n_feat;
        //cur_sp_edge_offset += graph[i].n_sp_edge;
    }
    
    //print1d(graph_node,num_graph);
    //print1d(adj_offset,num_graph);
    //print1d(feat_offset,num_graph);
    
    
    
    feat_all = (double *)calloc(feat_all_length,sizeof(double));
    sp_adj_all  = (double *)calloc(adj_all_length,sizeof(double));
    adj_all  = (double *)calloc(adj_all_length,sizeof(double));
    /*put all feature vectors into a big 1D array
    **put all adj matrix into a big 1D array
    */
    int feat_count=0;
    int adj_count=0;
    for(int i =0;i < num_graph;i++){
        for(int j=0;j< graph[i].n_node;j++){
            for(int k=0;k<n_feat;k++){
                feat_all[feat_count]=graph[i].feat[j][k];
                feat_count++;
            }
        }
        for(int j=0;j< graph[i].n_node;j++){
            for(int k=0;k<graph[i].n_node;k++){
                adj_all[adj_count]=graph[i].adj[j][k];
                sp_adj_all[adj_count]=graph[i].sp_adj[j][k];
                adj_count++;
            }
        }
        //memcpy(feat_all + feat_offset[i], graph[i].feat[0],sizeof(double)*graph_node[i]*n_feat);
        //memcpy(sp_adj_all + adj_offset[i], graph[i].sp_adj[0],sizeof(double)*graph_node[i]*graph_node[i]);
    }
    //print1d(feat_all,feat_all_length);
    //print1d(sp_adj_all,adj_all_length);
    
    
    
    return;
}

void init_graph_info_gpu4()
{

    num_comparison = GPU1_graph*(GPU1_graph+1)/2;
    /*initializing graph_num1 and graph_num2
    **these two arrays store which two graphs to compare for each GPU thread
    */
    graph1_num=(int *)calloc(num_comparison,sizeof(int));
    graph2_num=(int *)calloc(num_comparison,sizeof(int));
    for(int id=0;id<num_comparison;id++){
        int G1,G2;
        int id_temp;
        id_temp=id;
        for(G1=0;G1<GPU1_graph;G1++){
            if(id_temp-(GPU1_graph-G1)<0) break;
            else id_temp-=GPU1_graph-G1;
        }
        G2=id+G1-(GPU1_graph+1+GPU1_graph-G1)*G1/2;
        graph1_num[id]=G1;
        graph2_num[id]=G2;
    }
    //print1dint(graph1_num,num_comparison);
    //print1dint(graph2_num,num_comparison);
    
    
    /*initliazing graph_node to store number of nodes for each graph
    **and calculate the totally element in all feature vectors and adj matrix
    */
    feat_all_length = 0;
    adj_all_length = 0;    
    graph_node=(int *)calloc(GPU1_graph,sizeof(int));
    for(int i=0;i<GPU1_graph;i++){
        graph_node[i]=graph[i].n_node;
        feat_all_length += graph_node[i]*n_feat;
        adj_all_length += graph_node[i]*graph_node[i];
    }
    
    
    /*initializing offset arrays, so GPU can know where to start to fetch memory
    **
    */
    adj_offset=(int *)calloc(GPU1_graph,sizeof(int));
    feat_offset=(int *)calloc(GPU1_graph,sizeof(int));
    
    int cur_adj_offset=0;
    int cur_feat_offset=0;
    for(int i=0;i<GPU1_graph;i++){
        adj_offset[i] = cur_adj_offset;
        feat_offset[i] = cur_feat_offset;
        cur_adj_offset += graph_node[i]*graph_node[i];
        cur_feat_offset += graph_node[i]*n_feat;
    }
    
    //print1dint(graph_node,GPU1_graph);
    //print1dint(adj_offset,GPU1_graph);
    //print1dint(feat_offset,GPU1_graph);
    
    
    
    feat_all = (double *)calloc(feat_all_length,sizeof(double));
    sp_adj_all  = (double *)calloc(adj_all_length,sizeof(double));
    adj_all  = (double *)calloc(adj_all_length,sizeof(double));
    
    
    return;
}

