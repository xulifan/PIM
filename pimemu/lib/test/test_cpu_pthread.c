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

/* Parallel Prefix Sum ported to run on PIM CPUs */
#include <stdlib.h>
#include <stdio.h>
#ifdef EMULATE_PTHREADS
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include <pim.h>
#include <string.h>

#define ARRAY_SIZE 100000000

// The map from the list of threads (sequential list) to PIM device ID.
pim_device_id* list_of_pims;

/*****************************************************************************/
/********** Functions to find correct file to send to pim_init() *************/
/*****************************************************************************/
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

void initialize_pim(char *filename)
{

    int failure = PIM_SUCCESS;
#if 0
    failure = pim_init(filename);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): Failed to initialize PIM emulator.\n",
                __func__, __LINE__);
        if (failure == PIM_INIT_FILE_ACCESS_ERROR) {
            fprintf(stderr, "XML file access error.\n");
        }
        else if (failure == PIM_INIT_FILE_SYNTAX_ERROR) {
            fprintf(stderr, "XML file syntax error\n");
        }
        else { // failure == PIM_GENERAL_FAILURE
            fprintf(stderr, "General error\n");
        }
        exit(-1);
    }
#endif
}
/*****************************************************************************/
/********* End Functions to find correct file to send to pim_init() **********/
/*****************************************************************************/


// Use PIM API discovery functions to find the number of PIMs within the system
// As an exercise, this also finds the number of CPUs contained within those
// PIMs. Currently, we only launch one thread per PIM, however, so that is
// what this function returns.
unsigned int find_and_map_pims()
{
    int i;
    int failure = 0;
    uint32_t num_pims = 0;
    unsigned int number_of_pim_cpus = 0;
    void *device_info;
    size_t needed_size;
    failure = pim_get_device_ids(PIM_CLASS_0, 0, NULL, &num_pims);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): pim_get_device_ids failed(%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }
    list_of_pims = malloc(num_pims * sizeof(pim_device_id));
    failure = pim_get_device_ids(PIM_CLASS_0, num_pims, list_of_pims, NULL);
    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): pim_get_device_ids failed(%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }

    for (i = 0; i < num_pims; i++) {
        failure = pim_get_device_info(list_of_pims[i], PIM_CPU_CORES, 0, NULL, &needed_size);
        if (failure != PIM_SUCCESS) {
            fprintf(stderr,
                    "%s (%d): pim_get_device_info failed (%d)\n",
                    __func__, __LINE__, failure);
            exit(-1);
        }
        device_info = malloc(needed_size);
        if (device_info == NULL) {
            fprintf(stderr, "%s (%d): Out of memory for size %" PRIuMAX
                    ". Die.\n", __func__, __LINE__, (uintmax_t)needed_size);
            exit(-1);
        }
        failure = pim_get_device_info(list_of_pims[i], PIM_CPU_CORES,
                needed_size, device_info, NULL);
        if (failure != PIM_SUCCESS) {
            fprintf(stderr, "%s (%d). pim_get_device_info failed (%d)\n",
                    __func__, __LINE__, failure);
            exit(-1);
        }
        number_of_pim_cpus += *(uint32_t *)device_info;
        free(device_info);
    }
    printf("Number of PIM CPUs: %u ", number_of_pim_cpus);
    return num_pims;
}

// Basic function for performing a prefix some from some base address
// through some length. Note that this is not an in-place prefix sum
// The output is stored into out_array.
void prefix_sum(uint32_t *input, unsigned int length, uint64_t *out_array)
{
    int i = 1;
    if (length == 0 || input == NULL || out_array == NULL)
        return;
    if (length >= 1)
        out_array[0] = input[0];
    while (i < length) {
        out_array[i] = input[i] + out_array[i-1];
        i++;
    }
}

typedef struct {
    uint32_t *input;
    unsigned int length;
    uint64_t *out_array;
} prefix_sum_input_t;

// Called by pthreads, so needs a new fnction prototype. This stage is
// just a wrapper for the regular prefix_sum() above.
void *parallel_prefix_sum(void *args)
{
    prefix_sum_input_t *temp = (prefix_sum_input_t *)args;
    prefix_sum(temp->input, temp->length, temp->out_array);
    return NULL;
}

typedef struct {
    uint64_t input;
    unsigned int length;
    uint64_t *out_array;
} prefix_sum_second_input_t;

// The naive PPS algorithm used here splits the giant array into num_threads
// equal parts, and the threads do local prefix sum seperately. A
// smaller "prefix sum of last values of each local collection" is then done.
// This second part of the algorithm adds those needed values into the
// output array in order to make the correct numbers.
void *parallel_prefix_sum_second(void *args)
{
    int i = 0;
    uint64_t input, *out_array;
    unsigned int length;
    prefix_sum_second_input_t *temp = (prefix_sum_second_input_t *)args;
    out_array = temp->out_array;
    input = temp->input;
    length = temp->length;
    if (length == 0 || input == 0 || out_array == NULL)
        return NULL;
    while (i < length) {
        out_array[i] = out_array[i] + input;
        i++;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int i = 0, j = 0;

    // Serial prefix sum arrays
    uint32_t *input_array;
    uint64_t *output_array;

    // Parallel prefix sum arrays
    uint32_t **parallel_input_array;
    uint64_t **parallel_output_array;
    uint64_t *intermediate_vals;

    // The number of variables that each thread will work on
    int amount_to_move = 0;

    // Variables needed for spawning off the parallel prefix sum threads
    unsigned int num_threads;
    pthread_t *pim_threads;
    void *pim_args[NUM_PTHREAD_ARGUMENTS];
    size_t arg_size[NUM_PTHREAD_ARGUMENTS];
    pim_f pim_function;
    prefix_sum_input_t *thread_input;
    prefix_sum_second_input_t *second_thread_input;

    void *spawn_error = NULL;

    srand(0);

    printf("**************************************************\n");
    printf("Beginning CPU-PThreads PIM Spawn Test\n");

    /* Get PIM information */
/*
    if (argc < 2) {
        fprintf(stderr, "Requires a directory to search for XML file as its argument.\n");
        exit(-1);
    }
*/
    initialize_pim(argv[1]);
    num_threads = find_and_map_pims();
    printf("Number of threads: %d\n", num_threads);

    // FIXME: I believe this will not quite work if the array size and num_threads don't
    // divide evently. We will leave the remainder uselessly dangling at the end of the last
    // partial array. FIXME LATER
    if (num_threads != 0)
        amount_to_move = ARRAY_SIZE/num_threads;
    else {
        fprintf(stderr, "This test must be run with at least one PIM.\n");
        exit(-1);
    }
    // Allocates the per-PIM structures.
    parallel_input_array = malloc(sizeof(uint32_t *) * num_threads);
    parallel_output_array = malloc(sizeof(uint64_t *) * num_threads);
    for (i = 0; i < num_threads; i++) {
        parallel_input_array[i] = pim_malloc(sizeof(uint32_t) * amount_to_move, list_of_pims[i], PIM_MEM_DEFAULT_FLAGS, PIM_PLATFORM_PTHREAD_CPU);
        parallel_output_array[i] = pim_malloc(sizeof(uint64_t) * amount_to_move, list_of_pims[i], PIM_MEM_DEFAULT_FLAGS, PIM_PLATFORM_PTHREAD_CPU);
    }
    input_array = malloc(sizeof(uint32_t) * ARRAY_SIZE);
    output_array = malloc(sizeof(uint64_t) * ARRAY_SIZE);
    pim_threads = malloc(sizeof(pthread_t) * num_threads);
    thread_input = malloc(sizeof(prefix_sum_input_t) * num_threads);
    second_thread_input = malloc(sizeof(prefix_sum_second_input_t) * num_threads);
    intermediate_vals = malloc(sizeof(uint64_t) * num_threads);

    printf("Generating initial array...");
    fflush(stdout);
    for (i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = rand() % 1000;
        output_array[i] = 0;
    }
    for (i = 0; i < num_threads; i++) {
        for (j = 0; j < amount_to_move; j++) {
            parallel_input_array[i][j] = input_array[(i*amount_to_move)+j];
            parallel_output_array[i][j] = 0;
        }
    }

    printf("Calculating serial parallel prefix...\n");
    fflush(stdout);
    // Serial Version for Corectness Checking
    prefix_sum(input_array, ARRAY_SIZE, output_array);

    pim_emu_begin();
    printf("Parallel prefix stage 1...");
    fflush(stdout);
    // First Parallel Part
    for (i = 0; i < num_threads; i++) {
        (thread_input[i]).input = parallel_input_array[i];
        (thread_input[i]).length = amount_to_move;
        (thread_input[i]).out_array = parallel_output_array[i];
        pim_args[PTHREAD_ARG_THREAD] = &(pim_threads[i]); // pthread_t
        arg_size[PTHREAD_ARG_THREAD] = sizeof(pthread_t);
        pim_args[PTHREAD_ARG_ATTR] = NULL; // pthread_attr_t
        arg_size[PTHREAD_ARG_ATTR] = sizeof(pthread_attr_t);
        pim_args[PTHREAD_ARG_INPUT] = &(thread_input[i]); // void * for thread input
        arg_size[PTHREAD_ARG_INPUT] = sizeof(void *);
        pim_function.func_ptr = parallel_prefix_sum;
        spawn_error = pim_spawn(pim_function, pim_args, arg_size, NUM_PTHREAD_ARGUMENTS, list_of_pims[i], PIM_PLATFORM_PTHREAD_CPU);
        if (spawn_error == NULL)
            exit(-1);
    }
    for (i = 0; i < num_threads; i++) {
        pthread_join(pim_threads[i], NULL);
    }

    printf("stage 2...");
    fflush(stdout);
    // Reduce part
    intermediate_vals[0] = parallel_output_array[0][amount_to_move-1];
    for (i = 1; i < (num_threads-1); i++) {
        intermediate_vals[i] = parallel_output_array[i][amount_to_move-1] + intermediate_vals[i-1];
    }

    printf("stage 3...\n");
    fflush(stdout);
    // Add intermediate vals into everything in this PIM's section
    for (i = 1; i < num_threads; i++) {
        (second_thread_input[i]).input = intermediate_vals[i-1];
        (second_thread_input[i]).length = amount_to_move;
        (second_thread_input[i]).out_array = parallel_output_array[i];
        pim_args[PTHREAD_ARG_THREAD] = &(pim_threads[i]); // pthread_t
        arg_size[PTHREAD_ARG_THREAD] = sizeof(pthread_t);
        pim_args[PTHREAD_ARG_ATTR] = NULL; // pthread_attr_t
        arg_size[PTHREAD_ARG_ATTR] = sizeof(pthread_attr_t);
        pim_args[PTHREAD_ARG_INPUT] = &(second_thread_input[i]); // void * for thread input
        arg_size[PTHREAD_ARG_INPUT] = sizeof(void *);
        pim_function.func_ptr = parallel_prefix_sum_second;
        spawn_error = pim_spawn(pim_function, pim_args, arg_size, NUM_PTHREAD_ARGUMENTS, list_of_pims[i], PIM_PLATFORM_PTHREAD_CPU);
        if (spawn_error == NULL)
            exit(-1);
    }
    for (i = 1; i < num_threads; i++) {
        pthread_join(pim_threads[i], NULL);
    }
    pim_emu_end();

    printf("Checking results.\n");
    for (i = 0; i < num_threads; i++) {
        for (j = 0; j < amount_to_move; j++) {
            if (output_array[(amount_to_move*i)+j] != parallel_output_array[i][j]) {
                fprintf(stderr,
                        "Parallel Prefix Sum First differs at Index %d,%d. %"
                        PRIu64 " != %" PRIu64 "\n", i, j,
                        output_array[(amount_to_move*i)+j],
                        parallel_output_array[i][j]);
                exit(-1);
            }
        }
    }

    free(intermediate_vals);
    free(second_thread_input);
    free(thread_input);
    free(pim_threads);
    free(output_array);
    free(input_array);
    for (i = 0; i < num_threads; i++) {
        free(parallel_output_array[i]);
        free(parallel_input_array[i]);
    }
    free(parallel_output_array);
    free(parallel_input_array);

    printf("Success!\n");
    return 0;
}
