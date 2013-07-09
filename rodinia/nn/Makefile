CC=g++

PIMEMU=/home/lifxu/Desktop/pimemu/lib
LIBROXML_DIR=/usr/lib64
AMDAPPSDKROOT=/opt/AMDAPP


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

CONFIG_FILE=./PIM_config.xml
SOURCE_FILE=nearestNeighbor.cpp clutils.cpp utils.cpp
EXECUTABLE=nn.exe

all: 
	PIMEMUENV=$(CONFIG_FILE):1:$(PWD)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE) 

test:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./nn.exe filelist.txt -r 5 -lat 30 -lng 90
	
clean:
	rm -f *.o *~ $(EXECUTABLE) *.linkinfo
