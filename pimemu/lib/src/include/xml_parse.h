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

#ifndef __LIB_PIM_XML_PARSE_H_
#define __LIB_PIM_XML_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#include <stdint.h>
#include <roxml.h>

#include "pim-private.h"
#include "pim.h"

/*****************************************************************************/

/* int parse_pim_config_from_filename()
 * Initialize a pim_emu_configuration_t structure based off an XML file that uses the
 * PIM DTD to describe the system being emulated and other runtime information.
 * This, or the other parse_pim_config* function, will likely be the only thing you
 * call from this file.
 * Arguments:
 *      char *xml_file
 *          Input
 *          Path and filename for the XML File used to initialize this PIM emulation instance
 *          This file will be opened READ ONLY and parsed by the functions in this code.
 *      pim_emu_configuration_t *dest_configuration
 *          Output
 *          Pointer to an already existing pim_emu_configuration_t that will be filled in
 *          with the data parsed out of the XML file.
 * Returns:
 *      int
 *          PIM_SUCCESS if no problems
 *          PIM_INIT_FILE_ACCESS_ERROR if accessing the file runs into difficulties
 *          PIM_INIT_FILE_SYNTAX_ERROR if the XML file has an issue
 */
int parse_pim_config_from_filename(char *xml_file, pim_emu_configuration_t *dest_configuration);

/* int parse_pim_config()
 * Initialize a pim_emu_configuration_t structure based off an XML file that uses the
 * PIM DTD to describe the system being emulated and other runtime information.
 * This will likely be the only function you would call from this file.
 * Arguments:
 *      int fd
 *          Input
 *          Open file descriptor pointing to the XML File used to initialize this PIM
 *          emulation instance.
 *          This file will be opened READ ONLY and parsed by the functions in this code.
 *      pim_emu_configuration_t *dest_configuration
 *          Output
 *          Pointer to an already existing pim_emu_configuration_t that will be filled in
 *          with the data parsed out of the XML file.
 * Returns:
 *      int
 *          PIM_SUCCESS if no problems
 *          PIM_INIT_FILE_ACCESS_ERROR if accessing the file runs into difficulties
 *          PIM_INIT_FILE_SYNTAX_ERROR if the XML file has an issue
 */
int parse_pim_config(int fd, pim_emu_configuration_t *dest_configuration);

/* uint32_t get_num_xml_nodes()
 * Returns the number of children nodes whose name matches a particular string.
 * Useful for knowing how large of a structure to allocate to hold all parsed out data.
 * Arguments:
 *      node_t *device
 *          Input
 *          Parent XML node. All of the direct children of this XML node will be checked.
 *      char *search_string
 *          Input
 *          String that each child's name will be compared against.
 * Returns:
 *      uint32_t
 *          The number of children of *device that directly match *search-string
 */
uint32_t get_num_xml_nodes(node_t *device, char *search_string);

/* uint32_t get_num_{device}
 * Wrapper functions for get_num_xml_nodes()
 * Search for the number of CPU, GPU, DRAM, or PIM devices directly below the input node.
 * Arguments:
 *      node_t *device
 *      Input
 *      Parent XML node. All of the direct children of this XML node will be checked.
 * Returns:
 *      uint32_t
 *          The number of particular devices (defined by the particular function) that were
 *          found below the *device node.
 */
uint32_t get_num_cpus(node_t *device);
uint32_t get_num_gpus(node_t *device);
uint32_t get_num_drams(node_t *device);
uint32_t get_num_pims(node_t *device);

/* int count_and_create_{device}
 * Takes a parent node, finds the number of specified devices directly below it, and
 * creates + fills in the configuration structure for that device.
 * Arguments:
 *      node_t *device
 *          Input
 *          Parent XML node. All of the direct children of this XML node will be parsed.
 *      uint32_t *num_{device}
 *          Output
 *          This will return the number of devices of the specified type found under the parent
 *      {device}_configuration_t **{device}
 *          Output
 *          An array of configuration structures of the correct size will be allocated
 *          within this function and the values will be parsed out from the XML file.
 * Returns:
 *      int
 *          PIM_SUCCESS if no problems
 *          PIM_INIT_FILE_SYNTAX_ERROR if the XML file has an issue
 */
int count_and_create_cpus(node_t *device, uint32_t *num_cpus, cpu_configuration_t **cpus);
int count_and_create_gpus(node_t *device, uint32_t *num_gpus, gpu_configuration_t **gpus);
int count_and_create_dram(node_t *device, uint32_t *num_dram, dram_configuration_t **dram);
int count_and_create_pims(node_t *device, uint32_t *num_pims, pim_configuration_t **pims);

/* int find_and_parse_{device}
 * Useful after count_and_create. This finds every node of the specified type below the
 * inputted parent node. It then calls the actual XML parse on that node to get the
 * specified values into the configuration structure.
 * Arguments:
 *      node_t *device
 *          Input
 *          Parent XML node. All of the direct children of this XML node will be parsed.
 *      {device}_configuration_t **{device}
 *          Output
 *          An array of configuration structures that will hold the parsed out config values
 * Returns:
 *      int
 *          PIM_SUCCESS if no problems
 *          PIM_INIT_FILE_SYNTAX_ERROR if the XML file has an issue
 */
int find_and_parse_cpus(node_t *device, cpu_configuration_t *cpus);
int find_and_parse_gpus(node_t *device, gpu_configuration_t *gpus);
int find_and_parse_dram(node_t *device, dram_configuration_t *dram);
int find_and_parse_pims(node_t *device, pim_configuration_t *pims);

/* int parse_{device}_internals
 * Perform the parsing on the specified type of device. For things like EHPs and PIMs, which
 * are made up of many inner devices, this may end up calling into count_and_create for
 * things like CPUs and GPUs. Otherwise, it will read the XML data out for the lowest-level
 * devices and fill out the details in a configuration structure.
 * Arguments:
 *      node_t *device
 *          Input
 *          XML node that holds the data about this particular device
 *      {device}_configuration_t **{device}
 *          Output
 *          An array of configuration structures that will hold the parsed out config values
 * Returns:
 *      int
 *          PIM_SUCCESS if no problems
 *          PIM_INIT_FILE_SYNTAX_ERROR if the XML file has an issue
 */
int parse_cpu_internals(node_t *cpu_node, cpu_configuration_t *dest_config);
int parse_gpu_internals(node_t *gpu_node, gpu_configuration_t *dest_config);
int parse_dram_internals(node_t *dram_node, dram_configuration_t *dest_config);
int parse_pim_internals(node_t *pim_xml_device, pim_configuration_t *pim_config);
int parse_ehp_internals(node_t *root_node, ehp_configuration_t *dest_ehp_config);


#ifdef __cplusplus
}
#endif

#endif  /* __LIB_PIM_PRIVATE_H_ */
