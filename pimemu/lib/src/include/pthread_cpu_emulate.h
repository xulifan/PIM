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

#ifndef __PTHREAD_CPU_EMULATE_H_
#define __PTHREAD_CPU_EMULATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include "wpthread.h"
#else
#include <pthread.h>
#endif
#include "pim-private.h"

// CPU thread device does not need a pim_map() handler.
// Because data that is mapped to this device is already in host pointer format

// CPU thread device uses regular mapped memory, so PIM emulator higher
// up in the stack will handle the pim_unmap() call.

/* void *pim_cpu_malloc()
 * Perform the underlying memory allocation for a platform of type
 * PIM_PLATFORM_PTHREAD_CPU.
 * Arguments:
 *      size_t size
 *          Input
 *          Number of bytes requested to be allocated on the heap.
 *      pim_device_id device
 *          Input
 *          Device/memory stac k within the PIM-enabled system that should
 *          contain this memory.
 *      pim_acc_flags acc_flags
 *          Input
 *          Flags describing the access type of this region.
 * Returns:
 *      void*
 *          Pointer to the allocated memory or buffer structure.
 *          Returns NULL on any errors and sets errno. Errors are the same as
 *          seen with LIBC malloc (for platform=CPU) or OpenCL clCreateBuffer
 *          with the additional error that an invalid device will also cause
 *          the function to error out */
void *pim_cpu_malloc(size_t size, pim_device_id device,
        pim_acc_flags acc_flags);

/* void *pim_cpu_malloc_aligned()
 * Perform the underlying memory allocation for a platform of type
 * PIM_PLATFORM_PTHREAD_CPU. Requires the allocation to be aligned to
 * OS memory pages.
 * Arguments:
 *      size_t size
 *          Input
 *          Number of bytes requested to be allocated on the heap.
 *      pim_device_id device
 *          Input
 *          Device/memory stac k within the PIM-enabled system that should
 *          contain this memory.
 *      pim_acc_flags acc_flags
 *          Input
 *          Flags describing the access type of this region.
 * Returns:
 *      void*
 *          Pointer to the allocated memory or buffer structure.
 *          Returns NULL on any errors and sets errno. Errors are the same as
 *          seen with LIBC malloc (for platform=CPU) or OpenCL clCreateBuffer
 *          with the additional error that an invalid device will also cause
 *          the function to error out */
void *pim_cpu_malloc_aligned(size_t size, size_t alignment, pim_device_id device,
        pim_acc_flags acc_flags, void ** actual_ptr);

/* void int spawn_cpu_thread()
 * Internal mechanism that pim_spawn() calls for platforms of type
 * PIM_SPAWN_PTHREAD_CPU. In essence, this is a wrapper for a
 * pthread_create() call that can be pointed towards a particular PIM device.
 * Arguments:
 *      pim_f to_launch
 *          Input
 *          The function pointer to be invoked with starting the new thread.
 *      void **args
 *          Input
 *          Address of an array of pointers to arguments. This is essentially a
 *          wrapper around a pthread_create() function call (except for the
 *          function pointer, so the arguments are:
 *          1. pthread_t *thread: output for the ID of the newly created thread
 *          2. pthread_attr_t *attr: input for the attributes of the newly created thread
 *          4. void *arg: Sole argument being passed into new thread
 *      size_t *arg_sizes
 *          Input
 *          An array describing the length of each of the arguments pointed to
 *          in args.
 *      size_t nargs
 *          Input
 *          Number of arguments, and thus the dimension of the first arg array.
 *      pim_device_id device
 *          Input
 *          Device that this computation will run on.
 * Returns:
 *      int
 *          Will return the same as pthread_create()
 *          Success: returns 0
 *          Otherwise returns an error number. */
int spawn_cpu_thread(pim_f to_launch, void **args, size_t *arg_sizes, size_t nargs,
        pim_device_id device);

/*
* multiplatform cache size
*/
size_t cache_line_size(void);

/*
* windows page size
*/

#ifdef WIN32
size_t getpagesize(void);
#endif


#ifdef __cplusplus
}
#endif

#endif  /* __PTHREAD_CPU_EMULATE_H_ */
