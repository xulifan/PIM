#include <CL/cl.h>
#include <pim.h>
#include <iostream>

static const char *getOpenCLErrorString(cl_int err);
static const char *getPIMErrorString(int err);

uint32_t get_number_pim_compute_cores(pim_device_id pim_id, uint32_t core_type);
unsigned int find_pims();

void map_pims(int num_pims, int *num_gpus, int *num_cpus, int *gpus_per_pim, int *cpus_per_pim, pim_device_id *list_of_pims);
//void get_pim_cpu_ids(int num_cpus, pim_device_id *list_of_cpus);
//void get_pim_gpu_ids(int num_gpus, pim_device_id *list_of_gpus);

void pim_property(int num_pims, int *gpus_per_pim, int *cpus_per_pim, pim_device_id *list_of_pims);

void pim_spawn_args(void **args,size_t *sizes, size_t *nargs, char *ocl_source, char *build_flags, int dim, size_t *glbl, size_t *lcl, int *num_pre_event, cl_event * pre_event_list, cl_event *post_event_list);

template <typename T> void array_merge(T *original, T *source, T *result, int n);
template <typename T> void pim_array_sync(T *host_array, T **pim_mapped_array, void **pim_array, int array_size, int num_gpu);
template <typename T> void pim_memcpyDtoH(T *host_array, void *pim_array, int array_size);
template <typename T> void pim_memcpyHtoD(T *host_array, void *pim_array, int array_size);
template <typename T> void print1d(T *a,int x);

void pim_domain_decomposition(int *start_point, int *end_point, int *own_num_points, int num_gpus, int npoints);

template <typename T> void print1d(T *a,int x);
template <typename T> void pim_print1d(T* mapped_a, void *a,int x);
