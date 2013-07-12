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

#include "xml_parse.h"

int parse_pim_config_from_filename(char *xml_file, pim_emu_configuration_t *dest_configuration)
{
    int failure = 0;
    int xml_filed = open(xml_file, O_RDONLY);
    if (xml_filed < 0)
        return PIM_INIT_FILE_ACCESS_ERROR;
    failure = parse_pim_config(xml_filed, dest_configuration);
    return failure;
}

int parse_pim_config(int fd, pim_emu_configuration_t *dest_configuration)
{
    int failure = 0;
    node_t *root_node;
    root_node = roxml_load_fd(fd);
    if (root_node == NULL)
        return PIM_INIT_FILE_SYNTAX_ERROR;

    failure = parse_ehp_internals(root_node, &(dest_configuration->ehp));
    if (failure != PIM_SUCCESS) {
        roxml_close(root_node);
        return PIM_INIT_FILE_SYNTAX_ERROR;
    }

    failure = count_and_create_pims(root_node, &(dest_configuration->num_pim_cores), &(dest_configuration->pims));
    roxml_close(root_node);
    if (failure != PIM_SUCCESS)
        return PIM_INIT_FILE_SYNTAX_ERROR;

    return PIM_SUCCESS;
}

uint32_t get_num_xml_nodes(node_t *device, char *search_string)
{
    node_t *node_walker;
    uint32_t num_nodes = 0;
    int num_children, i;
    char *node_name;

    // Total number of XML children nodes for this device.
    // May be comments, etc. So not guaranteed to be descriptions of cores.
    num_children = roxml_get_chld_nb(device);
    node_walker = roxml_get_chld(device, NULL, 0);
    for (i = 0; i < num_children; i++) {
        if (node_walker == NULL) {
            fprintf(stderr,
                "%s (%d): Somehow, parsing for %s has yielded %d children, but the list stopped after %d\n",
                __func__, __LINE__, search_string, num_children, i);
        }
        if (roxml_get_type(node_walker) == ROXML_ELM_NODE) {
            node_name = roxml_get_name(node_walker, NULL, 0);
            if (node_name != NULL && (strcmp(node_name, search_string) == 0))
                num_nodes++;
            roxml_release(node_name);
        }
        node_walker = roxml_get_next_sibling(node_walker);
    }
    return num_nodes;
}

uint32_t get_num_cpus(node_t *device)
{
    return get_num_xml_nodes(device, (char*)"cpu_core");
}

uint32_t get_num_gpus(node_t *device)
{
    return get_num_xml_nodes(device, (char*)"gpu_core");
}

uint32_t get_num_dram(node_t *device)
{
    return get_num_xml_nodes(device, (char*)"dram");
}

uint32_t get_num_pims(node_t *device)
{
    return get_num_xml_nodes(device, (char*)"pim");
}

int count_and_create_pims(node_t *device, uint32_t *num_pims, pim_configuration_t **pims)
{
    uint32_t num_cores = get_num_pims(device);
    *num_pims = num_cores;
    *pims = (pim_configuration_t *)calloc(num_cores, sizeof(pim_configuration_t));
    if ((*pims) != NULL)
        return find_and_parse_pims(device, *pims);
    else if (*num_pims == 0)
        return PIM_SUCCESS;
    else {
        fprintf(stderr, "%s (%d): calloc() failed with error: %s\n",
                __func__, __LINE__, strerror(errno));
        return PIM_GENERAL_ERROR;
    }
}

int count_and_create_cpus(node_t *device, uint32_t *num_cpus, cpu_configuration_t **cpus)
{
    uint32_t num_cores = get_num_cpus(device);
    *num_cpus = num_cores;
    if (*num_cpus == 0)
        return PIM_SUCCESS;
    *cpus = (cpu_configuration_t *)calloc(num_cores, sizeof(cpu_configuration_t));
    if ((*cpus) != NULL)
        return find_and_parse_cpus(device, *cpus);
    else {
        fprintf(stderr, "%s (%d): calloc() failed with error: %s\n",
                __func__, __LINE__, strerror(errno));
        return PIM_GENERAL_ERROR;
    }
}

int count_and_create_gpus(node_t *device, uint32_t *num_gpus, gpu_configuration_t **gpus)
{
    uint32_t num_cores = get_num_gpus(device);
    *num_gpus = num_cores;
    *gpus = (gpu_configuration_t *)calloc(num_cores, sizeof(gpu_configuration_t));
    if ((*gpus) != NULL)
        return find_and_parse_gpus(device, *gpus);
    else if (*num_gpus == 0)
        return PIM_SUCCESS;
    else {
        fprintf(stderr, "%s (%d): calloc() failed with error: %s\n",
                __func__, __LINE__, strerror(errno));
        return PIM_GENERAL_ERROR;
    }
}

int count_and_create_dram(node_t *device, uint32_t *num_dram, dram_configuration_t **dram)
{
    uint32_t num_cores = get_num_dram(device);
    *num_dram = num_cores;
    *dram = (dram_configuration_t *)calloc(num_cores, sizeof(dram_configuration_t));
    if ((*dram) != NULL)
        return find_and_parse_dram(device, *dram);
    else if (*num_dram == 0)
        return PIM_SUCCESS;
    else {
        fprintf(stderr, "%s (%d): calloc() failed with error: %s\n",
                __func__, __LINE__, strerror(errno));
        return PIM_GENERAL_ERROR;
    }
}

int find_and_parse_cpus(node_t *device, cpu_configuration_t *cpus)
{
    int i;
    int current_cpu = 0;
    int num_children = roxml_get_chld_nb(device);
    node_t *node_walker = roxml_get_chld(device, NULL, 0);
    for (i = 0; i < num_children; i++) {
        // Find each CPU
        if (roxml_get_type(node_walker) == ROXML_ELM_NODE) {
            int return_val = 0;
            char *node_name = roxml_get_name(node_walker, NULL, 0);
            if (node_name != NULL && (strcmp(node_name, "cpu_core") == 0)) {
		memset(&cpus[current_cpu], 0, sizeof(cpu_configuration_t));
		cpus[current_cpu].id=current_cpu;
                return_val = parse_cpu_internals(node_walker, &((cpus)[current_cpu]));
                current_cpu++;
                if (return_val != PIM_SUCCESS)
                    return return_val;
            }
            roxml_release(node_name);
        }
        node_walker = roxml_get_next_sibling(node_walker);
    }
    return PIM_SUCCESS;
}

int find_and_parse_gpus(node_t *device, gpu_configuration_t *gpus)
{
    int i;
    int current_gpu = 0;
    int num_children = roxml_get_chld_nb(device);
    node_t *node_walker = roxml_get_chld(device, NULL, 0);
    for (i = 0; i < num_children; i++) {
        // Find each GPU 
        if (roxml_get_type(node_walker) == ROXML_ELM_NODE) {
            int return_val = 0;

            char *node_name = roxml_get_name(node_walker, NULL, 0);
            if (node_name != NULL && (strcmp(node_name, "gpu_core") == 0)) {
				memset(&gpus[current_gpu], 0, sizeof(gpu_configuration_t));
				(gpus)[current_gpu].id = current_gpu;
                return_val = parse_gpu_internals(node_walker, &((gpus)[current_gpu]));
               // default propetiers non-visible to a user right now 
                current_gpu++;
                if (return_val != PIM_SUCCESS)
                    return return_val;
            }
            roxml_release(node_name);
        }
        node_walker = roxml_get_next_sibling(node_walker);
    }
    return PIM_SUCCESS;
}

int find_and_parse_dram(node_t *device, dram_configuration_t *dram)
{
    int i;
    int current_dram = 0;
    int num_children = roxml_get_chld_nb(device);
    node_t *node_walker = roxml_get_chld(device, NULL, 0);
    for (i = 0; i < num_children; i++) {
        // Find each DRAM
        if (roxml_get_type(node_walker) == ROXML_ELM_NODE) {
            int return_val = 0;
            char *node_name = roxml_get_name(node_walker, NULL, 0);
            if (node_name != NULL && (strcmp(node_name, "dram") == 0)) {
				memset(&dram[current_dram], 0, sizeof(dram_configuration_t));
				(dram)[current_dram].id = current_dram;
                return_val = parse_dram_internals(node_walker, &((dram)[current_dram]));
                current_dram++;
                if (return_val != PIM_SUCCESS)
                    return return_val;
            }
            roxml_release(node_name);
        }
        node_walker = roxml_get_next_sibling(node_walker);
    }
    return PIM_SUCCESS;
}

int find_and_parse_pims(node_t *device, pim_configuration_t *pims)
{
    int i;
    int current_pim = 0;
    int num_children = roxml_get_chld_nb(device);
    node_t *node_walker = roxml_get_chld(device, NULL, 0);
    
    // For Each PIM, it's time to fill in the internals
    for (i = 0; i < num_children; i++) {
        // Find each PIM
        if (roxml_get_type(node_walker) == ROXML_ELM_NODE) {
            int return_val = 0;
            char *node_name = roxml_get_name(node_walker, NULL, 0);
            if (node_name != NULL && (strcmp(node_name, "pim") == 0)) {
				pims[current_pim].id = current_pim;
                return_val = parse_pim_internals(node_walker, &(pims[current_pim]));
                current_pim++;
                if (return_val != PIM_SUCCESS)
                    return return_val;
            }
            roxml_release(node_name);
        }
        node_walker = roxml_get_next_sibling(node_walker);
    }
    return PIM_SUCCESS;
}

int parse_cpu_internals(node_t *cpu_node, cpu_configuration_t *dest_config)
{
    int i;
    int num_children = roxml_get_chld_nb(cpu_node);
    node_t *config_walker = roxml_get_chld(cpu_node, NULL, 0);
    char *content, *node_name;
    for (i = 0; i < num_children; i++) {
        if (roxml_get_type(config_walker) == ROXML_ELM_NODE) {
            node_name = roxml_get_name(config_walker, NULL, 0);
            if (node_name != NULL) {
                content = roxml_get_content(config_walker, NULL, 0, NULL);
                if ((strcmp(node_name, "isa") == 0)) {
                    if (strcmp(content, "x86-64") == 0)
                        dest_config->isa = ISA_X86_64;
                    else if (strcmp(content, "arm") == 0)
                        dest_config->isa = ISA_ARM;
                    // Undefined ISA: will stay at zero and will return a syntax error later.
                }

                else if ((strcmp(node_name, "ordering") == 0)) {
                    if ((strcmp(content, "inorder") == 0))
                        dest_config->ordering = ORDERING_INORDER;
                    else if (strcmp(content, "ooo") == 0)
                        dest_config->ordering = ORDERING_OOO;
                }
                else if ((strcmp(node_name, "freq") == 0)) {
                    int frequency = atoi(content);
                    dest_config->freq = (uint32_t)frequency;
                }
                else if ((strcmp(node_name, "width") == 0)) {
                    int width = atoi(content);
                    if (width < 256)
                        dest_config->width = (uint8_t)width;
                    // Otherwise it's too big, we leave width at zero and syntax error later
                }
                else if ((strcmp(node_name, "num_hwthreads") == 0)) {
                    int num_threads = atoi(content);
                    if (num_threads < 256)
                        dest_config->num_hwthreads = (uint8_t)num_threads;
                }
                roxml_release(content);
            }
            roxml_release(node_name);
        }
        config_walker = roxml_get_next_sibling(config_walker);
    }
    if ((dest_config->freq == 0) || (dest_config->num_hwthreads  == 0))
        return PIM_INIT_FILE_SYNTAX_ERROR;
    return PIM_SUCCESS;
}

int parse_gpu_internals(node_t *gpu_node, gpu_configuration_t *dest_config)
{
    int i;
    int num_children = roxml_get_chld_nb(gpu_node);
    node_t *config_walker = roxml_get_chld(gpu_node, NULL, 0);
    char *content, *node_name;
    for (i = 0; i < num_children; i++) {
        if (roxml_get_type(config_walker) == ROXML_ELM_NODE) {
            node_name = roxml_get_name(config_walker, NULL, 0);
            if (node_name != NULL) {
                content = roxml_get_content(config_walker, NULL, 0, NULL);
                if ((strcmp(node_name, "isa") == 0)) {
                    if (strcmp(content, "si") == 0)
                        dest_config->isa = ISA_SI;
                    // Undefined ISA: will stay at zero and will return a syntax error later.
                }
                else if ((strcmp(node_name, "freq") == 0)) {
                    int frequency = atoi(content);
                    dest_config->freq = (uint32_t)frequency;
                }
                else if ((strcmp(node_name, "cus") == 0)) {
                    int cus = atoi(content);
                    dest_config->cus = (uint32_t)cus;
                }
                else if ((strcmp(node_name, "width") == 0)) {
                    int  width= atoi(content);
                    dest_config->width = (uint32_t)width;
                }
                else if ((strcmp(node_name, "num_instr_perclck") == 0)) {
                    int  ipc= atoi(content);
                    dest_config->num_instr_perclck = (uint32_t)ipc;
                }
                roxml_release(content);
            }
            roxml_release(node_name);
        }
        config_walker = roxml_get_next_sibling(config_walker);
    }
    if ( (dest_config->cus == 0)||
        (dest_config->freq == 0))
        return PIM_INIT_FILE_SYNTAX_ERROR;
    return PIM_SUCCESS;
}

int parse_dram_internals(node_t *dram_node, dram_configuration_t *dest_config)
{
    int i;
    int num_children = roxml_get_chld_nb(dram_node);
    node_t *config_walker = roxml_get_chld(dram_node, NULL, 0);
    char *content, *node_name;
    for (i = 0; i < num_children; i++) {
        if (roxml_get_type(config_walker) == ROXML_ELM_NODE) {
            node_name = roxml_get_name(config_walker, NULL, 0);
            if (node_name != NULL) {
                content = roxml_get_content(config_walker, NULL, 0, NULL);
                if ((strcmp(node_name, "size") == 0)) {
                    int size = atoi(content);
                    dest_config->size_in_mb = (uint32_t)size;
                }
                else if ((strcmp(node_name, "off_stack_bw") == 0)) {
                    int bw = atoi(content);
                    dest_config->off_stack_bw = (uint32_t)bw;
                }
                else if ((strcmp(node_name, "between_pim_bw") == 0)) {
                    int bw = atoi(content);
                    dest_config->between_pim_bw = (uint32_t)bw;
                }
                else if ((strcmp(node_name, "freq") == 0)) {
                    int freq = atoi(content);
                    dest_config->freq = (uint32_t)freq;
                }
                else if ((strcmp(node_name, "num_channels") == 0)) {
                    int channels = atoi(content);
                    dest_config->num_channels = (uint32_t)channels;
                }
                else if ((strcmp(node_name, "channel_width") == 0)) {
                    int width = atoi(content);
                    dest_config->channel_width = (uint32_t)width;
                }
                else if ((strcmp(node_name, "bits_percycle") == 0)) {
                    int bits = atoi(content);
                    dest_config->bits_percycle = (uint32_t)bits;
                }
                else if ((strcmp(node_name, "latency") == 0)) {
                    int latency = atoi(content);
                    dest_config->latency = (uint32_t)latency;
                }
                else if ((strcmp(node_name, "off_stack_latency") == 0)) {
                    int latency = atoi(content);
                    dest_config->off_stack_latency = (uint32_t)latency;
                }
                else if ((strcmp(node_name, "utilization") == 0)) {
                    int utilization = atoi(content);
                    dest_config->utilization = (uint32_t)utilization;
                }
                roxml_release(content);
            }
            roxml_release(node_name);
        }
        config_walker = roxml_get_next_sibling(config_walker);
    }
    if ((dest_config->freq == 0) || (dest_config->num_channels == 0) || (dest_config->off_stack_bw == 0)
		|| ( dest_config->channel_width == 0))
        return PIM_INIT_FILE_SYNTAX_ERROR;
    return PIM_SUCCESS;
}

int parse_pim_internals(node_t *pim_xml_device, pim_configuration_t *pim_config)
{
    int i, return_val;
    int num_children = roxml_get_chld_nb(pim_xml_device);
    node_t *node_walker = roxml_get_chld(pim_xml_device, NULL, 0);
    char *content, *node_name;

    // Set up the PIM CPU(s)
    return_val = count_and_create_cpus(pim_xml_device, &(pim_config->num_cpu_cores), &(pim_config->cpus));
    if (return_val != PIM_SUCCESS)
        return return_val;

    // Set up the PIM GPU(s)
    return_val = count_and_create_gpus(pim_xml_device, &(pim_config->num_gpu_cores), &(pim_config->gpus));
    if (return_val != PIM_SUCCESS)
        return return_val;

    // Set up the DRAM(s) attached to this PIM
    return_val = count_and_create_dram(pim_xml_device, &(pim_config->num_dram_stacks), &(pim_config->dram));
    if (return_val != PIM_SUCCESS)
        return return_val;

    // Set up the job launch and completion times associated with this PIM
    pim_config->ns_to_launch_to_pim = 1000; // Default to 1 microsecond
    pim_config->ns_to_complete_from_pim = 1000; // Default to 1 microsecond
    for (i = 0; i < num_children; i++) {
       if (roxml_get_type(node_walker) == ROXML_ELM_NODE) {
            node_name = roxml_get_name(node_walker, NULL, 0);
            if (node_name != NULL) {
                content = roxml_get_content(node_walker, NULL, 0, NULL);
                if ((strcmp(node_name, "job_start_time_ns") == 0)) {
                    unsigned long long start_time;
#ifdef WIN32
                    start_time = _strtoui64(content, NULL, 0);
#else
                    start_time = strtoull(content, NULL, 0);
#endif
                    pim_config->ns_to_launch_to_pim = (uint64_t)start_time;
                }
                else if ((strcmp(node_name, "job_end_time_ns") == 0)) {
                    unsigned long long end_time;
#ifdef WIN32
                    end_time = _strtoui64(content, NULL, 0);
#else
                    end_time = strtoull(content, NULL, 0);
#endif
                    pim_config->ns_to_complete_from_pim = (uint64_t)end_time;
                }
                roxml_release(content);
            }
            roxml_release(node_name);
        }
        node_walker = roxml_get_next_sibling(node_walker);
    }
    return PIM_SUCCESS;
}

int parse_ehp_internals(node_t *root_node, ehp_configuration_t *dest_ehp_config)
{
    node_t *device; // First layer in this DTD is always devices. EHP, PIMs, or NVMs.
    int num_children, return_val;

    num_children = roxml_get_chld_nb(root_node);
    if (num_children < 1) // Must have at least a single EHP
        return PIM_INIT_FILE_SYNTAX_ERROR;
    device = roxml_get_chld(root_node, (char*)"ehp", 0);
    if (device == NULL) // Must have an EHP
        return PIM_INIT_FILE_SYNTAX_ERROR;

    num_children = roxml_get_chld_nb(device);
    if (num_children < 1) // No CPU or GPU description
        return PIM_INIT_FILE_SYNTAX_ERROR;
    // Set up the EHP CPU(s)
    return_val = count_and_create_cpus(device, &(dest_ehp_config->num_cpu_cores), &(dest_ehp_config->cpus));
    if (return_val != PIM_SUCCESS)
        return return_val;
    if (dest_ehp_config->num_cpu_cores < 1) // EHP must have a CPU
        return PIM_INIT_FILE_SYNTAX_ERROR;

    // Set up the EHP GPU(s)
    return count_and_create_gpus(device, &(dest_ehp_config->num_gpu_cores), &(dest_ehp_config->gpus));
}
