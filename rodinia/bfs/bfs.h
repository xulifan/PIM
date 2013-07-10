#define __CL_ENABLE_EXCEPTIONS
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>

#ifdef  PROFILING
#include "timer.h"
#endif

#include "CLHelper.h"
#include "util.h"

#include "pim_utility.c"
struct Node
{
	int starting;
	int no_of_edges;
};



#define MAX_THREADS_PER_BLOCK 256

//Structure to hold a node information


void pim_launch_bfs2_kernel(void *g_graph_mask, void *g_updating_graph_mask, void *g_graph_visited, void *g_over, void *g_cost, int no_of_nodes, int start_node, int end_node, int own_num_nodes, int level, pim_device_id target, cl_event *complete);
void pim_launch_bfs1_kernel(void *g_graph_nodes, void *g_graph_edges, void *g_graph_mask, void *g_updating_graph_mask, void *g_graph_visited, void *g_cost, int no_of_nodes, int start_node, int end_node, int own_num_nodes, pim_device_id target, cl_event *complete);

template <typename T> void pim_array_sync(T *host_array, T **pim_mapped_array, void **pim_array, int array_size, int num_gpus);
template <typename T> void array_merge(T *original, T *source, T *result, int n);
void run_bfs_pim(int no_of_nodes, Node *h_graph_nodes, int edge_list_size, \
		int *h_graph_edges, char *h_graph_mask, char *h_updating_graph_mask, \
		char *h_graph_visited, int *h_cost) ;
