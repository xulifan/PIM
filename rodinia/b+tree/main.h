// # ifdef __cplusplus
// extern "C" {
// # endif

//===============================================================================================================================================================================================================200
//	HEADER
//===============================================================================================================================================================================================================200
#include <pim.h>
#include "pim_utility.c"
void 
kernel1_pim_wrapper(	record *records,
							long records_mem,
							knode *knodes,
							long knodes_elem,
							long knodes_mem,

							int order,
							long maxheight,
							int count,

							long *currKnode,
							long *offset,
							int *keys,
							record *ans);

// launch the PIM kernel for K queries on the B+tree
void pim_launch_findK_kernel(void *knodes, void *records, void *currKnode, void *offset, void *keys, void * ans, long height, long knodes_elem, int start_point, int order, int own_num_points, pim_device_id target, cl_event *complete);


void 
kernel2_pim_wrapper(        knode *knodes,
							long knodes_elem,
							long knodes_mem,

							int order,
							long maxheight,
							int count,

							long *currKnode,
							long *offset,
							long *lastKnode,
							long *offset_2,
							int *start,
							int *end,
							int *recstart,
							int *reclength);

// launch the PIM kernel for K range queries on the B+tree
void pim_launch_findRangeK_kernel(void *knodes, void *currKnode, void *offset, void *lastKnode, void * offset_2,  void *start, void *end, void *Recstart, void *Reclen, long height, long knodes_elem, int start_point, int order, int own_num_points, pim_device_id target, cl_event *complete);

int 
main(	int argc, 
		char *argv []);

//===============================================================================================================================================================================================================200
//	END
//===============================================================================================================================================================================================================200

// # ifdef __cplusplus
// }
// # endif
