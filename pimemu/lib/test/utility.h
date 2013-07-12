#include <CL/cl.h>
#include <pim.h>

static const char *getOpenCLErrorString(cl_int err);
static const char *getPIMErrorString(int err);

uint32_t get_number_pim_compute_cores(pim_device_id pim_id, uint32_t core_type);
unsigned int find_pims();

void map_pims(int num_pims, int *num_gpus, int *num_cpus, int *gpus_per_pim, int *cpus_per_pim, pim_device_id *list_of_pims);
//void get_pim_cpu_ids(int num_cpus, pim_device_id *list_of_cpus);
//void get_pim_gpu_ids(int num_gpus, pim_device_id *list_of_gpus);

void pim_property(int num_pims, int *gpus_per_pim, int *cpus_per_pim, pim_device_id *list_of_pims);

void pim_spawn_args(void **args,size_t *sizes, size_t *nargs, char *ocl_source, char *build_flags, int dim, size_t glbl[3], size_t lcl[3], int num_pre_event, cl_event * pre_event_list, cl_event *post_event_list);
