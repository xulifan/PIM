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

//#pragma OPENCL EXTENSION cl_amd_printf : enable



//////////////////////////////////////////////////////////////////

__kernel void waxpby(const int length, 
	const BASE_TYPE alpha, 
    const BASE_TYPE beta,
	__global const BASE_TYPE* x, 
    __global const BASE_TYPE* y,
    __global BASE_TYPE* w) {

    const int tid    = get_global_id(0);
    

	const int stride = get_global_size(0);
	const int start  = get_global_offset(0) + tid;
	const int end    = min((int)(get_global_offset(0) + get_global_size(0)), length); //length;

	/*for(int i = start; i < end; i += stride) {
		w[i]   = alpha * x[i]   + beta * y[i]; //w[i] = mad(alpha, x[i], beta * y[i]);
	}*/
	//int i = start;
	
	//if(i < length)
	//	w[i]   = alpha * x[i]   + beta * y[i];
	w[tid] = alpha * x[tid] + beta * y[tid];
}

//////////////////////////////////////////////////////////////////
  
__kernel void matvecdot(const int row_count,
	const __global int*   row_offsets,
	const __global int*   cols,
	const __global BASE_TYPE* coeffs,
	const __global BASE_TYPE* x,
	__global BASE_TYPE* y,
	const BASE_TYPE     beta,
	__global BASE_TYPE* partials) {
	
	const int tid     = get_global_id(0);
	BASE_TYPE sum_dp = 0;
	

	const int stride  = get_global_size(0);
//	const int end     = row_count;
//	const int start   = tid;

	//for(int row_counter = start; row_counter < end; row_counter += stride) {
	if(tid < row_count) {
	int row_counter = tid;
		const int row_start = row_offsets[row_counter];
		const int row_end   = row_offsets[row_counter + 1];
			
		BASE_TYPE sum   = beta * y[row_counter];
		
		for(int i = row_start; i < row_end; i++) {
			sum = mad(coeffs[i] , x[ cols[i] ], sum);
		}
		
		y[row_counter] = sum;
		
		sum_dp = sum_dp + x[row_counter] * sum;
    //}
    
    partials[tid] = sum_dp;
    }
}

//////////////////////////////////////////////////////////////////

__kernel void dotprod(const int length, 
	__global const BASE_TYPE* x, 
    __global const BASE_TYPE* y,
    __global BASE_TYPE* partials) {
    
    const int tid       = get_global_id(0);
    

	const int stride = get_global_size(0);
	const int end    = length;
	const int start  = tid;

    BASE_TYPE sum = 0;


    for(int i = start; i < end; i += stride) {
		sum = mad(x[i], y[i], sum);
	}
    
    partials[tid] = sum;
}


//////////////////////////////////////////////////////////////////

__kernel void matvec(const int row_count,
	const __global int*   row_offsets,
	const __global int*   cols,
	const __global BASE_TYPE* coeffs,
	const __global BASE_TYPE* x,
	__global BASE_TYPE* y,
	const BASE_TYPE     beta) {
	
	const int tid     = get_global_id(0);
	

	const int stride  = get_global_size(0);
	const int end     = row_count;
	const int start   = tid;

	for(int row_counter = start; row_counter < end; row_counter += stride) {
		const int row_start = row_offsets[row_counter];
		const int row_end   = row_offsets[row_counter + 1];
			
		BASE_TYPE sum   = beta * y[row_counter];
					

		for(int i = row_start; i < row_end; i++) {
			sum = mad(coeffs[i] , x[ cols[i] ], sum);
		}
		y[row_counter] = sum;
	}
}
