#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <CL/cl.h>
#include <sstream>
#include <dirent.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <pim.h>
#include "pim_utility.c"

#define infinity INFINITY

#define BLOCK_SIZE_1D 128
#define BLOCK_SIZE_2D 16
#define LAMBDA 1.000
#define DEVICE_QUERY 0
using namespace std;

int n_feat;
int option;
int gk_opt;
int platform_opt;
int vert_option;
int threshold;
int iso_graph;
int GPU1_graph;
int GPU3_graph;

int max_node=0;
int max_edge=0;
int max_sp=0;
int min_node=0;
int min_edge=0;
int min_sp=0;
int avg_node=0;
int avg_edge=0;
int avg_sp=0;
int total_node=0;
int total_edge=0;
int total_sp=0;

//double vk_params[4]={2.0f,2*0.1*0.1,0,1};
double vk_params[4]={3.0f,15,0,1};

double kernel_time =0;
double sp_kernel_time =0;
double vertex_kernel_time =0;
double convert_kernel_time =0;
double edge_kernel_time =0;
double matrix_calc_time =0;
double mem_cpy_time =0;
double reduce_time =0;
double read_graph_time =0;
double edge_init_time =0;
double gpu_time =0;
double mem_init_time =0;
double gpu_inner_time =0;
double cpu_kernel_time =0;
double openmp_max_time =0;
double gpu1_kernel_time =0;
double gpu2_kernel_time =0;
double gpu1_total_time =0;
double gpu2_total_time =0;
double gpu3_total_time =0;
double gpu1d_total_time =0;
double gpu2d_total_time =0;
double gpu4_total_time =0;

double RW_cpu_kernel_time =0;
double RW1_total_time=0;
double agg_time=0;
double kron_time=0;
double matmul_time=0;
double gpu_sum_mem_cpy_time=0;
double gpu_sum_time=0;
double mem_init_kernel_time=0;
double mat_padding_time=0;
double vect_add_time=0;
double kron_vert_time=0;
double vect_mul_time=0;

double unord_kernel_time=0; 
double unord_gpu_total_time=0;
double mat_transpose_time=0;

double **K_Matrix;

double *edge_g1;
double *edge_g2;
int *sp_edge_x1;
int *sp_edge_x2;
int *sp_edge_y1;
int *sp_edge_y2;

int n_node1;
int n_node2;
int n_edge1;
int n_edge2;
int n_nodek;

int feat_all_length;
int adj_all_length;

int *graph1_num; //length of num_comparison, store which graph to compare
int *graph2_num; //length of num_comparison, store which graph to compare
int *graph_node; //length of num_graph, store number of nodes for each graph;
int *graph_sp_edge_num; //length of num_graph, store number of shorest path for each graph;
int *graph_edge_num;
int *adj_offset; //length of num_graph, store offset for graphs in adjacent matrix
int *feat_offset;//length of num_graph, store offset for graphs in feature vectors
int *sp_edge_offset; //length of num_graph, store offset for cl_sp_edge_w,cl_sp_edge_x, and cl_sp_edge_y
int *edge_offset;
double *feat_all;//store feature vectors of all graphs 
double *adj_all; //store adj matrix of all graphs
double *sp_adj_all; //store sp adj matrix of all graphs
double *sp_edge_w_all; //pack all shortest paths w from all graph into one big array
int *sp_edge_x_all;    //pack all shortest paths x from all graph into one big array
int *sp_edge_y_all;    //pack all shortest paths y from all graph into one big array
double *edge_w_all;
int *edge_x_all;
int *edge_y_all;

int *adj_list_all;   //store all the adj list for each graph
int *adj_list_offset_all; //store the adj list offset for each node
int *adj_list_all_offset; //store the starting position for each graph in adj_list_all
int *n_neighbor_all;  //store the number of neighbors for each node in each graph
int *n_neighbor_all_offset; //store the starting position for each graph in n_neighbor_all;

int total_neighbor=0;


int num_graph;
int num_comparison;
char input_dirc[1024];
char input_file[1024];


//OpenCL variables
#define MAX_SOURCE_SIZE (0x100000)
char str_temp[1024];
char driver_version[1024];
char device_version[1024];
char device_extension[1024];
char *source_str;
size_t source_size;
size_t printf_buffer_size;
cl_platform_id *platform_id;
cl_device_id device_id;   
cl_uint num_devices;
cl_uint num_platforms;
cl_int errcode;
cl_context clGPUContext;
cl_command_queue clCommandQue;
cl_program clProgram;
cl_device_local_mem_type local_mem_type;	
cl_ulong global_mem_size;
cl_ulong global_mem_cache_size;
cl_ulong global_mem_cacheline_size;
cl_ulong max_mem_alloc_size;
cl_ulong local_mem_size;
 

//OpenCL kernels
cl_kernel reduce_kernel;
cl_kernel vertex_gauss_kernel;
cl_kernel edge_kernel_1d;


//OpenCL memory objects
cl_mem cl_k_matrix;

//GPU3 memory objects
cl_mem cl_sp_adj_g1;
cl_mem cl_sp_adj_g2;
cl_mem cl_adj1;
cl_mem cl_adj2;
cl_mem cl_feat_g1;
cl_mem cl_feat_g2;
cl_mem cl_sp_edge_num;
cl_mem cl_sp_edge_offset;
cl_mem cl_sp_edge_w;
cl_mem cl_sp_edge_x;
cl_mem cl_sp_edge_y;
cl_mem cl_edge_offset;
cl_mem cl_edge_w;
cl_mem cl_edge_x;
cl_mem cl_edge_y;
cl_mem cl_vertex;
cl_mem cl_edge_g1;
cl_mem cl_edge_g2;
cl_mem cl_edge_x1;
cl_mem cl_edge_x2;
cl_mem cl_edge_y1;
cl_mem cl_edge_y2;
cl_mem cl_edge;


//GPU1 memory objects
cl_mem cl_sp_adj;
cl_mem cl_feat;
cl_mem cl_graph_node;
cl_mem cl_adj_offset;
cl_mem cl_feat_offset;
cl_mem cl_graph1_num;
cl_mem cl_graph2_num;

//RW memory objects
cl_mem cl_kroneck_p;      // padding matrix
cl_mem cl_kroneck_unp; // unpadding matrix
cl_mem cl_kroneck_agg;

//unord kernel memory objects
cl_mem cl_adj;

//opencl init
void OpenCL_init();
void read_cl_file();
void cl_initialization();
void cl_load_prog();
void cl_clean_up();


//utility
void NormalizeKMatrix();
int iskey(char **dest, char *str, int n);
int lookupchar(char **dest, char *str, int n);
int lookupint(int **dest, int *key, int dim0, int dim1);
void insert(int *dest, int num, int n);
double rtclock();
void OpenCLErrorCheck(const cl_int status, const char * func);
template <typename T> void print1dto2d(T *a, int x, int y);
template <typename T> void print2d(T **a,int x, int y);
template <typename T> void print3d(T ***a,int x, int y, int z);
template <typename T> void print1d(T *a,int x);

template <typename T> void array_init(T *a, int n, T val);
template <typename T> void vect_add(T *a, T *b, int n, double x);
template <typename T> void matrixmul_cpu(T *m1, int rowa, int cola, T *m2, int rowb, int colb, T *m3);
template <typename T> T vect_sum(T *a, int n);
template <typename T> void printgpumem1d(cl_mem a, int n, T nth);
template <typename T> void printgpumem2d(cl_mem a, int n, int m,T nth);

//malloc
template <typename T> void free2d(T **a);
template <typename T> void free3d(T ***a);
int **malloc2d(int x, int y);
float **malloc2dfloat(int x, int y);
double **malloc2ddouble(int x, int y);
int ***malloc3d(int x, int y, int z);
char **malloc2dchar(int x, int y);
char ***malloc3dchar(int x, int y, int z);
double ***malloc3ddouble(int x, int y, int z);


//read input
void read_input_file(int argc, char *argv[]);
void read_graph(int i);
void init_graph_info();
void init_graph_info_gpu4();
void init_adj_list(int g);

//cpu_func
void convert_to_sp(int i);
void count_sp_edge(int k);
void init_edge(int k);
double gaussian_k(int g1, int g2, int idx1, int idx2);
double intersect_k(int g1, int g2, int idx1, int idx2);
double drac_k(int g1, int g2, int idx1, int idx2) ;
void SPGK_CPU();
void clean_up();
void pack_sp_edge();
void pack_adj_list();
void output(int argc, char *agrv[]);



//spk PIM
void SPGK_PIM();
void pim_launch_vert_gauss_kernel(void *vert, void *feat1, void *feat2, int n1, int n2, int nfeat, double param, pim_device_id target);
void pim_launch_edge_kernel(void *edge, void *vert, void *w1, void *w2, void *x1, void *x2, void *y1, void *y2, int edge1, int edge2, int node1, int node2, double param, pim_device_id target);
void pim_launch_reduce_kernel(void *input, void *result, int num, pim_device_id target);

//spk gpu
void SPGK_GPU();        //copy data of a pair of graphs into GPU memroy for each kernel execution
double execute_spgk_gpu(int g1, int g2);     //copy data of a pair of graphs into GPU memroy for each kernel execution

//kernerl launch 
void cl_launch_vert_gauss_kernel(cl_mem vert, cl_mem feat1, cl_mem feat2, int n1, int n2, int nfeat, double y);
void cl_launch_edge_kernel_1d(cl_mem edge, cl_mem vert, cl_mem w1, cl_mem w2, cl_mem x1, cl_mem x2, cl_mem y1, cl_mem y2, int edge1, int edge2, int node1, int node2, double para);
void cl_launch_reduce(cl_mem input, cl_mem result, int num);

template <typename T> void gpu_mem_sum(cl_mem a,int n, T *sum);

class Graph
{
    public:
        Graph()
        //Graph(const std::string& fname)
        {
            //graph_fname = fname;
            //label = fname.substr(fname.find_last_of("/") + 1);
	        //label = label.substr(0, label.find_first_of("_"));
            n_node=0;
            n_edge=0;
            n_sp_edge=0;
            t_neighbor=0;
        }
        ~Graph()
        {
            free(sp_edge_x);
            free(sp_edge_y);
            free(sp_edge_w);
            free(edge_x);
            free(edge_y);
            free(edge_w);
            free(adj_list);
            free(adj_list_offset);
            free(n_neighbor);
            free2d(feat);
            free2d(adj);
            free2d(sp_adj);
            

        }
        double **feat;     // feature vector
        double **adj;      // adjacent matrix
        double **sp_adj;   // all shortest path adjacent matrix
        double *sp_edge_w; // weight for shortest path
        double *edge_w;    // weight for edge
                
        int n_node;        // number of nodes
        int n_edge;        // number of edges
        int n_sp_edge;     // number of shortest paths
        int t_neighbor;    // number of total neighbors, size of adj_list
        int *sp_edge_x;    // x coordinate for shortest path
        int *sp_edge_y;    // y coordinate for shortest path
        int *edge_x;       // x coordinate for edge
        int *edge_y;       // y coordinate for edge
        int *adj_list;     // adjacent list for the graph
        int *adj_list_offset; // offset for each node's adjacent list in adj_list
        int *n_neighbor;   // number of neighbors for each node
        
        char graph_fname[1024];
        char label[24];
        
    private:
        
};

Graph *graph;




