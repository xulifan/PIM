
#include "spk.h"
#include "read_input.c"
#include "malloc.c"
#include "utility.c"
#include "cpu_func.c"
#include "opencl_init.c"
#include "spk_gpu.c"
#include "spk_one_pim_per_pair.c"
#include "spk_multi_pim_per_pair_1.c"
#include "spk_multi_pim_per_pair_2.c"
#include "spk_multi_pim_per_pair_3.c"

int main(int argc, char *argv[])
{
    double total_start,total_end;
    total_start=rtclock();

    cout.setf(ios::fixed,ios::floatfield);
    cout.precision(3);

    read_input_file(argc,argv);
    
    input_process();
    
    if(option ==0){

        OpenCL_init();
        // GPU implemetation of SPGK
        SPGK_GPU();
        cl_clean_up();
    }
    else if(option ==1){

        printf(" use only one PIM and only process one pair of graphs \n");
        SPGK_PIM();
    }
    else if(option ==2){
        printf(" use multiple PIMs in parallel, each PIM can process one pair of graphs\n");
        SPGK_mult_PIM();
    }
    else if(option ==3){
        printf(" use multiple PIMs on one pair of graphs (scheme 1) \n");
        SPGK_mult_PIM_one_pair_1();
    }
    else if(option ==4){
        printf(" use multiple PIMs on one pair of graphs (scheme 2) \n");
        SPGK_mult_PIM_one_pair_2();
    }
    else if(option ==5){
        printf(" use multiple PIMS on one pair of graphs (scheme 3) \n");
        SPGK_mult_PIM_one_pair_3();
    }

    total_end=rtclock();

    printf("convert kernel time  : %.2lf\n",convert_kernel_time);
    printf("read    graph  time  : %.2lf\n",read_graph_time);
    printf("edge    init   time  : %.2lf\n",edge_init_time);

    printf("Total Runtime: %.2lf\n",(total_end - total_start));
    	
	print2d(K_Matrix,num_graph,num_graph);
	//NormalizeKMatrix();

    output(argc, argv);

    clean_up();

    return 0;
}
