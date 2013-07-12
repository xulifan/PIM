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

inline uint my_get_global_id(uint dim)
{

   return(get_global_id(dim));
}

inline uint my_get_global_size(uint dim)
{
   return(get_global_size(dim));
}

//////////////////////////////////////////////////////////////////

__kernel void waxpby(const int length, 
	const float alpha, 
    const float beta,
	__global const float* x, 
    __global const float* y,
    __global float* w) {

    const int tid    = my_get_global_id(0);
    

	w[tid] = alpha * x[tid] + beta * y[tid];

}

//////////////////////////////////////////////////////////////////
  
__kernel void matvecdot(const int row_count,
	const __global int*   row_offsets,
	const __global int*   cols,
	const __global float* coeffs,
	const __global float* x,
	__global float* y,
	const float     beta,
	__global float* partials) {
	
	const int tid     = my_get_global_id(0);
	float sum_dp = 0;
	
#ifdef __GPU__
	const int stride  = my_get_global_size(0);
	const int end     = row_count;
	const int start   = tid;
#else
	const int stride  = 1;
	const int end     = (tid == my_get_global_size(0) - 1) ? row_count : (tid+1) * (row_count / my_get_global_size(0));
	const int start   = tid * (row_count / my_get_global_size(0));
#endif

	//for(int row_counter = start; row_counter < end; row_counter += stride) {
	if(tid < row_count) {
	int row_counter = tid;
		const int row_start = row_offsets[row_counter];
		const int row_end   = row_offsets[row_counter + 1];
			
		float sum   = beta * y[row_counter];
		
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
	__global const float* x, 
    __global const float* y,
    __global float* partials) {
    
    const int tid       = my_get_global_id(0);
    
#ifdef __GPU__
	const int stride = my_get_global_size(0);
	const int end    = length;
	const int start  = tid;
#else
	//const int stride = 1;
	const int end    = (tid == my_get_global_size(0) - 1) ? length : (tid + 1) * (length / my_get_global_size(0));
	const int start  = tid * (length / my_get_global_size(0));
#endif

    float sum = 0;

#ifdef __GPU__
    for(int i = start; i < end; i += stride) {
		sum = mad(x[i], y[i], sum);
	}
#else
    for(int i = start; i < end; i++) {
    	sum = mad(x[i], y[i], sum);
    }
#endif
    
    partials[tid] = sum;
}


//////////////////////////////////////////////////////////////////

__kernel void matvec(const int row_count,
	const __global int*   row_offsets,
	const __global int*   cols,
	const __global float* coeffs,
	const __global float* x,
	__global float* y,
	const float     beta) {
	
	const int tid     = my_get_global_id(0);
	
#ifdef __GPU__
	const int stride  = my_get_global_size(0);
	const int end     = row_count;
	const int start   = tid;
#else
	const int stride  = 1;
	const int end     = (tid == my_get_global_size(0) - 1) ? row_count : (tid+1) * (row_count / my_get_global_size(0));
	const int start   = tid * (row_count / my_get_global_size(0));
#endif

	for(int row_counter = start; row_counter < end; row_counter += stride) {
		const int row_start = row_offsets[row_counter];
		const int row_end   = row_offsets[row_counter + 1];
			
		float sum   = beta * y[row_counter];
					
#ifdef __GPU__
		for(int i = row_start; i < row_end; i++) {
			sum = mad(coeffs[i] , x[ cols[i] ], sum);
		}
#else		
		for(int i = row_start; i < row_end; i++) {
			sum = sum + coeffs[i] * x[ cols[i] ];
		}
#endif			
		y[row_counter] = sum;
	}
}


__kernel void waxpby_withloop(
	const int loops,
	const float dummy0,
	const int length, 
	const float alpha, 
    const float beta,
	__global const float* x, 
    __global const float* y,
    __global float* w) {

const int tid    = my_get_global_id(0);
    

float loop_res = alpha * x[tid] + beta * y[tid];
float w_out = loop_res;  
   for( int i = 0; i < loops; i++)
   {
	   w_out *= (loop_res + 1.f); 
	   w_out /= (loop_res + 1.f + dummy0); 
   }

	w[tid] = w_out; 

}
