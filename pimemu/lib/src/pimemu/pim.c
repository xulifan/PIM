/*******************************************************************************
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 *
 * RESTRICTED RIGHTS NOTICE (DEC 2007)
 * (a)     This computer software is submitted with restricted rights under
 *     Government Contract No. DE-AC52-8MA27344 and subcontract B600716. It
 *     may not be used, reproduced, or disclosed by the Government except as
 *     provided in paragraph (b) of this notice or as otherwise expressly
 *     stated in the contract.
 *
 * (b)     This computer software may be -
 *     (1) Used or copied for use with the computer(s) for which it was
 *         acquired, including use at any Government installation to which
 *         the computer(s) may be transferred;
 *     (2) Used or copied for use with a backup computer if any computer for
 *         which it was acquired is inoperative;
 *     (3) Reproduced for safekeeping (archives) or backup purposes;
 *     (4) Modified, adapted, or combined with other computer software,
 *         provided that the modified, adapted, or combined portions of the
 *         derivative software incorporating any of the delivered, restricted
 *         computer software shall be subject to the same restricted rights;
 *     (5) Disclosed to and reproduced for use by support service contractors
 *         or their subcontractors in accordance with paragraphs (b)(1)
 *         through (4) of this notice; and
 *     (6) Used or copied for use with a replacement computer.
 *
 * (c)     Notwithstanding the foregoing, if this computer software is
 *         copyrighted computer software, it is licensed to the Government with
 *         the minimum rights set forth in paragraph (b) of this notice.
 *
 * (d)     Any other rights or limitations regarding the use, duplication, or
 *     disclosure of this computer software are to be expressly stated in, or
 *     incorporated in, the contract.
 *
 * (e)     This notice shall be marked on any reproduction of this computer
 *     software, in whole or in part.
 ******************************************************************************/

#include "pim-private.h"
#include "xml_parse.h"
#include "cpu_profile.h" // for cpu profiling 
#include "validate_config.h" // for validating configuration

/************** Global variables shared throughout the emulator **************/
// This RW lock (since the vast majority of work will be by read-only users)
// is to guard the 
static
pthread_rwlock_t configuration_rwlock;

static
pim_emu_configuration_t pim_emu_config;

static
pim_emu_configuration_t *base_pim_emu_config = 0;

static
char * config_xml = 0;

static
char * temp_file_loc = 0;

static
pthread_rwlock_t pim_mem_rwlock;

static
pim_mem_object_t *pim_memory = 0;

/********************************************************************************/
static
pthread_rwlock_t pim_data_stream_lock;


/*******************************************************************************/



#define PIM_GENERAL_TRY(function, msg, failret)                         \
{                                                                       \
    int failvar = 0;                                                    \
    failvar = function;                                                 \
    if (failvar != 0)                                                   \
    {                                                                   \
        if (msg != NULL)                                                \
            fprintf(stderr, "%s (%d): %s", __func__, __LINE__, msg);    \
        errno = failvar;                                                \
        return failret;                                                 \
    }                                                                   \
}

#define PIM_EXIT_TRY(function, msg)                                     \
{                                                                       \
    int failvar = 0;                                                    \
    failvar = function;                                                 \
    if (failvar != 0)                                                   \
    {                                                                   \
        if (msg != NULL)                                                \
            fprintf(stderr, "%s (%d): %s", __func__, __LINE__, msg);    \
        exit(failvar);                                                  \
    }                                                                   \
}

#define PIM_MSG(msg)               \
{                                         \
   if (msg != NULL)                       \
   { \
      fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, msg);    \
   }\
}


static
void get_value_from_ehp(void *param_value,
	                        size_t *param_value_size_ret,
                            pim_device_info param_name);

static int mem_mappedptr_criteria(pim_mem_object_t * pointer, void * mappedptr)
{
    int ret = 0;
    ret = (int)((pointer->mapped_pointer == mappedptr) && pointer->currently_mapped);
    return(ret);
}

int isEmuInitialized(void)
{
	return( base_pim_emu_config != NULL );
}

extern
const pim_emu_configuration_t *getEmuArchitectureConfig(void )
{
	return(base_pim_emu_config);
}

extern
const char * getEmuTempFilesLocation(void)
{
	return((const char*)temp_file_loc);
}

extern
const char * getEmuArchitectureConfigFile(void)
{
	return((const char*)config_xml);
}

// CALLED FROM CONSTRUCTOR
static
int pim_init(char* conf_file)
{
    int i, j, gpu_failure;
    pim_emu_configuration_t temp_configuration;
    int failure, xml_filed;


    PIM_GENERAL_TRY(pthread_rwlock_init(&pim_data_stream_lock, NULL),
        "Unable to initialize PIM emu gathering data RWLock\n",
               PIM_GENERAL_ERROR);


    PIM_GENERAL_TRY(pthread_rwlock_init(&configuration_rwlock, NULL),
        "Unable to initialize PIM emu configuration RWLock\n",
        PIM_GENERAL_ERROR);

    PIM_GENERAL_TRY(pthread_rwlock_init(&pim_mem_rwlock, NULL),
        "Unable to initialize PIM emu mem RWLock\n", PIM_GENERAL_ERROR);

    // Parse out information from conf_file into this configuration
    xml_filed = open(conf_file, O_RDONLY);
    if (xml_filed < 0)
        return PIM_INIT_FILE_ACCESS_ERROR;
    // parse_pim_config() does an roxml_close() which actually fcloses the fd
    failure = parse_pim_config(xml_filed, &temp_configuration);
    if (failure != PIM_SUCCESS)
        return failure;
    //Vaidate the configuration file
    failure = validate_configuration(&temp_configuration);
    if (failure != PIM_SUCCESS)
        return failure;
    
    // initialize PIM GPU interface
    for (i = 0; i < (int)temp_configuration.num_pim_cores; i++)  {
        // one context per PIM
        pim_configuration_t *pim;
        gpu_configuration_t *gpus;
        
        pim = temp_configuration.pims + i;
        if (pim == NULL) {
            fprintf(stderr, "Unable to initialize GPUs in PIM %d\n", i);
            fprintf(stderr, "PIM internal data structure does not exist.\n");
            exit(-1);
        }
        gpus = pim->gpus;

        if (pim->num_gpu_cores > 0) {
            for (j = 0; j < (int)pim->num_gpu_cores; j++) {
                // init OPENCL queue PER GPU CORE
                gpu_failure = pim_get_gpu_id(&gpus[j].gpu_handle);
                // FIXME if there is no devices?
                if (gpu_failure != PIM_SUCCESS) {
                    fprintf(stderr,
                            "%s (%d): Cannot init a real GPU HW id %d; on PIM id %d \n",
                            __func__, __LINE__, j, i); 
                    exit(-1);
                }
            }
        }
    }

    PIM_GENERAL_TRY(pthread_rwlock_wrlock(&configuration_rwlock),
        "Unable to write acquire PIM emu config RWLock\n", PIM_GENERAL_ERROR);
    pim_emu_config = temp_configuration;
	base_pim_emu_config = &pim_emu_config;
    pim_memory = NULL;

    PIM_EXIT_TRY(pthread_rwlock_unlock(&configuration_rwlock),
        "Unable to unlock PIM emu config RWLock\n");
    return PIM_SUCCESS;
}

int pim_get_device_ids(pim_device_type device_type,
    uint32_t num_entries, pim_device_id *devices, uint32_t *num_devices)
{
    uint32_t number_of_pims, i;
    uint32_t number_to_copy;
    // For now, we only define one PIM device type
    if (device_type != PIM_CLASS_0)
        return PIM_INVALID_DEVICE_TYPE;
    PIM_GENERAL_TRY(pthread_rwlock_rdlock(&configuration_rwlock),
        "Unable to read acquire PIM emu config RWLock\n", PIM_GENERAL_ERROR);
    number_of_pims = pim_emu_config.num_pim_cores;
    PIM_EXIT_TRY(pthread_rwlock_unlock(&configuration_rwlock),
        "Unable to unlock PIM emu config RWLock\n");

    if (num_devices != NULL)
        *num_devices = number_of_pims;

    if (devices != NULL) {
        if (number_of_pims > num_entries)
            number_to_copy = num_entries;
        else
            number_to_copy = number_of_pims;

        for (i = 0; i < number_to_copy; i++) {
            // Currently, we address PIMs sequentially from 0 to the max number
            // So instead of reading any more config info out, we can just
            // fill in the device return array with sequential numbers
            devices[i] = i;
        }
    }

    return PIM_SUCCESS;
}

int pim_get_device_info(pim_device_id device, pim_device_info param_name,
    size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    uint32_t number_of_pims;
    int error_val = PIM_SUCCESS;
    uint32_t *uint32_array = (uint32_t *)param_value;
    uint64_t *uint64_array = (uint64_t *)param_value;
    // Must put a big lock around all of this, or we will get an atomicity
    // violation later. We expect number of pims to stay the same throughout
    // this function until we return everything.
    PIM_GENERAL_TRY(pthread_rwlock_rdlock(&configuration_rwlock),
            "Unable to read acquire PIM emu config RWLock\n",
            PIM_GENERAL_ERROR);

    number_of_pims = pim_emu_config.num_pim_cores;
    
    // Figure out which PIM configuration data is desired so that we know
    // how much space is needed and what to copy.
    if (param_name > PIM_MAXIMUM_PARAMETER) {
        PIM_EXIT_TRY(pthread_rwlock_unlock(&configuration_rwlock),
                "Unable to unlock PIM emu config RWLock\n");
        return PIM_INVALID_PARAMETER;
    }

    if (device == PIM_HOST_ID) {
        if (param_value_size_ret != NULL) {
             *param_value_size_ret = sizeof(uint32_t);
		}
		if (param_value) {
		    switch (param_name)
		    {
                case HOST_CPU_CORES:
                case HOST_CPU_FREQ:
                case HOST_GPU_CORES:
                case HOST_GPU_NUM_CUS:
                case HOST_GPU_FREQ:
                    get_value_from_ehp(param_value,
                            param_value_size_ret,
                            param_name);
                    break;
                default:
                    error_val = PIM_INVALID_PARAMETER;
            }
		}
        PIM_EXIT_TRY(pthread_rwlock_unlock(&configuration_rwlock),
                "Unable to unlock PIM emu config RWLock\n");
        return error_val;
	}

    // Currently number devices sequentially. e.g. 2 PIMs are 0 and 1.
    // device = 2 is therefore an error.
    if (device >= number_of_pims) {
        PIM_EXIT_TRY(pthread_rwlock_unlock(&configuration_rwlock),
                "Unable to unlock PIM emu config RWLock\n");
        return PIM_INVALID_DEVICE_ID;
    }

    // Check to see if we have enough space on the return stack
    // param_value != NULL, so if we have enough space, time for a copy
    switch (param_name)
    {
        case PIM_MEM_CAPACITY:
        case PIM_MEM_BANDWIDTH:
        case PIM_HOST_BANDWIDTH:
        case PIM_INTERPIM_BANDWIDTH:
            if (param_value_size_ret != NULL)
                *param_value_size_ret = sizeof(uint32_t);
            if (param_value != NULL) {
                if (param_value_size < sizeof(uint32_t))
                    error_val = PIM_INSUFFICIENT_SPACE;
                else
                    get_value_from_dram(device, uint32_array, param_name);
            }
            break;
        case PIM_MEM_FREE:
            if (param_value_size_ret != NULL)
                *param_value_size_ret = sizeof(uint64_t);
            if (param_value != NULL) {
                if (param_value_size < sizeof(uint64_t))
                    error_val = PIM_INSUFFICIENT_SPACE;
                else
                    get_free_dram_space(device, uint64_array);
            }
            break;
        case PIM_CPU_CORES:
        case PIM_GPU_CORES:
            if (param_value_size_ret != NULL)
                *param_value_size_ret = sizeof(uint32_t);
            if (param_value != NULL) {
                if (param_value_size < sizeof(uint32_t))
                    error_val = PIM_INSUFFICIENT_SPACE;
                else
                    get_value_from_pim(device, uint32_array, param_name);
            }
            break;
        case PIM_CPU_FREQ:
            if (param_value_size_ret != NULL)
                *param_value_size_ret = sizeof(uint32_t);
            if (param_value != NULL) {
                if (param_value_size < sizeof(uint32_t))
                    error_val = PIM_INSUFFICIENT_SPACE;
                else
                    get_value_from_pim_cpu(device, 0, param_value, param_value_size_ret, param_name);
            }
		case PIM_GPU_NUM_CUS:
		case PIM_GPU_FREQ:
            if (param_value_size_ret != NULL)
                *param_value_size_ret = sizeof(uint32_t);
            if (param_value != NULL) {
                if (param_value_size < sizeof(uint32_t))
                    error_val = PIM_INSUFFICIENT_SPACE;
                else
                     get_value_from_pim_gpu(device, 0, param_value, param_value_size_ret, param_name);
			}
			break;
		case PIM_MEM_MBPS:
		case PIM_MEM_NUM_CHANNELS:
            if (param_value_size_ret != NULL)
                *param_value_size_ret = sizeof(uint32_t);
            if (param_value != NULL) {
                if (param_value_size < sizeof(uint32_t))
                    error_val = PIM_INSUFFICIENT_SPACE;
                else
                    get_value_from_pim_dram(device, 0, param_value, param_value_size_ret, param_name);
			}
			break;
        default:
			error_val = PIM_INVALID_PARAMETER;
            break;
    }

    PIM_EXIT_TRY(pthread_rwlock_unlock(&configuration_rwlock),
            "Unable to unlock PIM emu config RWLock\n");

    return error_val;
}

static
void get_value_from_ehp(void *param_value,
	                        size_t *param_value_size_ret,
                            pim_device_info param_name)
{
	const pim_emu_configuration_t * conf = getEmuArchitectureConfig();
	    if ( param_value_size_ret)
	    {
	   	   *param_value_size_ret = sizeof(uint32_t);
		}
		switch (param_name)
		{
		case HOST_CPU_CORES :
			*(uint32_t*)param_value = conf->ehp.num_cpu_cores;
			break;
		case HOST_CPU_FREQ :
			*(uint32_t*)param_value = conf->ehp.cpus[0].freq;
			break;
		case HOST_GPU_CORES :
			*(uint32_t*)param_value = conf->ehp.num_gpu_cores;
			break;
        case HOST_GPU_NUM_CUS :
			if( conf->ehp.num_gpu_cores > 0 )
			    *(uint32_t*)param_value = conf->ehp.gpus[0].cus;
			break;
		case HOST_GPU_FREQ:
			if( conf->ehp.num_gpu_cores > 0 )
			    *(uint32_t*)param_value = conf->ehp.gpus[0].freq;
			break;
		default:
			break;
		}
}

// configuration_rwlock must be at least Read-Locked when calling this function
void get_value_from_dram(uint32_t device, uint32_t *output,
        pim_device_info param_name)
{
    int j;
    for (j = 0; j < (int)pim_emu_config.pims[device].num_dram_stacks; j++) {
        double temp_val;
        temp_val = ((double)pim_emu_config.pims[device].dram[0].freq * 1000000. *
                pim_emu_config.pims[device].dram[0].num_channels *
                pim_emu_config.pims[device].dram[0].channel_width *
                pim_emu_config.pims[device].dram[0].bits_percycle *
                (double) pim_emu_config.pims[device].dram[0].utilization/100.)/8; //bytes
        temp_val = (temp_val / 1000000); // megabytes

        switch(param_name)
        {
            case PIM_MEM_CAPACITY:
                *output += pim_emu_config.pims[device].dram[j].size_in_mb;
                break;
            case PIM_MEM_BANDWIDTH:
                // FIXME Doesn't work with more than one DRAM of different BW
                *output = (uint32_t)temp_val;
                break;
            case PIM_HOST_BANDWIDTH:
                // FIXME Same as above
                *output = (uint32_t)(temp_val * ((double)pim_emu_config.pims[device].dram[0].off_stack_bw)/100);
                break;
            case PIM_INTERPIM_BANDWIDTH:
                // FIXME Same as above
                *output = (uint32_t)(temp_val * ((double)pim_emu_config.pims[device].dram[0].between_pim_bw)/100);
                break;
            default:
                break;
        }
    }
}

// configuration_rwlock must be at least Read-Locked when calling this function
void get_free_dram_space(uint32_t device, uint64_t *output)
{
    // TODO:We currently don't keep track of actual amount of free space.
    // Instead, we will just return "all space is free". This is wrong!
    int j;
    for (j = 0; j < (int)pim_emu_config.pims[device].num_dram_stacks; j++) {
        *output = (pim_emu_config.pims[device].dram[j].size_in_mb * 1048576);
    }
}

// configuration_rwlock must be at least Read-Locked when calling this function
void get_value_from_pim(uint32_t device, uint32_t *output,
        pim_device_info param_name)
{
    switch(param_name)
    {
        case PIM_CPU_CORES:
            *output = pim_emu_config.pims[device].num_cpu_cores;
            break;
        case PIM_GPU_CORES:
            *output = pim_emu_config.pims[device].num_gpu_cores;
            break;
        default:
            break;
    }
}

void get_value_from_pim_cpu(uint32_t device, uint32_t cpu, void *param_value,
                            size_t *param_value_size_ret,
                            pim_device_info param_name)
{
    // Currently always return uint32_t. Technically, ISA and Ordering are
    // uint8_t, but we currently don't allow them to be queried through this.
    if (param_value_size_ret)
        *param_value_size_ret = sizeof(uint32_t);
    switch(param_name)
    {
        case PIM_CPU_FREQ:
            *(uint32_t *)param_value = (uint32_t)pim_emu_config.pims[device].cpus[cpu].freq;
            break;

        default:
            break;
    }
}

void get_value_from_pim_gpu(uint32_t device, uint32_t gpu, void *param_value,
	                        size_t *param_value_size_ret,
                            pim_device_info param_name)
{
    if (param_value_size_ret)
	    *param_value_size_ret = sizeof(uint32_t);
    switch(param_name)
    {
		case PIM_GPU_NUM_CUS:
            *(uint32_t *)param_value = (uint32_t)pim_emu_config.pims[device].gpus[gpu].cus;
            break;
		case PIM_GPU_FREQ:
            *(uint32_t *)param_value = (uint32_t)pim_emu_config.pims[device].gpus[gpu].freq;
			break;

        default:
            break;
    }
}

void get_value_from_pim_dram(uint32_t device, uint32_t dram, void *param_value,
	                        size_t *param_value_size_ret,
                            pim_device_info param_name)
{
    if (param_value_size_ret)
	    *param_value_size_ret = sizeof(uint32_t);
    switch(param_name)
    {

		case PIM_MEM_MBPS:
            *(uint32_t *)param_value = (uint32_t)pim_emu_config.pims[device].dram[dram].freq *
                (uint32_t)pim_emu_config.pims[device].dram[dram].bits_percycle;
			break;

		case PIM_MEM_NUM_CHANNELS:
            *(uint32_t *)param_value = (uint32_t)pim_emu_config.pims[device].dram[dram].num_channels;
            break;
        default:
            break;
    }
}

// Must hold pim_mem_rwlock() as Writer before calling this.
int insert_memobj(pim_mem_object_t *to_insert)
{
    pim_mem_object_t *pointer;
    pim_mem_object_t *previous_pointer = NULL;
    if (to_insert == NULL) {
        fprintf(stderr,
            "%s (%d): Attempting to insert a null pointer into linked list\n",
            __func__, __LINE__);
        return PIM_GENERAL_ERROR;
    }
    pointer = pim_memory; // Must be here, or data race.
    while (pointer != NULL &&
            pointer->address_of_object < to_insert->address_of_object) {
        previous_pointer = pointer;
        pointer = pointer->next;
    }
    to_insert->next = pointer;
    if (previous_pointer == NULL) { // Insert at beginning of list
        pim_memory = to_insert;
    }
    else {
        previous_pointer->next = to_insert;
    }
    return PIM_SUCCESS;
}

// Must hold pim_mem_rwlock() as Writer before calling this.
int remove_memobj(pim_mem_object_t *to_remove)
{
    pim_mem_object_t *pointer;
    pim_mem_object_t *previous_pointer = NULL;
    if (to_remove == NULL) {
        fprintf(stderr,
            "%s (%d): Attempting to remove a null pointer from linked list\n",
            __func__, __LINE__);
        return PIM_GENERAL_ERROR;
    }
    pointer = pim_memory;
    while (pointer != NULL && pointer != to_remove) {
        previous_pointer = pointer;
        pointer = pointer->next;
    }
    if (pointer == to_remove) {
        if (previous_pointer == NULL)
            pim_memory = pointer->next;
        else
            previous_pointer->next = pointer->next;
        free(pointer);
        return PIM_SUCCESS;
    }
    else
        return PIM_GENERAL_ERROR;
}

// Must hold pim_mem_rwlock() as at least Reader before calling this.
pim_mem_object_t *find_memobj_bycrit(MEM_OBJ_CRITERIA_F crit_f, void *serach_val)
{
    pim_mem_object_t *pointer;
	int found = 0;

    pointer = pim_memory;
    while (pointer != NULL && !(found = crit_f(pointer, serach_val))) {
        pointer = pointer->next;
    }
    if (pointer != NULL && found)
        return pointer;
    else
        return NULL;
}

// Must hold pim_mem_rwlock() as at least Reader before calling this.
pim_mem_object_t *find_memobj(void *address)
{
    pim_mem_object_t *pointer;
    uintptr_t addr_val = (uintptr_t)address;
    pointer = pim_memory;
    while (pointer != NULL && pointer->address_of_object < addr_val) {
        pointer = pointer->next;
    }
    if (pointer != NULL && pointer->address_of_object == addr_val)
        return pointer;
    else
        return NULL;
}

int pim_get_id(void)
{
	return(getPIMID());
}

void *pim_malloc(size_t size, pim_device_id device, pim_acc_flags acc_flags, pim_platform_type platform)
{
    // Dead simple beginning
    if (platform == PIM_PLATFORM_PTHREAD_CPU) {
        void *return_val;
        int insert_success;
        pim_mem_object_t *cpu_memory_object;
		void * allocated_ptr = NULL;
		switch ( acc_flags & PIM_MEM_ALIGNED )
		{
		case PIM_MEM_ALIGN_WORD :
		     {
			    return_val = pim_cpu_malloc_aligned(size, 8, device, acc_flags, &allocated_ptr);
		     }
			 break;
		case PIM_MEM_ALIGN_PAGE :
		     {
             size_t page_sz = 0;

		         page_sz = getpagesize();
			     return_val = pim_cpu_malloc_aligned(size, page_sz, device, acc_flags, &allocated_ptr);
			 
		     }
			 break;
		case PIM_MEM_ALIGN_CACHELINE :
		     {
             size_t cache_sz = 0;
			     cache_sz = cache_line_size();
			     return_val = pim_cpu_malloc_aligned(size, cache_sz, device, acc_flags, &allocated_ptr);

		      }
			 break;
		default:
		
		     {
                 allocated_ptr = return_val = pim_cpu_malloc(size, device, acc_flags);
		     }
		}

        if (return_val != NULL) {
            cpu_memory_object = (pim_mem_object_t *)calloc(1, sizeof(pim_mem_object_t));
            if (cpu_memory_object == NULL) {
                fprintf(stderr, "%s (%d): internal malloced failed.\n%s\n",
                        __func__, __LINE__, strerror(errno));
                exit(-1);
            }
            cpu_memory_object->address_of_object = (uintptr_t)return_val;
            cpu_memory_object->mapped_pointer = return_val;
			cpu_memory_object->allocated_pointer  = allocated_ptr;
			cpu_memory_object->flags = acc_flags;
            cpu_memory_object->currently_mapped = 0;
            cpu_memory_object->type = PIM_PLATFORM_PTHREAD_CPU;
            if (pthread_rwlock_wrlock(&pim_mem_rwlock)) {
                fprintf(stderr, "%s (%d): Unable to acquire PIM memory RWLock\n",
                        __func__, __LINE__);
                free(return_val);
                free(cpu_memory_object);
                return NULL;
            }
            insert_success = insert_memobj(cpu_memory_object);
            PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                    "Unable to unlock PIM memory RWLock\n");
            if (insert_success != PIM_SUCCESS) {
                free(return_val);
                free(cpu_memory_object);
                errno = PIM_GENERAL_ERROR;
                return NULL;
            }
        }
        return return_val;
    }
    else if (platform == PIM_PLATFORM_OPENCL_GPU) {
		PIM_GPU_MEM return_val;
        int insert_success;
        pim_mem_object_t *gpu_memory_object;
		PIM_GPU_ID gpu_dev = 0;
		if (pim_emu_config.pims[device].num_gpu_cores > 0) {
		   gpu_dev = (PIM_GPU_ID)pim_emu_config.pims[device].gpus[0].gpu_handle;
		}

		if (pim_emu_config.pims[device].num_gpu_cores == 0 || gpu_dev == 0) {
            fprintf(stderr, "%s (%d): Unable to acquire PIM-GPU handle\n",
                    __func__, __LINE__);
             return NULL;
		}

        return_val = pim_gpu_mem_alloc(gpu_dev, size, acc_flags);
        if (return_val != NULL) {
            gpu_memory_object = (pim_mem_object_t *)calloc(1, sizeof(pim_mem_object_t));
            if (gpu_memory_object == NULL) {
                fprintf(stderr, "%s (%d): internal malloced failed.\n%s\n",
                    __func__, __LINE__, strerror(errno));
                exit(-1);
            }
            gpu_memory_object->address_of_object = (uintptr_t)return_val;
			gpu_memory_object->gpu_handle = gpu_dev;
            gpu_memory_object->mapped_pointer = 0;
			gpu_memory_object->flags = acc_flags;
            gpu_memory_object->currently_mapped = 0;
            gpu_memory_object->type = PIM_PLATFORM_OPENCL_GPU;
            if (pthread_rwlock_wrlock(&pim_mem_rwlock)) {
                fprintf(stderr, "%s (%d): Unable to acquire PIM memory RWLock\n",
                        __func__, __LINE__);
                free(return_val);
                free(gpu_memory_object);
                return NULL;
            }
            insert_success = insert_memobj(gpu_memory_object);
            PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                    "Unable to unlock PIM memory RWLock\n");
            if (insert_success != PIM_SUCCESS) {
                free(return_val);
                free(gpu_memory_object);
                errno = PIM_GENERAL_ERROR;
                return NULL;
            }
        }
        return return_val;
    }
    else {
        errno = PIM_INVALID_DEVICE_TYPE;
        return NULL;
    }
}


void pim_free(void *object)
{
    pim_mem_object_t *memobj_to_free = NULL;
    if (pthread_rwlock_wrlock(&pim_mem_rwlock)) {
        fprintf(stderr, "%s (%d): Unable to acquire PIM memory RWLock\n",
                __func__, __LINE__);
        return;
    }
    memobj_to_free = find_memobj(object);
    if (memobj_to_free == NULL) {
        fprintf(stderr,
                "Attempting to pim_free() something at address %p that was not allocated with pim_malloc()\n",
                object);
        PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                "Unable to unlock PIM memory RWLock\n");
        return;
    }
    // It was in the list. Now we know how to remove it.
    if (memobj_to_free->type == PIM_PLATFORM_PTHREAD_CPU) {
        // On a CPU, this object is the actual thing.
        free( memobj_to_free->allocated_pointer);
    }
    else if (memobj_to_free->type == PIM_PLATFORM_OPENCL_GPU) {
	    PIM_GPU_ID gpu_dev = memobj_to_free->gpu_handle;
		pim_gpu_mem_free(gpu_dev,object);
    }
    remove_memobj(memobj_to_free);
    PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
            "Unable to unlock PIM memory RWLock\n");
}

void *pim_map(void *handle, pim_platform_type platform)
{
    void * ret_val = NULL;
    PIM_GPU_ID gpu_dev = 0;
    pim_mem_object_t *memobj_to_map = NULL;

    if (pthread_rwlock_wrlock(&pim_mem_rwlock)) {
        fprintf(stderr, "%s (%d): Unable to acquire PIM memory RWLock\n",
                __func__, __LINE__);
        errno = PIM_GENERAL_ERROR;
        return NULL;
    }

//	if ( platform == PIM_PLATFORM_OPENCL_GPU )
	{
        itIsRuntimeCall(1);
        beforeGenericEvent();
	}
    memobj_to_map = find_memobj(handle);
    if (memobj_to_map && memobj_to_map->type == PIM_PLATFORM_PTHREAD_CPU) {
        // CPU pim_malloc() actually just teturns a raw void*
        // So mapping it doesn't do anything diferent.
        PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                "Unable to unlock PIM memory config RWLock\n");
        return handle;
    }

    if (memobj_to_map && memobj_to_map->type == PIM_PLATFORM_OPENCL_GPU && !memobj_to_map->currently_mapped) {
        gpu_dev = memobj_to_map->gpu_handle;

        if (!gpu_dev) {
            fprintf(stderr, "%s (%d): failed to find gpu handle\n",
                    __func__, __LINE__);
            PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                    "Unable to unlock PIM memory config RWLock\n");
            errno = PIM_GENERAL_ERROR;
            afterGenericEvent();
            itIsRuntimeCall(0);
            return NULL;
        }

        ret_val = pim_gpu_mem_map(gpu_dev, handle, memobj_to_map->flags);
 
        if (!ret_val) {
            fprintf(stderr, "%s (%d): fail to map\n", __func__, __LINE__);
            PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                    "Unable to unlock PIM memory config RWLock\n")
            errno = PIM_GENERAL_ERROR;
            return NULL;
        }

        memobj_to_map->mapped_pointer = ret_val;
        memobj_to_map->currently_mapped = 1;
    }

	{
       afterGenericEvent();
       itIsRuntimeCall(0);
	}

    PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
            "Unable to unlock PIM memory config RWLock\n");
    return ret_val; // ret_val is NULL by default
}

int pim_unmap(void *ptr)
{
    int ret = PIM_SUCCESS;
    PIM_GPU_ID gpu_dev = 0;
    pim_mem_object_t *memobj_to_unmap = NULL;
    PIM_GENERAL_TRY(pthread_rwlock_wrlock(&pim_mem_rwlock),
            "Unable to acquire PIM memory RWLock\n", PIM_GENERAL_ERROR);

	{
        itIsRuntimeCall(1);
        beforeGenericEvent();
	}


    memobj_to_unmap = find_memobj_bycrit(mem_mappedptr_criteria, ptr);
    if ( !memobj_to_unmap ) {
        PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                "Unable to unlock PIM memory RWLock\n");
        return PIM_GENERAL_ERROR;
    }

    // Only PIM_PLATFORM_OPENCL_GPU needs to worry about this. There is no such
    // thing as "unmap" for PIM_PLATFORM_PTHREAD_CPU, since it is always a ptr
    if (memobj_to_unmap->type == PIM_PLATFORM_OPENCL_GPU && memobj_to_unmap->currently_mapped)
    {
        gpu_dev = memobj_to_unmap->gpu_handle;
        if (!gpu_dev) {
            errno = PIM_GENERAL_ERROR;
            PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
                    "Unable to unlock PIM memory RWLock\n");
            return PIM_GENERAL_ERROR;
        }
        ret = pim_gpu_mem_unmap(gpu_dev, (PIM_GPU_MEM)memobj_to_unmap->address_of_object, ptr);
        if (ret == PIM_SUCCESS) {
            memobj_to_unmap->mapped_pointer = 0;
            memobj_to_unmap->currently_mapped = 0;
        }
    }

	{
       afterGenericEvent();
       itIsRuntimeCall(0);
	}

    PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_mem_rwlock),
            "Unable to unlock PIM memory RWLock\n");
    return ret;
}

void *pim_spawn(pim_f to_launch, void **args, size_t *arg_sizes, size_t nargs,
    pim_device_id device, pim_platform_type platform_type)
{
    if (platform_type == PIM_PLATFORM_PTHREAD_CPU) {
        int return_val = spawn_cpu_thread(to_launch, args, arg_sizes, nargs, device);
        if (return_val == 0) {
            // This *(args[0]) is a points to the pthread_t* describing this launch
            return *(void**)(args[0]);
        }
        else {
            errno = PIM_GENERAL_ERROR;
            return NULL;
        }
    }
    else if (platform_type == PIM_PLATFORM_OPENCL_GPU) {
        int return_val = PIM_SUCCESS;
        PIM_GPU_ID gpu_dev = 0;
        void * kern;
        if (pim_emu_config.pims[device].num_gpu_cores > 0) {
            gpu_dev = (PIM_GPU_ID)pim_emu_config.pims[device].gpus[0].gpu_handle;
        }
        else {
            errno = PIM_GENERAL_ERROR;
            return NULL;
        }
        kern = to_launch.func_name;

		// stop counters
        incAndStopCnt();
        return_val = pim_gpu_spawn_inparallel_sync(gpu_dev, kern,
                (const void**)(args),(const size_t*)(arg_sizes), nargs);
// start counters
		decAndStartCnt();
        if (return_val == PIM_SUCCESS) {
            // FIXME!!
            return to_launch.func_name;
        }
        else {
            errno = PIM_GENERAL_ERROR;
            return NULL;
        }
    }
    else {
        errno = PIM_INVALID_DEVICE_TYPE;
        return NULL;
    }
}

/******************************************************************************/
/************* pim run-time internals ****************************************/



/*****************************************************************************/
/********** Functions to find correct file to send to pim_init() *************/
/*****************************************************************************/
static
char* fix_directory(char *input)
{
    char *directory = 0;
    size_t directory_length;

    directory_length = strlen(input);
	if (input[directory_length-1] != '/' &&
		input[directory_length-1] != '\\' )
	{

        directory_length += 1;

       directory = (char*)malloc(sizeof(char) * (directory_length + 1));
       assert(directory);

       strcpy(directory, input);
	   directory[directory_length-1] = '/';

       directory[directory_length] = '\0';

	}
	return(directory);
}

static
char* find_directory(char *input)
{
    char *directory;
    size_t directory_length;

	char * pos1 = strrchr(input, '/');
	char * pos2 = strrchr(input, '\\');
	char *faraway;

	if ( pos1 ||  pos2 )
	{
	    faraway = (!pos1)? pos2 : ((!pos2) ? pos1 : (( pos1 < pos2) ? pos2 : pos1));

        directory_length = (size_t)(faraway - input) + 1;
	}
	else
	{
		directory_length = 2;
	}

    directory = (char*)malloc(sizeof(char) * (directory_length + 1));

	assert(directory);

    if (pos1 ||  pos2 )
	{
       strncpy(directory, input, directory_length);
	   directory[directory_length] = 0;
	}
	else
	{
		strcpy(directory, "./");
	}
	return(directory);
}

// env format
// <CONFIG FILE>[[:<EMU MODE>]:<TEMP DIRECTORY>]

static
int getEnvConfig( char ** config_xml_nm_,  int * emu_mode_,  char ** temp_file_loc_, const char * env_ )
{
// wew've check 0 lenght before calling
int conf_len, mode_len, temp_len;
char * pos_s = (char*)env_;
char * pos_c = strchr((char*)pos_s, ':');

// xml location
    conf_len =  (int)((pos_c == 0 )? strlen(pos_s) : (int)(pos_c - pos_s));

	*config_xml_nm_ = (char*)malloc(conf_len+1);
	memcpy(*config_xml_nm_, pos_s, conf_len);
	(*config_xml_nm_)[conf_len] = 0;

// mode
	*emu_mode_ = __DAFUTLT_MODE__;
	if ( pos_c != 0 )
	{
    char *mode = 0;
	     pos_s = pos_c + 1;
	     pos_c = strchr((char*)pos_s, ':');
	     mode_len =  (int)((pos_c == 0 )? strlen(pos_s) : (int)(pos_c - pos_s));
   	     mode = (char*)malloc(mode_len+1);
	     assert(mode);
         memcpy(mode, pos_s, mode_len);
	     mode[mode_len] = 0;
	     *emu_mode_ = (atoi(mode) > 0 );
		 free(mode);
	}
// temp file location
	if ( pos_c != 0 )
	{
	     pos_s = pos_c + 1;
	     pos_c = strchr((char*)pos_s, ':');
	     temp_len =  (int)((pos_c == 0 )? strlen(pos_s) : (int)(pos_c - pos_s));
		 if ( temp_len < 2 )
		 {
            *temp_file_loc_ =  find_directory(*config_xml_nm_);
		 }
		 else
		 {
   	        *temp_file_loc_ = (char*)malloc(temp_len+1);
	        assert(*temp_file_loc_);
            memcpy(*temp_file_loc_, pos_s, temp_len);
	        (*temp_file_loc_)[temp_len] = 0;
		 }
	}
// extract from xml 
	else
	{
       *temp_file_loc_ =  find_directory(*config_xml_nm_);
	}
// fix directory
	{
		char * temp_loc;
		if ( (temp_loc = fix_directory(*temp_file_loc_)) != NULL )
		{
			free(*temp_file_loc_);
			*temp_file_loc_ = temp_loc;
		}
	}

	return(0);
}

static
int getEnv( char ** env_, const char * env_var_nm_ )
{
 char  *env_root;
 size_t var_len;

#ifdef WIN32
    _dupenv_s(&env_root, &var_len, env_var_nm_);

#else
     env_root = getenv(env_var_nm_);
	 var_len = (env_root != NULL) ? strlen(env_root) : 0;
#endif
	if ( var_len == 0 )
	{
	   PIM_MSG("pim_emu error: cannot find PIMEMUENV env var\n" );
// FIXME!!!
//     usage();
	   exit(0);
	}

	*env_ = (char*)malloc(var_len+1);

	assert(*env_);
	strcpy(*env_,env_root);
	return(0);
}

static
void setCapturing(int val_);

#ifndef WIN32
__attribute__((constructor))
static
#endif
int pim_constructor ( void )
{
    char * env= 0;
    int capture_mode;
    int retval;

// in Linux the call to load the pthread library has been made from
// pthread_once function (see).


	pim_pthreadinterceptorInit();

	pim_cliterceptorInit();

    getEnv( &env, __PIMEMUENV__ );
    getEnvConfig( &config_xml,  &capture_mode,  &temp_file_loc, env );
	if ( env)
	{
		free(env);
	}

	setCapturing(capture_mode);
	if ((retval = pimemu_constructor()) == PIM_SUCCESS )
	{

       retval = pim_init(config_xml);
	}

    if (PIM_SUCCESS != retval) {
        fprintf(stderr, "Error initializing PIM system.\n");
        fprintf(stderr, "Failed with return value %d\n", retval);
        exit(-1);
    }

	return(0);
}

#ifndef WIN32
__attribute__((destructor))
static
#endif
int pim_destructor ( void )
{
	pimemu_destructor();

	removePIMFactory();

	if ( temp_file_loc )
	{
		free(temp_file_loc);
		temp_file_loc = 0;
	}
	if ( config_xml )
	{
		free (config_xml);
		config_xml = 0;
	}
	return(0);
}

/******************************************************************************/
/************* emulator data gathering ****************************************/
static
int startCounting( void );
static
int endCounting( void );

int pim_emu_begin(void)
{
int ret = PIM_SUCCESS;
	startCounting();
	pim_emu_begin_impl();
	ret = ( isThreadCounting() )? PIM_SUCCESS : PIM_GENERAL_ERROR;
	return(ret);
}

int pim_emu_end(void)
{
int ret = PIM_SUCCESS;
	pim_emu_end_impl();
	endCounting();
	ret = ( !isThreadCounting() )? PIM_SUCCESS : PIM_GENERAL_ERROR;
	return(ret);
}



/******************************************************************************/
/************   private *******************************************************/

#define __DEFAULT_TIMELINESTREAMING_FILE__ "pimemu_timelinedata.csv"
#define __DEFAULT_CPUCOUNTERSTREAMING_FILE__ "pimemu_cpucounterdata.csv"

static
int emu_mode = 1;
static
int counting = 0;
static
int capturing_mode = 0;
static
int cpu_counting_init = 0;
static
int capturing_init = 0;


static  char sTimelineStreamFileName[128] = {0};
static FILE * sTimelineStreamFile = 0;
static  char sCPUCountersStreamFileName[128] = {0};
static FILE * sCPUCountersStreamFile = 0;

static
int pimemuStreamFile( char * strea_nm_, FILE ** stream_file_, const char * name_ )
{
	if ( !name_ )
	{
         PIM_MSG("Unable open pimemu data gathering file\n");
		 return(-1);

	}
	
    strcpy(strea_nm_, name_);
	
	*stream_file_ = fopen(strea_nm_, "wt");

	if ( !(*stream_file_) )
	{
     char msg[256];
	     sprintf(msg,"Unable open pimemu data gathering file: %s\n", strea_nm_);
         PIM_MSG(msg);
		 return(-1);
	}

	return (0);
}

static
int pimemuOpenTimelineStreamFile( const char * name_ )
{
	pimemuStreamFile( sTimelineStreamFileName, &sTimelineStreamFile, name_ );
	return (0);
}


int pimemuInitTimelineStreaming(void)
{
 //   PIM_GENERAL_TRY(pthread_rwlock_wrlock(&pim_data_stream_lock),
 //      "Unable to write acquire PIM emu data gathering RWLock\n", PIM_GENERAL_ERROR);
	if ( !capturing_init )
	{
  char buf[1024];
       capturing_init = 1;
       strcpy(buf, getEmuTempFilesLocation());
	   strcat(buf, __DEFAULT_TIMELINESTREAMING_FILE__);
	   pimemuOpenTimelineStreamFile(buf);
	}
 //   PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_data_stream_lock),
  //      "Unable to unlock PIM emu data gathering RWLock\n");
	return(0);
}

int pimemuDeinitTimelineStreaming(void)
{
 //   PIM_GENERAL_TRY(pthread_rwlock_wrlock(&pim_data_stream_lock),
  //     "Unable to write acquire PIM emu data gathering RWLock\n", PIM_GENERAL_ERROR);
	if ( capturing_init )
	{
	   capturing_init = 0;
	   fclose(sTimelineStreamFile);
	   sTimelineStreamFile = 0;
	}
 //   PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_data_stream_lock),
 //       "Unable to unlock PIM emu data gathering RWLock\n");
	return(0);
}


static
int pimemuOpenCPUCountersStreamFile( const char * name_ )
{
	pimemuStreamFile( sCPUCountersStreamFileName, &sCPUCountersStreamFile, name_ );
	return (0);
}

int pimemuInitCPUCounterStreaming(void) //should be done only once at the start of a program and in the main thread
{
  int ret=1;
//    PIM_GENERAL_TRY(pthread_rwlock_wrlock(&pim_data_stream_lock),
//       "Unable to write acquire PIM emu data gathering RWLock\n", PIM_GENERAL_ERROR);
     if ( !cpu_counting_init )
	 {
		 cpu_counting_init = 1;
#ifndef WIN32
  char msg[1024];
       strcpy(msg, getEmuTempFilesLocation());
	   strcat(msg, __DEFAULT_CPUCOUNTERSTREAMING_FILE__);
       pimemuOpenCPUCountersStreamFile(msg);
   /*** Init Papi Library and register it to support pthreads **********/
       ret=init_perfCounters(NULL); 
       if( !ret)
	   {
           fprintf(stderr,"Could not init perf counters \n");
	   }
//	 fprintf(stderr,"Perf counters are initialized\n");
    // print the headers for the perf counters
      if(print_counterheader(msg,1024) == 0){
          fprintf(stderr,"buffer too small to printer CPU counter headers \n");
    }
    pimemuCPUCountersSendString(msg);
#endif
	 }
 //   PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_data_stream_lock),
  //      "Unable to unlock PIM emu data gathering RWLock\n");
	return(ret); //returns 1 on success and 0 on failuire
}

int pimemuDeinitCPUCounterStreaming(void) //should be done only once at the start of a program and in the main thread
{   
//    PIM_GENERAL_TRY(pthread_rwlock_wrlock(&pim_data_stream_lock),
//       "Unable to write acquire PIM emu data gathering RWLock\n", PIM_GENERAL_ERROR);
	 if ( cpu_counting_init )
	 {
		 cpu_counting_init = 0;
#ifndef WIN32
    // We shutdown PAPI counters on LAST PIM Call 
       shutdown_perfCounters();
	   fclose(sCPUCountersStreamFile);
	   sCPUCountersStreamFile = 0;
#endif
     }
//    PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_data_stream_lock),
 //       "Unable to unlock PIM emu data gathering RWLock\n");
     return(0);
}


/******************************************************************************/
/************   external *******************************************************/

int startCounting( void )
{
int ret = counting;
	counting = 1;
	return(ret);
}

int endCounting( void )
{
int ret = counting;
    counting = 0;
	return(ret);
}

int isCountingMode( void )
{
	return(counting);
}


int isCounting( void )
{
	return(isCountingMode());
}

static
int getEmuMode(void)
{
	return(emu_mode);
}

int isEmuMode(void)
{
	return( getEmuMode() & isEmuInitialized());
}

static
void setCapturing(int val_)
{
	capturing_mode = val_;
}
	

static
int getCapturing(void)
{
	return(capturing_mode);
}
int isCapturing(void)
{
	return(getCapturing() && isEmuMode());
}

int pimemuSendString(FILE * file_, const char* string_)
{
    PIM_GENERAL_TRY(pthread_rwlock_wrlock(&pim_data_stream_lock),
        "Unable to write acquire PIM emu data gathering RWLock\n", PIM_GENERAL_ERROR);
	if (file_)
	{
		fputs(string_, file_);
		fflush(file_);
	}
    PIM_EXIT_TRY(pthread_rwlock_unlock(&pim_data_stream_lock),
        "Unable to unlock PIM emu data gathering RWLock\n");
	return(0);
}

int pimemuTimelineSendString(const char* string_)
{
// send messages we stacked before
	if ( isCapturing() )
	{
	    sendStackedTimeLineMsg(sTimelineStreamFile);
		if ( string_ )
		{
	       pimemuSendString(sTimelineStreamFile, string_);
		}
	}
	return(0);
}


int pimemuCPUCountersSendString(const char* string_)
{
	if ( isCapturing() )
	{

    	pimemuSendString(sCPUCountersStreamFile, string_);
	}
	return(0);

}
