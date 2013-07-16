CC=g++

PIMEMU=/home/lifxu/Desktop/pimemu/lib
LIBROXML_DIR=/usr/lib64
AMDAPPSDKROOT=/opt/AMDAPP
DATA_DIRECTORY=/home/lifxu/Desktop/Benchmarks/rodinia_2.3/data

PIMEMU_DIR=${PIMEMU}/src/pimemu
MODEL_DIR=${PIMEMU}/src/model
UTILS_DIR=${PIMEMU}/src/utils
TEST_DIR=$(PIMEMU)/test
CONFIG_DIR=$(PIMEMU)/config
BIN_DIR=${PIMEMU}/bin
LIB_DIR=${PIMEMU}/lib
INC_DIR=${PIMEMU}/include
LIBPAPI_DIR=/usr/local/lib


LDLIBS=-ldl -lOpenCL
CFLAGS=-rdynamic -pthread -Wall -O3 -I$(AMDAPPSDKROOT)/include -I$(PIMEMU)/src/include
OUTPUT_LIB=pim

CONFIG_FILE=../../PIM_config/PIM_config.xml
SOURCE_FILE=
EXECUTABLE=


ALL: OCL_particlefilter_naive OCL_particlefilter_double OCL_particlefilter_single
	PIMEMUENV=$(CONFIG_FILE):1:$(PWD)

OCL_particlefilter_naive: ex_particle_OCL_naive_seq.o
	$(CC)  $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) ex_particle_OCL_naive_seq.o  -o OCL_particlefilter_naive 

OCL_particlefilter_double: ex_particle_OCL_double_seq.o
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) ex_particle_OCL_double_seq.o -o OCL_particlefilter_double 

OCL_particlefilter_single: ex_particle_OCL_single_seq.o
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) ex_particle_OCL_single_seq.o -o OCL_particlefilter_single 


%.o: %.[ch]
	$(CC) $(CC_FLAGS) $< -c

ex_particle_OCL_naive_seq.o: ex_particle_OCL_naive_seq.cpp
	$(CC) $(CFLAGS) $(LDLIBS) ex_particle_OCL_naive_seq.cpp -c

ex_particle_OCL_double_seq.o: ex_particle_OCL_double_seq.cpp
	$(CC) $(CFLAGS) $(LDLIBS) ex_particle_OCL_double_seq.cpp -c

ex_particle_OCL_single_seq.o: ex_particle_OCL_single_seq.cpp
	$(CC) $(CFLAGS) $(LDLIBS) ex_particle_OCL_single_seq.cpp -c

testnaive:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./OCL_particlefilter_naive -x 128 -y 128 -z 10 -np 1000

testdouble:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./OCL_particlefilter_double -x 128 -y 128 -z 10 -np 20000

testsingle:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./OCL_particlefilter_single -x 128 -y 128 -z 10 -np 40000

clean:
	rm -f *.o *~ OCL_particlefilter_naive OCL_particlefilter_double OCL_particlefilter_single *.linkinfo
