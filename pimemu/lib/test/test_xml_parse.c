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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "xml_parse.h"

#define FAIL_IFN(item, value)                               \
do {                                                        \
    if (item != value)                                      \
    {                                                       \
        printf("Failed!\n\n");                              \
        fflush(stdout);                                     \
        fprintf(stderr, #item" (%d) != "#value"\n", item);  \
        failure = -1;                                       \
    }                                                       \
} while(0)

pim_emu_configuration_t test_configuration;

char *concat_directory_and_name(const char *directory, const char *name)
{
    size_t directory_length, name_length;
    char* return_str;

    // Directory is guaranteed to have a slash after it.
    directory_length = strlen(directory);
    name_length = strlen(name);

    return_str = malloc(sizeof(char) * (directory_length + name_length + 1));
    if (return_str == NULL) {
        fprintf(stderr, "Unable to allocate Directory+Name Concat. Die.\n");
        exit(-1);
    }
    return_str[(directory_length + name_length)] = '\0';
    strcpy(return_str, directory);
    strcpy((return_str+directory_length), name);
    return return_str;
}

int first_test(char *directory)
{
    int xml_file, failure;
    char *filename;
    int param;
    printf("**************************************************\n");
    printf("Beginning XML Parsing Test\n");
    printf("\t1. Testing Good XML (1.xml): ");
    filename = concat_directory_and_name(directory, "1.xml");
    xml_file = open(filename, O_RDONLY);
    free(filename);
    if (xml_file < 0) {
        int error_number = errno;
        fprintf(stderr, "\nError accessing the file %s\n", filename);
        fprintf(stderr, "%s\n", strerror(error_number));
        return PIM_INIT_FILE_ACCESS_ERROR;
    }

    failure = parse_pim_config(xml_file, &test_configuration);
    // Parse also closes the files by doing a roxml_close
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "\nParsing failed with error code %d\n", failure);
        return -1;
    }

    // Manual translation of the test XML file.
    failure = 0;
    param = 0;

    FAIL_IFN(test_configuration.ehp.num_cpu_cores, 16);
    for (param = 0; param < 16; param++) {
        FAIL_IFN(test_configuration.ehp.cpus[param].freq, 4000);
        FAIL_IFN(test_configuration.ehp.cpus[param].num_hwthreads, 1);
        FAIL_IFN(test_configuration.ehp.cpus[param].isa, ISA_X86_64);
        FAIL_IFN(test_configuration.ehp.cpus[param].width, 4);
        FAIL_IFN(test_configuration.ehp.cpus[param].ordering, ORDERING_OOO);
    }

    FAIL_IFN(test_configuration.ehp.num_gpu_cores, 1);
    FAIL_IFN(test_configuration.ehp.gpus[0].freq, 1000);
    FAIL_IFN(test_configuration.ehp.gpus[0].cus, 192);
    FAIL_IFN(test_configuration.ehp.gpus[0].isa, ISA_SI);

    FAIL_IFN(test_configuration.num_pim_cores, 1);
    FAIL_IFN(test_configuration.pims[0].num_cpu_cores, 2);
    for (param = 0; param < 2; param++) {
        FAIL_IFN(test_configuration.pims[0].cpus[param].freq, 2000);
        FAIL_IFN(test_configuration.pims[0].cpus[param].num_hwthreads, 1);
        FAIL_IFN(test_configuration.pims[0].cpus[param].isa, ISA_X86_64);
        FAIL_IFN(test_configuration.pims[0].cpus[param].width, 2);
        FAIL_IFN(test_configuration.pims[0].cpus[param].ordering, ORDERING_OOO);
    }
    
    FAIL_IFN(test_configuration.pims[0].num_gpu_cores, 1);
    FAIL_IFN(test_configuration.pims[0].gpus[0].freq, 800);
    FAIL_IFN(test_configuration.pims[0].gpus[0].cus, 16);
    FAIL_IFN(test_configuration.pims[0].gpus[0].isa, ISA_SI);
    
    FAIL_IFN(test_configuration.pims[0].num_dram_stacks, 1);
    FAIL_IFN(test_configuration.pims[0].dram[0].size_in_mb, 4096);
    FAIL_IFN(test_configuration.pims[0].dram[0].freq, 2000);
    FAIL_IFN(test_configuration.pims[0].dram[0].num_channels, 8);
    FAIL_IFN(test_configuration.pims[0].dram[0].channel_width, 256);
    FAIL_IFN(test_configuration.pims[0].dram[0].bits_percycle, 1);
    FAIL_IFN(test_configuration.pims[0].dram[0].latency, 100);
    FAIL_IFN(test_configuration.pims[0].dram[0].utilization, 80);
    FAIL_IFN(test_configuration.pims[0].dram[0].off_stack_bw, 25);
    FAIL_IFN(test_configuration.pims[0].dram[0].between_pim_bw, 25);

    FAIL_IFN(test_configuration.pims[0].ns_to_launch_to_pim, 1000);
    FAIL_IFN(test_configuration.pims[0].ns_to_complete_from_pim, 1000);

    if (failure != PIM_SUCCESS) {
        exit(-1);
    }
    printf("Passed\n");
    return failure;
}

int second_test(char *directory)
{
    int xml_file, failure;
    char *filename;

    printf("\t2. Testing XML Missing CPU info (bad1.xml): ");
    filename = concat_directory_and_name(directory, "bad1.xml");
    xml_file = open(filename, O_RDONLY);
    if (xml_file < 0) {
        int error_number = errno;
        fprintf(stderr, "\nError accessing the file %s\n", filename);
        fprintf(stderr, "%s\n", strerror(error_number));
        free(filename);
        return PIM_INIT_FILE_ACCESS_ERROR;
    }
    free(filename);

    failure = parse_pim_config(xml_file, &test_configuration);
    // Parse also closes the files by doing a roxml_close
    if (failure != PIM_INIT_FILE_SYNTAX_ERROR) {
        fprintf(stderr, "\nParsing failed with error code %d\n", failure);
        return -1;
    }
    printf("\tPassed\n");
    return PIM_SUCCESS;
}

int main(int argc, char **argv)
{
    char *directory;
    size_t directory_length;
    int failure = 0;

    if (argc < 2) {
        fprintf(stderr, "XML Parser Tester requires a directory to search for XML tests as its argument.\n");
        exit(-1);
    }

    directory_length = strlen(argv[1]);

    if (argv[1][directory_length-1] != '/')
        directory_length += 1;

    directory = malloc(sizeof(char) * (directory_length + 1));
    if (directory == NULL) {
        fprintf(stderr, "Unable to allocate directory string. Die\n");
        exit(-1);
    }
    directory[directory_length] = '\0';

    strcpy(directory, argv[1]);
    if (argv[1][directory_length] != '/')
        directory[directory_length-1] = '/';

    failure = first_test(directory);
    if (failure != PIM_SUCCESS)
        return failure;
    failure = second_test(directory);
    free(directory);
    if (failure != PIM_SUCCESS)
        return failure;
    printf("All XML Parsing Tests Succeeded\n");
    return 0;
}

