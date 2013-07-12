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

#ifndef _PSG_SHARED_H_
#define _PSG_SHARED_H_


const char *cluErrorString(cl_int);
/****************************************************************************************/
// Fill a host vector with random values

template <typename TYPE> 
int fillWithRand(
         std::vector<TYPE> &vec_, 
         size_t width_,
         size_t height_,
         const TYPE rangeMin_,
         const TYPE rangeMax_,
         unsigned int seed_ = 0)
{

    if(vec_.empty())
    {
        std::cout << "Cannot fill vector." << std::endl;
        return EXIT_FAILURE;
    }

    // set seed
    if(!seed_)
        seed_ = (unsigned int)time(NULL);

    srand(seed_);

    // set the range
    double range = double(rangeMax_ - rangeMin_) + 1.0; 

    /* random initialisation of input */
    for(size_t i = 0; i < height_; i++)
        for(size_t j = 0; j < width_; j++)
        {
         size_t index = i*width_ + j;
            vec_[index] = rangeMin_ + (TYPE)(range*rand()/(RAND_MAX + 1.0)); 
        }

    return CL_SUCCESS;
}

template <typename TYPE> 
int fillWithRand(
         TYPE *vec_, 
         size_t width_,
         size_t height_,
         const TYPE rangeMin_,
         const TYPE rangeMax_,
         unsigned int seed_ = 0)
{

    if(!vec_)
    {
        return EXIT_FAILURE;
    }

    // set seed
    if(!seed_)
        seed_ = (unsigned int)time(NULL);

    srand(seed_);

    // set the range
    double range = double(rangeMax_ - rangeMin_) + 1.0; 

    /* random initialisation of input */
    for(size_t i = 0; i < height_; i++)
        for(size_t j = 0; j < width_; j++)
        {
         size_t index = i*width_ + j;
            vec_[index] = rangeMin_ + (TYPE)(range*rand()/(RAND_MAX + 1.0)); 
        }

    return CL_SUCCESS;
}

template <typename TYPE>
    int compare(TYPE * host_buffer_, TYPE * gpu_buffer_, ::size_t len_)
	{
	int ret = -1;
	int failures = 0;
	::size_t first_failure = 0;
       for( ::size_t i = 0; i < len_; i++)
       {


	       if ( host_buffer_[i] == gpu_buffer_[i] )
		   {
	//		   printf("passed at %d\n", i);
		   }
		   else
		   {
			   ret = (int)i;
 //   	       printf("Failed at indx=%d\n", ret);
			   failures++;
			   if ( failures == 1 )
			   {
				   first_failure = i;
				 char format[1024] = {0};
				   strcpy(format, "First falire at index %d,\nh=");
				   strcat(format,TypeTraits<TYPE>::print_format());
				   strcat(format," g=");
				   strcat(format,TypeTraits<TYPE>::print_format());
				   strcat(format,"\n");
				   printf(format,
						    i,host_buffer_[i], gpu_buffer_[i]);
			   }
//			   break;
		   }
       }

       if (failures)
       {
	      printf("Total failures: %d, first failure at %d\n", failures, first_failure);
       }
	   else
	   {
	      printf("Passed!\n");
	   }
	   return(ret);
	}


// CPU SIMULATION
template <typename TYPE>
	void waxpby_kernel( TYPE alpha_,
		               TYPE beta_,
					   TYPE * x_,
					   TYPE * y_,
					   TYPE * w_,
					   ::size_t length_)
	{
		for( int i = 0; i < length_; i++)
		{
			w_[i] = alpha_ * x_[i] + beta_ * y_[i];
		}
	}

template <typename TYPE>
	TYPE dotprod_kernel(
					   TYPE * x_,
					   TYPE * y_,
					   ::size_t length_)
	{
     TYPE ret = 0;  
	 double tmp_ret = 0;
		for( int i = 0; i < length_; i++)
		{
			tmp_ret += (double)x_[i] * (double)y_[i];
		}
		ret = (TYPE)tmp_ret;
		return(ret);
	}


#endif // _SHARED_H_
