
#include "spk.h"

#include "read_input.c"
#include "malloc.c"
#include "utility.c"
#include "cpu_func.c"
#include "opencl_init.c"
#include "spk_gpu.c"
#include "spk_pim.c"


int main(int argc, char *argv[])
{
    cout.setf(ios::fixed,ios::floatfield);
    cout.precision(3);
    read_input_file(argc,argv);
    
    double total_start,total_end;
    double t_start1,t_end1;
    
    total_start=rtclock();
    
    t_start1=rtclock();
    for(int i =0;i<num_graph;i++) read_graph(i);
    for(int i =0;i<num_graph;i++) init_adj_list(i);

    pack_adj_list();
    t_end1=rtclock();
    read_graph_time+=t_end1-t_start1;
	
	t_start1=rtclock();
    
    for(int i =0;i<num_graph;i++) convert_to_sp(i);
    t_end1=rtclock();
	convert_kernel_time+=t_end1-t_start1;

	t_start1=rtclock();
    for(int i =0;i<num_graph;i++) count_sp_edge(i);
    for(int i =0;i<num_graph;i++) init_edge(i);
    pack_sp_edge();
    t_end1=rtclock();
	edge_init_time+=t_end1-t_start1;	
    
    init_graph_info();

    cout<<"min_node max_node avg_node min_edge max_edge avg_edge min_sp max_sp avg_sp:"<<endl;
    cout<<min_node<<" "<<max_node<<" "<<total_node/num_graph<<" "<<min_edge<<" "<<max_edge<<" "<<total_edge/num_graph<<" "<<min_sp<<" "<<max_sp<<" "<<total_sp/num_graph<<endl;
    
if(option ==0){
    OpenCL_init();

    SPGK_GPU();
    cout<<"GPU total time: "<<gpu3_total_time<<endl;
    cout<<"all kernel time: "<<vertex_kernel_time+edge_kernel_time+reduce_time<<endl;
    cout<<"mem copy   time: "<<mem_init_time<<endl;
    cout<<"reduc copy time: "<<gpu_sum_mem_cpy_time<<endl;
    cout<<"reduc sum  time: "<<gpu_sum_time<<endl;
    cout<<"vertex     time: "<<vertex_kernel_time<<endl;
    cout<<"edge       time: "<<edge_kernel_time<<endl;
    cout<<"reduce     time: "<<reduce_time<<endl;

    cl_clean_up();
    
    
    
    clean_up();
}
else if(option ==1){
    SPGK_PIM();
}
else if(option ==2){
    SPGK_mult_PIM();
}


    total_end=rtclock();

    printf("convert kernel time  : %.2lf\n",convert_kernel_time);
    printf("read    graph  time  : %.2lf\n",read_graph_time);
    printf("edge    init   time  : %.2lf\n",edge_init_time);

    printf("Total Runtime: %.2lf\n",(total_end - total_start));
    	
	print2d(K_Matrix,num_graph,num_graph);
	//NormalizeKMatrix();

    output(argc, argv);

    return 0;
}
