The directory structure is as follows
 
src/model - all the model (cpu, gpu, ordering) source files
src/pimemu - all the pim files, 
src/utils - xmlparser source file
src/include - all header files 
 
config/ all xml files
test/  - has all the test files (source and test headers) that we are using for validation including our toy programs, 
doc/ -  documentation and user guide
prereqs/ - this has the roxml source and patch
lib/ - the pim lib is copied here after successful make
bin/ - has the run script and the python graph script 
 

*************************************************
		COMPILATION
*************************************************
Default path for lib papi is assumed to be /usr/local/lib and lib roxml is /usr/lib64

To compile using default paths 
make 
make test  - this will compile the test program and execute a validation suite 

To compile with different paths specify path for papi using LIBPAPI_DIR and/or for lib roxml using LIBROXML_DIR 

make LIBPAPI_DIR=<path to lib papi> LIBROMXL_DIR=<path to roxml>
make test LIBPAPI_DIR=<path to lib papi> LIBROMXL_DIR=<path to lib roxml>

*************************************************
		INSTALLATION
*************************************************
Default install path is /usr/local
make install - default path is /usr/local , headers are copied to /usr/local/include libs are copied to /usr/local/lib and binaries are copied to /usr/local/bin

To change install path specify it with prefix
make install prefix=<install path>
 

***********************************************
		RUNNING EMULATION
***********************************************
After an application has been ported to the piemu api (see user guide and API
docs for details) the run script can be used  for running the emulation
including profiling modeling and producing the emulated run results

for example to run test_cpu_gpu application in the test dir of the build directory

cd test/
../bin/run_pim_emulation.sh -c ../config/PIM.xml -e ../install/ -o $HOME/tmp/ -t $HOME/tmp --run ./test_cpu_gpu.exe ./
will produce the following result files in $HOME/tmp : emulation_result.csv emulation_result.pdf

for help on the run script see user guide or run it with -h flag 
../bin/run_pim_emulation.sh -h 
