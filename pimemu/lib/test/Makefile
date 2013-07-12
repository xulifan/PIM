CC=gcc

CURDIR=/home/lifxu/Desktop/pimemu/lib
PIMEMU_DIR=${CURDIR}/src/pimemu
MODEL_DIR=${CURDIR}/src/model
UTILS_DIR=${CURDIR}/src/utils
TEST_DIR=$(CURDIR)/test
CONFIG_DIR=$(CURDIR)/config
BIN_DIR=${CURDIR}/bin
LIB_DIR=${CURDIR}/lib
INC_DIR=${CURDIR}/include
LIBPAPI_DIR=/usr/local/lib
LIBROXML_DIR=/usr/lib64

# Name of all the source files we need for the simulator
CSOURCES=${PIMEMU_DIR}/pim.c ${UTILS_DIR}/xml_parse.c $(UTILS_DIR)/validate_config.c ${PIMEMU_DIR}/pthread_cpu_emulate.c ${PIMEMU_DIR}/PIMEMU.c ${PIMEMU_DIR}/pimemu_clinterceptor.c ${PIMEMU_DIR}/pimemu_pthreadinterceptor.c
CPPSOURCES= ${PIMEMU_DIR}/opencl_gpu_emulate.cpp ${PIMEMU_DIR}/pimg.cpp ${PIMEMU_DIR}/psg_shared.cpp 

# PIM tool related files
COBJECTS=$(CSOURCES:.c=.o) 
CPPOBJECTS=$(CPPSOURCES:.cpp=.o)

# Name of the final library we want and its version number
OUTPUT_LIB=pim
OUTPUT_LIB_NAME=lib$(OUTPUT_LIB)
MAJOR_VERSION=1
MINOR_VERSION=0
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

# Shared libraries that are required for the PIM simulator
LDLIBS=-ldl

CFLAGS=-rdynamic -pthread -Wall -O3  -std=c99 -I$(AMDAPPSDKROOT)/include -I$(CURDIR)/src/include
CXXFLAGS=$(CFLAGS) -std=c99 -I$(CURDIR)
LDFLAGS=-lstdc++


# Shared library for CPU papi profiling 
CPUPROFILE_CSRC=${MODEL_DIR}/cpu_profile.c
CPUPROFILE_CPPSRC=
CPUPROFILE_COBJECTS=$(CPUPROFILE_CSRC:.c=.o)
CPUPROFILE_CPPOBJECTS=$(CPUPROFILE_CPPSRC:.cpp=.o)
CPUPROFILE_LINKING_LIBS= 



SOURCE_FILE=lifan_test.c
EXECUTABLE=lifan_test.exe

test:
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) 
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE)
run:
	run_pim_emulation.sh -c 2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./lifan_test.exe
clean:
	rm -rf *~ $(EXECUTABLE)
