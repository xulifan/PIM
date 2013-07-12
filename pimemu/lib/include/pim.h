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

#ifndef __LIB_PIM_H_
#define __LIB_PIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <errno.h>
#define __STDC_FORMAT_MACROS
#ifdef WIN32
#include <stdint.h>
#include "pim_win.h"
#else
#include <inttypes.h>
#endif
/******************************************************************************/

/* HOST ID */
#define PIM_EHP_ID                                 -2
#define PIM_HOST_ID                                PIM_EHP_ID
/* PIM INVALID ID */
#define PIM_INVALID_ID                             -1

/* Return Codes */
#define PIM_SUCCESS                                 0
#define PIM_GENERAL_ERROR                           -1

#define PIM_BUILTWITHOUT_PTHREAD                    -2

#define PIM_INIT_FILE_ACCESS_ERROR                  -3
#define PIM_INIT_FILE_SYNTAX_ERROR                  -4

#define PIM_INVALID_DEVICE_TYPE                     -5
#define PIM_INVALID_DEVICE_ID                       -6
#define PIM_INVALID_PARAMETER                       -7
#define PIM_INSUFFICIENT_SPACE                      -8
#define PIM_SPAWN_ERROR_BAD_TYPE                    -9

// This error happens when a pim_spawn() is targetted at a device that does not
// have the capability to run the spawn type requested
#define PIM_SPAWN_ERROR_DEVICE_TYPE_MISMATCH        -10

//Error returns for PIM configuration validation
#define PIM_INVALID_CPU_CONFIGURATION_PARAMETER -11
#define PIM_INVALID_GPU_CONFIGURATION_PARAMETER -12
#define PIM_INVALID_DRAM_CONFIGURATION_PARAMETER -13
#define PIM_INVALID_EHP_CONFIGURATION_PARAMETER -14
#define PIM_INVALID_PIM_CONFIGURATION_PARAMETER -15
#define PIM_INVALID_CONFIGURATION -16


/* Custom Structures */
typedef uint32_t pim_device_type;
#define PIM_CLASS_0                                 0

typedef uint32_t pim_device_id;

typedef uint32_t pim_acc_flags;
#define PIM_MEM_PIM_READ                            (1<<0)
#define PIM_MEM_PIM_WRITE                           (1<<1)
#define PIM_MEM_PIM_RW                              (PIM_MEM_PIM_READ|PIM_MEM_PIM_WRITE)
#define PIM_MEM_HOST_READ                           (1<<2)
#define PIM_MEM_HOST_WRITE                          (1<<3)
#define PIM_MEM_HOST_RW                             (PIM_MEM_HOST_READ|PIM_MEM_HOST_WRITE)
#define PIM_MEM_HOST_NOACCESS                       (1<<4)
#define PIM_MEM_PEER_READ                           (1<<5)
#define PIM_MEM_PEER_WRITE                          (1<<6)
#define PIM_MEM_PEER_RW                             (PIM_MEM_PEER_READ|PIM_MEM_PEER_WRITE)
#define PIM_MEM_PEER_NOACCESS                       (1<<7)
#define PIM_MEM_DEFAULT_FLAGS                       (PIM_MEM_PIM_RW|PIM_MEM_HOST_RW|PIM_MEM_PEER_RW)
#define PIM_MEM_ALIGN_WORD                          (1<<8) // 64-bit word-aligned
#define PIM_MEM_ALIGN_CACHELINE                     (1<<9) // cache-line-aligned
#define PIM_MEM_ALIGN_PAGE                          (1<<10)  // OS-page-aligned
#define PIM_MEM_ALIGNED                             (PIM_MEM_ALIGN_WORD | PIM_MEM_ALIGN_CACHELINE | PIM_MEM_ALIGN_PAGE)


typedef uint32_t pim_platform_type;
#define PIM_PLATFORM_PTHREAD_CPU                    1
#define PIM_PLATFORM_OPENCL_GPU                     2

typedef uint32_t pim_device_info;
#define PIM_DEFAULT_PARAMETER                       0
#define PIM_HOST_BANDWIDTH                          1
#define PIM_INTERPIM_BANDWIDTH                      2
#define PIM_CPU_CORES                               3
#define PIM_CPU_FREQ                                4
#define PIM_GPU_CORES                               5
#define PIM_GPU_NUM_CUS                             6
#define PIM_GPU_FREQ                                7
#define PIM_MEM_CAPACITY                            8
#define PIM_MEM_FREE                                9
#define PIM_MEM_BANDWIDTH                           10
#define PIM_MEM_MBPS                                 11
#define PIM_MEM_NUM_CHANNELS                        12
#define HOST_CPU_CORES                              13
#define HOST_CPU_FREQ                               14
#define HOST_GPU_CORES                              15
#define HOST_GPU_NUM_CUS                            16
#define HOST_GPU_FREQ                               17
#define PIM_MAXIMUM_PARAMETER                       HOST_GPU_FREQ

typedef union {
    void *(* func_ptr)(void *);
    void *func_name;
} pim_f;

#define NUM_PTHREAD_ARGUMENTS 3
#define PTHREAD_ARG_THREAD 0
#define PTHREAD_ARG_ATTR 1
#define PTHREAD_ARG_INPUT 2


#define NUM_OPENCL_ARGUMENTS    8
#define OPENCL_ARG_SOURCE       0
#define OPENCL_ARG_OPTIONS      1
#define OPENCL_ARG_DIM          2
#define OPENCL_ARG_GLOBALSIZE   3
#define OPENCL_ARG_LOCALSIZE    4
#define OPENCL_ARG_NUMPREEVENTS 5
#define OPENCL_ARG_PREEVENTS    6
#define OPENCL_ARG_POSTEVENT    7


/******************************************************************************/

/* Platform APIs */

/* int pim_get_device_ids()
 * Query the number of PIM devices of a given type that are available and
 * retrieve their device IDs.
 * Arguments:
 *      pim_device_type device_type
 *          Input
 *          Denotes the particular class of PIMs that are to be returned
 *      uint32_t num_entries
 *          Input
 *          The maximum number of devices that this function call should return
 *      pim_device_id *devices
 *          Output
 *          Contains a list of PIM devices of type device_type. The array should
 *          already exist upon calling this function. Passing NULL will cause
 *          this function to not fill in any list.
 *      uint32_t *num_devices
 *          Output
 *          Is updated by the call to reflect the total number of devices (of
 *          type 'device_type') found in the system. If num_devices is NULL,
 *          the argument is ignored.
 * Returns:
 *      int
 *          PIM_SUCCESS if no errors
 *          PIM_INVALID_DEVICE_TYPE if device_type is not defined properly.
 *          PIM_GENERAL_ERROR on all other problems. */
extern int
pim_get_device_ids(pim_device_type device_type,
                   uint32_t num_entries,
                   pim_device_id *devices,
                   uint32_t *num_devices);

/* int pim_get_id()
 * Retrieve the PIM device ID of the device that this code is run on.
 * For example, a thread running on PIM 3 will see 3, while a thread
 * running on the host processor will see the constant PIM_EHP_ID.
 * Arguments:
 *      None
 * Returns:
 *      int
 *          The device ID of the PIM the code that calls this function
 *          is running on.
 *          If that code is running on the host, then PIM_EHP_ID. */
extern int
pim_get_id(void);


/* int pim_get_device_info()
 * Query a particular PIM device to learn more about its parameters
 * Arguments:
 *      pim_device_id device
 *          Input
 *          The ID number, as returned by pim_get_device_ids(), of the device
 *          to be queried.
 *      pim_device_info param_name
 *          Input
 *          The parameter being queried. Supported parameters vary by type
 *          and class of device.
 *      size_t param_value_size
 *          Input
 *          Size, in bytes, of the memory pointed to by param_value
 *      void *param_value
 *          Output
 *          Pointer to memory location where the value of the parameter being
 *          queried will be stored. Passing NULL will cause this function to
 *          not fill in any list.
 *      size_t *param_value_size_ret
 *          Output
 *          Is updated by the call to reflect the total number of bytes
 *          returned in param_value. param_value_size is the upper bound
 *          for this number. If param_value_size_ret is NULL, the argument
 *          is ignored.
 * Returns:
 *      int
 *          PIM_SUCCESS if no errors
 *          PIM_INVALID_DEVICE_ID if pim_device_id is not an existing device
 *          PIM_INVALID_PARAMETER if the parameter being queried is not valid
 *              for the device type of the queried device.
 *          PIM_GENERAL_ERROR on all other problems */
extern int
pim_get_device_info(pim_device_id device,
                    pim_device_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret);

/* Memory APIs */

/* void *pim_malloc() with flags and device type
 * Attempts to allocate some amount of memory to a memory stack associated
 * with a particular PIM device.
 * The extra flags allow hints about the access type (e.g. read only,
 * write-only) and what type of device this memory will be for.
 * Legacy OpenCL types, platform=PIM_PLATFORM_OPENCL_GPU, will return an
 * OpenCL buffer in the void*.
 * Arguments:
 *      size_t size
 *          Input
 *          Requested number of malloced bytes.
 *      pim_device_id device
 *          Input
 *          Device whose memory stack is the allocation destination
 *      pim_acc_flags acc_flags
 *          Input
 *          Flags describing the access type
 *      pim_platform_type platform
 *          Input
 *          Platform (e.g. CPU, GPU) that will primarily access this memory
 * Returns:
 *      void*
 *          Pointer to the allocated memory or buffer structure.
 *          Returns NULL on any errors and sets errno. Errors are the same as
 *          seen with LIBC malloc (for platform=CPU) or OpenCL clCreateBuffer
 *          with the additional error that an invalid device will also cause
 *          the function to error out */
extern void *
pim_malloc(size_t size,
           pim_device_id device,
           pim_acc_flags acc_flags,
           pim_platform_type platform);


/* void pim_free()
 * This must be used to de-allocate memory that was created using pim_malloc().
 * Performing a regular free() or library-specific de-allocation will result
 * in undefined results.
 * Arguments:
 *      void *object
 *          Input
 *          The object to be de-allocated. This must be the object, of whatever
 *          particular opaque type, returned by a previosu pim_malloc* call.
 *      Returns:
 *          Void */
extern void
pim_free(void *object);

/* void *pim_map()
 * Data structures that are returned by pim_malloc() for some platform types
 * (for example, the value returned when platform=PIM_PLATFORM_OPENCL_GPU)
 * must be mapped to a native host pointer before being accessed in some way
 * besides through OpenCL calls. This function performs that mapping.
 * Arguments:
 *      void *handle
 *          Input
 *          Structure returned by a pim_malloc() to a non-native device
 *      pim_platform_type platform
 *          Input
 *          Platform type that this memory should be changed to
 * Returns:
 *      void *
 *          Pointer to a host-native memory location for the data contained
 *          within the handle structure. 
 *          Will return NULL and set errno when encountering problems. */
extern void *
pim_map(void *handle, pim_platform_type platform);

/* void *pim_unmap()
 * Remove host CPU access to a region that was previously pim_map()ed.
 * Arguments:
 *      void *ptr
 *          Input
 *          Pointer that was returned from a pim_map()
 * Returns:
 *      int
 *          PIM_SUCCESS on success.
 *          TODO other error codes. */
extern int
pim_unmap(void *ptr);

/* Computation APIs */

/* void *pim_spawn()
 * Allows an asynchronous dispatch of some piece of computation to the APU that
 * is located on a taretted device.
 * Arguments:
 *      pim_f to_launch
 *          Input
 *          Either a function pointer (to_launch.func_ptr) or a pointer to other
 *          launch information, such as a string that is the kernel name
 *          (to_launch.func_name). The type of information that must be passed in
 *          depends on the platform_type.
 *      void **args
 *          Input
 *          Address of an array of pointers to arguments. The exact format of
 *          this list of arguments depends on the platform_type. For example, a
 *          CPU_THREAD type may include the addresses for the arguments for the
 *          function.
 *      size_t *arg_sizes
 *          Input
 *          An array describing the length of each of the arguments pointed to
 *          in args
 *      size_t nargs
 *          Input
 *          Number of arguments, and thus the dimension of the first arg array.
 *      pim_device_id device
 *          Input
 *          Device that this computation will run on.
 *      pim_platform_type platform_type
 *          Input
 *          Specifies the type of task being spawned, such as a CPU thread or
 *          an OpenCL call for a GPU
 * Returns:
 *      void*
 *          Will return a pointer to a platform_type specific struct on success
 *          Will return NULL on any type of failure, with errno set to some
 *          particular value depending on the error:
 *          Otherwise will return one of the PIM_SPAWN_ERROR_* constants.
 *          Or PIM_INVALID_DEVICE_ID if device does not point to a legal PIM
 *          Or PIM_GENERAL_ERROR on everything else. */
extern void*
pim_spawn(pim_f to_launch,
          void **args,
          size_t *arg_sizes,
          size_t nargs,
          pim_device_id device,
          pim_platform_type platform_type);


/* Performance Analysis APIs */
/* These function calls are not part of the "official" PIM API, but are instead
 * part of the PIM emulation and performance analysis infrastructure. */

/* int pim_emu_begin()
 * Enables performance analysis within the emulator after this point.
 * Before calling this the first time, no performance analysis is performed.
 * This is useful when, for instance, your program has a large amount of
 * initialization that you do not wish to model as running within the PIM
 * system.
 * NOTE: This should only be called from the main thread in the program.
 * Calling it from other threads will result in undefined operation.
 * Arguments:
 *      None
 * Returns:
 *      int
 *          PIM_SUCCESS if a performance gathering was successfully enabled,
 *          Otherwise, PIM_GENERAL_ERROR. */
extern int
pim_emu_begin(void);


/* int pim_emu_end()
 * Stops the performance analysis within the emulator after this point.
 * This can be called before a program ends if there are checking or shutdown
 * tasks that you do not desire to measure. If this is not called by the time
 * the program ends, the PIM emulator will call it implicitly.
 * This can *also* be used to "pause" performance gathering in the middle of
 * the program. Calling pim_emu_begin() afterwards will "unpause" the
 * gathering of performance data.
 * NOTE: This should only be called from the main thread in the program.
 * Calling it from other threads will result in undefined operation.
 * Arguments:
 *      None
 * Returns:
 *      int
 *          PIM_SUCCESS if a performance gathering was successfully enabled,
 *          Otherwise, PIM_GENERAL_ERROR. */
extern int
pim_emu_end(void); 

#ifdef __cplusplus
}
#endif

#endif  /* __LIB_PIM_H_ */
