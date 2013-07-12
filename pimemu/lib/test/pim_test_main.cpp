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

#include "pim_test.hpp"
#include "pim_test_cpu.hpp"
#include "pim_test_gpu.hpp"


/*****************************************************************************/
/******************** pimTest *************************************************/

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

    return_str = (char*)malloc(sizeof(char) * (directory_length + name_length + 1));
    if (return_str == NULL) {
        fprintf(stderr, "Unable to allocate Directory+Name Concat. Die.\n");
        exit(-1);
    }

    return_str[(directory_length + name_length)] = '\0';
    strcpy(return_str, directory);
    strcpy((return_str+directory_length), name);
    return return_str;
}

char* fix_directory(char *input)
{
    char *directory;
    size_t directory_length;

    directory_length = strlen(input);

    if (input[directory_length-1] != '/')
        directory_length += 1;

    directory = (char*)malloc(sizeof(char) * (directory_length + 1));
    if (directory == NULL) {
        fprintf(stderr, "Unable to allocate directory string. Die.\n");
        exit(-1);
    }
    directory[directory_length] = '\0';

    strcpy(directory, input);
    if (input[directory_length] != '/')
        directory[directory_length-1] = '/';
	return(directory);
}

char* find_directory(char *input)
{
    char *directory;
    size_t directory_length;

	char * pos1 = strrchr(input, '/');
	char * pos2 = strrchr(input, '\\');

	char *faraway = (!pos1)? pos2 : ((!pos2) ? pos1 : (( pos1 < pos2) ? pos2 : pos1));

    directory_length = (size_t)(faraway - input) + 1;

    directory = (char*)malloc(sizeof(char) * (directory_length + 1));

    strncpy(directory, input, directory_length);
	directory[directory_length] = 0;
	return(directory);
}



/*****************************************************************************/
/******************** pimTest *************************************************/
// The map from the list of threads (sequential list) to PIM device ID.



static
uint32_t get_number_pim_compute_cores(ExSclConfig &testDevConfig, uint32_t core_type)
{

    int failure = 0;
  
	for (int i = 0; i < (int)testDevConfig.num_pims; i++)
	{
    int32_t *pcore = (core_type == PIM_CPU_CORES) ? &testDevConfig.pim_cpu_cores[i]
		                                            : &testDevConfig.pim_gpu_cores[i];
        failure = pim_get_device_info(testDevConfig.list_of_pims[i], core_type,
                                      sizeof(uint32_t), pcore, NULL);
        if (failure != PIM_SUCCESS) {
            fprintf(stderr, "%s (%d). pim_get_device_info failed (%d)\n",
                __func__, __LINE__, failure);
            exit(-1);
		}
    }

    return(0);
}

// Use PIM API discovery functions to find the number of PIMs within the system
// As an exercise, this also finds the number of CPUs contained within those
// PIMs. Currently, we only launch one thread per PIM, however, so that is
// what this function returns.


unsigned int find_and_map_pims(ExSclConfig &testDevConfig)
{

    int failure = 0;
    uint32_t num_pims = 0;


    failure = pim_get_device_ids(PIM_CLASS_0, 0, NULL, &testDevConfig.num_pims);

    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): pim_get_device_ids failed(%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }

	num_pims = testDevConfig.num_pims;
    testDevConfig.list_of_pims = new pim_device_id[num_pims];


    failure = pim_get_device_ids(PIM_CLASS_0, testDevConfig.num_pims, testDevConfig.list_of_pims, NULL);

    if (failure != PIM_SUCCESS) {
        fprintf(stderr, "%s (%d): pim_get_device_ids failed(%d)\n",
                __func__, __LINE__, failure);
        exit(-1);
    }


	testDevConfig.pim_cpu_cores = new int32_t[num_pims];

	testDevConfig.pim_gpu_cores = new int32_t[num_pims];

	get_number_pim_compute_cores(testDevConfig, PIM_CPU_CORES);
	get_number_pim_compute_cores(testDevConfig, PIM_GPU_CORES);

    return num_pims;
}


/******************************************************************************/
/*************** TESTS ********************************************************/

static
void * hg0(void * arg);

static
void * hc0(void * arg);

static
void * hpg0(void * arg);

static
void * hpc1(void * arg);
static
void * hpg1(void * arg);
static
void * pg1(void * arg);


static const char * test_nm[] = 
{
"-hc0",
"-hg0",
"-hpg0",
"-hpg1",
"-hpc1",
"-pg1",
 NULL
};

static testThreadFunc test_seqs[32] =
{
hc0,
hg0,
hpg0,
hpg1,
hpc1,
pg1,
NULL
};


static int getTestID(const testThreadFunc test_seqs_)
{
int ret = -1;
    for (int  t = 0;  test_nm[t] && test_seqs[t]; t++)
	{
		if ( test_seqs[t] == test_seqs_ )
		{
			ret = t;
			break;
		}
	}
	return(ret);
}

static
int c1(CPimTest<_BASE_TYPE_> ** launchedTest, TestConfig *cConf, const testThreadFunc * test_seqs, int num_launches)
{
int ret = 0;


int orig_test_id = cConf->test_id;
RunableTestConfig * origRunTest = cConf->tests;
RunableTestConfig * runTest = new RunableTestConfig[num_launches];

   assert(runTest);

   memset(runTest, 0 , sizeof(RunableTestConfig) * num_launches);

   (cConf)->num_tests = num_launches;
   (cConf)->tests = runTest;

   for(int i = 0; i < (int)(cConf)->num_tests; i++)
   {
	   (cConf)->test_id = i;

	   (cConf)->tests[i] = origRunTest[orig_test_id];
	   (cConf)->tests[i].test_seqs = test_seqs[i];
	   launchedTest[i] = new CCPUTest<_BASE_TYPE_>(cConf);
       ((CCPUTest<_BASE_TYPE_>*)launchedTest[i])->Launch();
   }
   
   return(ret);
}

static
void * hc0(void * arg)
{


const TestConfig * passedTest = (const TestConfig * )arg;

CPimTest<_BASE_TYPE_> * ptrs[16];
int num_test_ptrs = 0;

     if ( passedTest->tests[passedTest->test_id].depth > 1 )
     {
     TestConfig nextTest = *passedTest;
	 testThreadFunc test_seqs[1] = {hc0};
	     nextTest.tests[passedTest->test_id].depth--;

	     c1(ptrs, &nextTest, test_seqs, 1);

		 num_test_ptrs++;

     }

CCPU_WAXPBY<_BASE_TYPE_> cpu_waxpby0(passedTest);

     cpu_waxpby0.Init();
	 cpu_waxpby0.Launch();

     ptrs[num_test_ptrs++] = &cpu_waxpby0;


CCPU_DOTPTOD<_BASE_TYPE_> cpu_dotprod0(passedTest);

     cpu_dotprod0.Init();
	 cpu_dotprod0.Launch();

     ptrs[num_test_ptrs++] = &cpu_dotprod0;


CCPU_WAXPBY<_BASE_TYPE_> cpu_waxpby1(passedTest);

     cpu_waxpby1.Init();
	 cpu_waxpby1.Launch();

     ptrs[num_test_ptrs++] = &cpu_waxpby1;



CCPU_DOTPTOD<_BASE_TYPE_> cpu_dotprod1(passedTest);

     cpu_dotprod1.Init();

	 cpu_dotprod1.Launch(num_test_ptrs, ptrs);



	 cpu_dotprod1.Wait();


	 return(0);

}

static
void * hg0(void * arg)
{



const TestConfig * passedTest = (const TestConfig * )arg;

CPimTest<_BASE_TYPE_> * ptrs[16];
int num_test_ptrs = 0;

     if ( passedTest->tests[passedTest->test_id].depth > 1 )
     {
     TestConfig nextTest = *passedTest;
	 testThreadFunc test_seqs[1] = {hg0};
	     nextTest.tests[passedTest->test_id].depth--;

	     c1(ptrs, &nextTest, test_seqs, 1);

		 num_test_ptrs++;

     }

CGPU_WAXPBY<_BASE_TYPE_> GPU_waxpby0(passedTest);

   ptrs[num_test_ptrs++] = &GPU_waxpby0;

   GPU_waxpby0.Init();

   GPU_waxpby0.Launch();


CGPU_DOTPROD<_BASE_TYPE_> GPU_dotprod0(passedTest);

   GPU_dotprod0.Init();

   ptrs[num_test_ptrs++] = &GPU_dotprod0;

   GPU_dotprod0.Launch();


CGPU_WAXPBY<_BASE_TYPE_> GPU_waxpby1(passedTest);

   ptrs[num_test_ptrs++] = &GPU_waxpby1;

   GPU_waxpby1.Init();

   GPU_waxpby1.Launch();


CGPU_DOTPROD<_BASE_TYPE_> GPU_dotprod1(passedTest);

   GPU_dotprod1.Init();

   ptrs[num_test_ptrs++] = &GPU_dotprod1;

   GPU_dotprod1.Launch();





   GPU_dotprod1.WaitForEvents(num_test_ptrs, ptrs);

   return(0);

}




static
void * hpg0(void * arg)
{

CPimTest<_BASE_TYPE_> * ptrs[16];

const TestConfig * passedTest = (const TestConfig * )arg;


CPIMGPU_WAXPBY<_BASE_TYPE_> pimGPU_waxpby0(passedTest);

    ptrs[0] = &pimGPU_waxpby0;

    pimGPU_waxpby0.Init();

    pimGPU_waxpby0.Launch();


CPIMGPU_DOTPROD<_BASE_TYPE_> pimGPU_dotprod0(passedTest);

    ptrs[1] = &pimGPU_dotprod0;

	pimGPU_dotprod0.Init();

    pimGPU_dotprod0.Launch();

CPIMGPU_WAXPBY<_BASE_TYPE_> pimGPU_waxpby1(passedTest);

    ptrs[2] = &pimGPU_waxpby1;

    pimGPU_waxpby1.Init();

    pimGPU_waxpby1.Launch();


CPIMGPU_DOTPROD<_BASE_TYPE_> pimGPU_dotprod1(passedTest);

    ptrs[3] = &pimGPU_dotprod1;

	pimGPU_dotprod1.Init();

    pimGPU_dotprod1.Launch();


	pimGPU_dotprod1.WaitForEvents(4,ptrs);


	return(0);

}

static
void *hpg1(void * arg)
{
const TestConfig * passedTest = (const TestConfig * )arg;
TestConfig *pimGPUTest = new TestConfig[passedTest->modelConfig.num_pims];
CCPUTest<_BASE_TYPE_> **pimGpu = new CCPUTest<_BASE_TYPE_> *[passedTest->modelConfig.num_pims];
RunableTestConfig * runTest = new RunableTestConfig[passedTest->modelConfig.num_pims];
int num_pim_gpu_spawned;
   assert(pimGPUTest && pimGpu && runTest);

   memset(pimGpu, 0, sizeof(CCPUTest<_BASE_TYPE_> *) * passedTest->modelConfig.num_pims);
   memset(runTest, 0 , sizeof(RunableTestConfig) *passedTest->modelConfig.num_pims);
   num_pim_gpu_spawned = 0;
   for(int i = 0; i < (int)passedTest->modelConfig.num_pims; i++)
   {
   uint32_t num_gpus;
       pim_get_device_info(i,  PIM_GPU_CORES, sizeof(uint32_t), &num_gpus, NULL);
	   if ( num_gpus == 0 )
	   {
		   continue;
	   }
	   pimGPUTest[num_pim_gpu_spawned] = *passedTest;
       pimGPUTest[num_pim_gpu_spawned].pim_id = num_pim_gpu_spawned;
	   pimGPUTest[num_pim_gpu_spawned].test_id = 0;
	   pimGPUTest[num_pim_gpu_spawned].num_tests = 1;
       runTest[num_pim_gpu_spawned] = passedTest->tests[passedTest->test_id];
	   runTest[num_pim_gpu_spawned].test_seqs = hpg0;
	   pimGPUTest[num_pim_gpu_spawned].tests = &runTest[num_pim_gpu_spawned];

	   num_pim_gpu_spawned++;
   }
   
   for(int i = 0; i <  num_pim_gpu_spawned; i++)
   {
	   pimGPUTest[i].tests->problem_sz /= num_pim_gpu_spawned;
	   pimGpu[i] = new CCPUTest<_BASE_TYPE_>(pimGPUTest+ i);
	   assert(pimGpu[i]);
	   pimGpu[i]->Launch();
   }


   pimGpu[num_pim_gpu_spawned-1]->WaitForEvents(num_pim_gpu_spawned,(CPimTest<_BASE_TYPE_>**)pimGpu);
  

   if( pimGPUTest)
   {
	   delete [] pimGPUTest;
   }

   for( int i = 0; pimGpu && i < (int)passedTest->modelConfig.num_pims; i++ )
   {
	   if ( pimGpu[i] )
	   {
	       delete pimGpu[i];
	   }
   }

   if ( pimGpu )
   {
	   delete [] pimGpu;
   }

   if ( runTest ) 
   {
	   delete [] runTest;
   }
   return(NULL);
}



static
void * hpc1(void * arg)
{

const TestConfig * passedTest = (const TestConfig * )arg;
TestConfig *pimCPUTest = new TestConfig[passedTest->modelConfig.num_pims];
CPIMCPUTest<_BASE_TYPE_> **pimCpu = new CPIMCPUTest<_BASE_TYPE_> *[passedTest->modelConfig.num_pims];
RunableTestConfig * runTest = new RunableTestConfig[passedTest->modelConfig.num_pims];
   assert(pimCPUTest && pimCpu && runTest);
   memset(runTest, 0 , sizeof(RunableTestConfig) *passedTest->modelConfig.num_pims);
   for(int i = 0; i < (int)passedTest->modelConfig.num_pims; i++)
   {

	   pimCPUTest[i] = *passedTest;
       pimCPUTest[i].pim_id = i;
	   pimCPUTest[i].test_id = 0;
	   pimCPUTest[i].num_tests = 1;
	   runTest[i] = passedTest->tests[passedTest->test_id];
	   runTest[i].problem_sz /= passedTest->modelConfig.num_pims;
	   runTest[i].test_seqs = hc0;
	   pimCPUTest[i].tests = &runTest[i];
	   pimCpu[i] = new CPIMCPUTest<_BASE_TYPE_>(pimCPUTest+ i);
	   pimCpu[i]->Launch();
   }
   


   for(int i = 0; i < (int)passedTest->modelConfig.num_pims; i++)
   {
	   pimCpu[i]->Wait();
   }

   if( pimCPUTest)
   {
	   delete [] pimCPUTest;
   }

   for( int i = 0; pimCpu && i < (int)passedTest->modelConfig.num_pims; i++ )
   {
	   if ( pimCpu[i] )
	   {
	       delete pimCpu[i];
	   }
   }

   if ( pimCpu )
   {
	   delete [] pimCpu;
   }

   if ( runTest ) 
   {
	   delete [] runTest;
   }
   return(0);

}

static
void * pg1(void * arg)
{


const TestConfig * passedTest = (const TestConfig * )arg;
TestConfig *pimCPUTest = new TestConfig[passedTest->modelConfig.num_pims];
CPIMCPUTest<_BASE_TYPE_> **pimCpu = new CPIMCPUTest<_BASE_TYPE_> *[passedTest->modelConfig.num_pims];
RunableTestConfig * runTest = new RunableTestConfig[passedTest->modelConfig.num_pims];
int num_pim_gpu_spawned;

   assert(pimCPUTest && pimCpu && runTest);

   memset(pimCpu, 0, sizeof(CPIMCPUTest<_BASE_TYPE_> *) * passedTest->modelConfig.num_pims);
   memset(runTest, 0 , sizeof(RunableTestConfig) *passedTest->modelConfig.num_pims);
   num_pim_gpu_spawned = 0;
   for(int i = 0; i < (int)passedTest->modelConfig.num_pims; i++)
   {
   uint32_t num_gpus;
       pim_get_device_info(i,  PIM_GPU_CORES, sizeof(uint32_t), &num_gpus, NULL);
	   if ( num_gpus == 0 )
	   {
		   continue;
	   }

	   pimCPUTest[num_pim_gpu_spawned] = *passedTest;
       pimCPUTest[num_pim_gpu_spawned].pim_id = num_pim_gpu_spawned;
	   pimCPUTest[num_pim_gpu_spawned].test_id = 0;
	   pimCPUTest[num_pim_gpu_spawned].num_tests = 1;
       runTest[num_pim_gpu_spawned] = passedTest->tests[passedTest->test_id];
	   runTest[num_pim_gpu_spawned].test_seqs = hg0;
	   pimCPUTest[num_pim_gpu_spawned].tests = &runTest[num_pim_gpu_spawned];
	   num_pim_gpu_spawned++;

   }
   
   for(int i = 0; i <  num_pim_gpu_spawned; i++)
   {
	   pimCPUTest[i].tests->problem_sz /= num_pim_gpu_spawned;
	   pimCpu[i] = new CPIMCPUTest<_BASE_TYPE_>(pimCPUTest+ i);
	   assert(pimCpu[i]);
	   pimCpu[i]->Launch();
   }


   pimCpu[num_pim_gpu_spawned-1]->WaitForEvents(num_pim_gpu_spawned,(CPimTest<_BASE_TYPE_>**) pimCpu);

   if( pimCPUTest)
   {
	   delete [] pimCPUTest;
   }

   for( int i = 0; pimCpu && i < (int)passedTest->modelConfig.num_pims; i++ )
   {
	   if ( pimCpu[i] )
	   {
	       delete pimCpu[i];
	   }
   }

   if ( pimCpu )
   {
	   delete [] pimCpu;
   }

   if ( runTest ) 
   {
	   delete [] runTest;
   }
   return(0);

}






static
void usage()
{
            printf( "How to run:\n");
            printf( "app <options> <tests>\n");
            printf( "\nOptions:\n");
            printf( "   -ocl_file        GPU kernels file.\n" );
            printf( "   -ocl_loc         GPU kernels location.\n" );
            printf( "   -ocl_opt         GPU kernels compile options.\n" );
            printf( "   -sz              problem size.\n" );
            printf( "   -loops           loop limit.\n" );
            printf( "   -depth           invocation depth limit.\n" );
            printf( "\nTests:\n");
            printf( "   -hc0             host cpu simple test.\n" );
            printf( "   -hg0             host gpu simple test.\n" );
            printf( "   -hpg0            host-spawned simple PIM gpu test.\n" );
            printf( "   -hpg1            host-spawned PIM gpu test.\n" );
            printf( "   -hpc1            host-spawned PIM cpu test.\n" );
            printf( "   -pg1             PIM gpu test.\n" );

            printf( "   -h              print this message.\n\n");

            exit(0);
}



static
int parseOptions(int argc, char * argv[], TestConfig &testConfig)
{
int ret = 0;
int indx;
size_t problem_sz = 0;
int loops = 1;
int depth = 1;
char * ocl_loc = 0;
size_t loc_sz;
char * ocl_opt = 0;
size_t opt_sz;
char * ocl_file = 0;
size_t file_sz;
    for(indx = 1; indx < argc;indx++) 
    {   
		if ( !strcmp(argv[indx], "-sz") && (indx+1 < argc))
		{
			problem_sz = atoi(argv[++indx]);
		}
		if ( !strcmp(argv[indx], "-loops") && (indx+1 < argc))
		{
			loops = atoi(argv[++indx]);
		}
		if ( !strcmp(argv[indx], "-depth") && (indx+1 < argc))
		{
			depth = atoi(argv[++indx]);
		}
		if ( !strcmp(argv[indx], "-ocl_loc") && (indx+1 < argc))
		{
            loc_sz = strlen(argv[++indx]) + 1;
		    ocl_loc = (char*)malloc(loc_sz);
			assert(ocl_loc);
			strcpy(ocl_loc, argv[indx]);
		}

		if ( !strcmp(argv[indx], "-ocl_opt") && (indx+1 < argc))
		{
            opt_sz = strlen(argv[++indx]) + 1;
		    ocl_opt = (char*)malloc(opt_sz );
			assert(ocl_opt);
			strcpy(ocl_opt, argv[indx]);
		}

		if ( !strcmp(argv[indx], "-ocl_file") && (indx+1 < argc))
		{
            file_sz = strlen(argv[++indx]) + 1;
		    ocl_file = (char*)malloc(file_sz);
			assert(ocl_file);
			strcpy(ocl_file, argv[indx]);
		}

		for ( int t = 0;  test_nm[t] && test_seqs[t]; t++) 
		{
			if (!strcmp(test_nm[t],argv[indx]))
			{
				testConfig.tests[testConfig.num_tests].test_nm = (char*)malloc(strlen(test_nm[t]) + 1);

				assert(testConfig.tests[testConfig.num_tests].test_nm);
				strcpy((char*)testConfig.tests[testConfig.num_tests].test_nm, test_nm[t]);
				if( ocl_loc )
				{
                   testConfig.tests[testConfig.num_tests].kernel_loc = (char*)malloc(loc_sz);
				   assert(testConfig.tests[testConfig.num_tests].kernel_loc);
				   strcpy((char*)testConfig.tests[testConfig.num_tests].kernel_loc,ocl_loc);
				}
				if( ocl_opt )
				{
                   testConfig.tests[testConfig.num_tests].kernel_opions = (char*)malloc(opt_sz);
				   assert(testConfig.tests[testConfig.num_tests].kernel_opions);
				   strcpy((char*)testConfig.tests[testConfig.num_tests].kernel_opions,ocl_opt);
				}

				if( ocl_file )
				{
                   testConfig.tests[testConfig.num_tests].kernel_file = (char*) malloc(file_sz);
				   assert(testConfig.tests[testConfig.num_tests].kernel_file);
				   strcpy((char*)testConfig.tests[testConfig.num_tests].kernel_file,ocl_file);
				}

				testConfig.tests[testConfig.num_tests].problem_sz = problem_sz;
				testConfig.tests[testConfig.num_tests].loops = loops;
				testConfig.tests[testConfig.num_tests].depth = depth;
				testConfig.tests[testConfig.num_tests++].test_seqs = test_seqs[t];
			}
		}
	}
	if ( ocl_loc )
	{
		free(ocl_loc);
	}

	if ( ocl_opt )
	{
		free(ocl_opt);
	}

	if ( ocl_file )
	{
		free(ocl_file);
	}

    return(ret);
}



int main(int argc, char **argv)
{

TestConfig testConfig;

    memset(&testConfig, 0, sizeof(TestConfig));

int t;
    for ( t = 0;  test_nm[t] && test_seqs[t]; t++); 



	testConfig.tests = (RunableTestConfig*) new RunableTestConfig[t];

	assert(testConfig.tests);
	memset(testConfig.tests, 0, sizeof( RunableTestConfig) * t);

    parseOptions(argc, argv, testConfig);

	if ( testConfig.num_tests == 0 || testConfig.tests[0].problem_sz <= 0)
	{
		printf("PIM_TEST ERROR: Found not tests or problem size is unavaialble\n");
		usage();
	}

	find_and_map_pims(testConfig.modelConfig);

	 pthread_rwlock_init(&CPimTest<_BASE_TYPE_>::m_ocl_rwlock, NULL);
CCPUTest<_BASE_TYPE_> **testCpu = new CCPUTest<_BASE_TYPE_> *[testConfig.num_tests];
    assert(testCpu);
	for(int test = 0; test < testConfig.num_tests; test++)
	{

			testConfig.test_id = test;
			testCpu[test] = new CCPUTest<_BASE_TYPE_>(&testConfig);
			assert(testCpu[test] );
			
	}

	printf("***************************************************************\n");
	printf("********************* PIM STRESS TEST *************************\n");

	if ( testCpu[0]->m_kernel_file[0] != 0 )
	{
	   printf("GPU kernels file : %s\n", testCpu[0]->m_kernel_file);
	}
	if ( testCpu[0]->m_kernel_loc[0] != 0 )
	{
	   printf("GPU kernel file location : %s\n", testCpu[0]->m_kernel_loc);
	}
	if ( testCpu[0]->m_kernel_opt[0] != 0 )
	{
	   printf("GPU kernels compile options : %s\n", testCpu[0]->m_kernel_opt);
	}
	printf("Problem size : %d\n", testConfig.tests[0].problem_sz);
	printf("Number of loops : %d\n", testConfig.tests[0].loops);
	printf("Call depth : %d\n", testConfig.tests[0].depth);
	printf("Tests launched:\n");
    for(int i = 0; i < testConfig.num_tests; i++)
	{
		printf("%s\n",testConfig.tests[i].test_nm + 1);

	}
	printf("***************************************************************\n\n");

    pim_emu_begin();
	for(int test = 0; test < testConfig.num_tests; test++)
	{

		    testCpu[test]->Launch();
//			testCpu[test]->Wait();
			
	}

	testCpu[testConfig.num_tests-1]->WaitForEvents(testConfig.num_tests, (CPimTest<_BASE_TYPE_> **)testCpu);

    pim_emu_end();



long long lunchedThs = CPimTest<_BASE_TYPE_>::luanchedThreadsTotal();
long long sunkThs = CPimTest<_BASE_TYPE_>::sunkThreadsTotal();

    printf("Total threads launched : %lld, sunk : %lld\n", lunchedThs, sunkThs);


    for (int i = 0; i < testConfig.num_tests; i++ )
	{
		if ( testConfig.tests[i].test_nm )
		{
		free((char*)testConfig.tests[i].test_nm);
		}
		if ( testConfig.tests[i].kernel_loc )
		{
			free((char*)testConfig.tests[i].kernel_loc);
		}

		if ( testConfig.tests[i].kernel_file )
		{
			free((char*)testConfig.tests[i].kernel_file);
		}

		if ( testConfig.tests[i].kernel_opions )
		{
			free((char*)testConfig.tests[i].kernel_opions);
		}

	}

    if ( testConfig.tests )
	{
		delete [] testConfig.tests;
	}
    if ( testConfig.modelConfig.pim_cpu_cores )
	{
		delete [] testConfig.modelConfig.pim_cpu_cores;
	}

    if ( testConfig.modelConfig.pim_gpu_cores )
	{
		delete []  (testConfig.modelConfig.pim_gpu_cores);
	}

    if ( testConfig.modelConfig.list_of_pims )
	{
		delete [] (testConfig.modelConfig.list_of_pims);
	}
 }

