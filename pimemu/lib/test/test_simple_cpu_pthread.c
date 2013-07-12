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
#include <pim.h>
#include <pthread.h>
#include <string.h>

#define PIM_NUMBER 12345
#define CPU_NUMBER 98765

void *pim_work(void *parameter)
{
    //printf("In PIM thread: %d\n", *(int *)parameter);
    if ((*(int *)parameter) != PIM_NUMBER) {
        fprintf(stderr, "\tPIM Thread Value Incorrect: %d\n", *(int *)parameter);
        exit(-1);
    }
    return NULL;
}

void *cpu_work(void *parameter)
{
    //printf("In CPU thread: %d\n", *(int *)parameter);
    if ((*(int *)parameter) != CPU_NUMBER) {
        fprintf(stderr, "\tCPU Thread Value Incorrect: %d\n", *(int *)parameter);
        exit(-1);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int *pim_num;
    int *cpu_num;
    void *pim_args[NUM_PTHREAD_ARGUMENTS];
    size_t arg_size[NUM_PTHREAD_ARGUMENTS];
    pthread_t cpu_thread, pim_thread;

    void *spawn_error = NULL;
    int pthread_error = 0;

    pim_f pim_function;

    printf("**************************************************\n");
    printf("Beginning Simple CPU-PThreads PIM Spawn Test\n");

    pim_num = malloc(sizeof(int));
    cpu_num = malloc(sizeof(int));

    *pim_num = PIM_NUMBER;
    *cpu_num = CPU_NUMBER;

    pim_args[PTHREAD_ARG_THREAD] = &pim_thread;
    arg_size[PTHREAD_ARG_THREAD] = sizeof(pthread_t);
    pim_args[PTHREAD_ARG_ATTR] = NULL;
    arg_size[PTHREAD_ARG_ATTR] = sizeof(pthread_attr_t);
    pim_args[PTHREAD_ARG_INPUT] = pim_num;
    arg_size[PTHREAD_ARG_INPUT] = sizeof(void *);
    pim_function.func_ptr = pim_work;

    // Launch a thraed to "PIM 0." Doesn't matter that PIM 0 doesn't exist.
    // This is just to make sure that the PIM spawn call actually happens.
    spawn_error = pim_spawn(pim_function, pim_args, arg_size, NUM_PTHREAD_ARGUMENTS, 0, PIM_PLATFORM_PTHREAD_CPU);
    if (spawn_error == NULL) {
        fprintf(stderr, "Failed to spawn with errno: %x\n", errno);
        exit(-1);
    }
    //pthread_create(&pim_thread, NULL, pim_work, &pim_num);
    pthread_error = pthread_create(&cpu_thread, NULL, cpu_work, cpu_num);
    if (pthread_error != 0) {
        int temp_errno = errno;
        fprintf(stderr, "Failed to pthread create with errno: %x\n", errno);
        fprintf(stderr, "%s\n", strerror(temp_errno));
        exit(-1);
    }

    pthread_join(pim_thread, NULL);
    pthread_join(cpu_thread, NULL);

    printf("Success!\n");

    free(pim_num);
    free(cpu_num);

    return 0;
}
