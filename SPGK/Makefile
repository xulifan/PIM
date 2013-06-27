CC=g++

PIMEMU=/home/lifxu/Desktop/pimemu/lib
PIMEMU_DIR=${PIMEMU}/src/pimemu
MODEL_DIR=${PIMEMU}/src/model
UTILS_DIR=${PIMEMU}/src/utils
TEST_DIR=$(PIMEMU)/test
CONFIG_DIR=$(PIMEMU)/config
BIN_DIR=${PIMEMU}/bin
LIB_DIR=${PIMEMU}/lib
INC_DIR=${PIMEMU}/include
LIBPAPI_DIR=/usr/local/lib
LIBROXML_DIR=/usr/lib64
AMDAPPSDKROOT=/opt/AMDAPP

LDLIBS=-ldl
CFLAGS=-rdynamic -pthread -Wall -O3 -I$(AMDAPPSDKROOT)/include -I$(PIMEMU)/src/include
OUTPUT_LIB=pim


SOURCE_FILE=spk.c
EXECUTABLE=spk.exe

all:
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) 
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE)

run0:
	run_pim_emulation.sh -c $(CONFIG_DIR)/2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./$(EXECUTABLE) -directory /home/lifxu/Desktop/github/personal/graph_gen/output -platform 1 -gk 0 -option 0 -vertex 0 -gauss 0.1
run1:
	run_pim_emulation.sh -c $(CONFIG_DIR)/2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./$(EXECUTABLE) -directory /home/lifxu/Desktop/github/personal/graph_gen/output -platform 1 -gk 0 -option 1 -vertex 0 -gauss 0.1
run2:
	run_pim_emulation.sh -c $(CONFIG_DIR)/2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./$(EXECUTABLE) -directory /home/lifxu/Desktop/github/personal/graph_gen/output -platform 1 -gk 0 -option 2 -vertex 0 -gauss 0.1
run3:
	run_pim_emulation.sh -c $(CONFIG_DIR)/2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./$(EXECUTABLE) -directory /home/lifxu/Desktop/github/personal/graph_gen/output -platform 1 -gk 0 -option 3 -vertex 0 -gauss 0.1
run4:
	run_pim_emulation.sh -c $(CONFIG_DIR)/2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./$(EXECUTABLE) -directory /home/lifxu/Desktop/github/personal/graph_gen/output -platform 1 -gk 0 -option 4 -vertex 0 -gauss 0.1
run5:
	run_pim_emulation.sh -c $(CONFIG_DIR)/2.xml -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./$(EXECUTABLE) -directory /home/lifxu/Desktop/github/personal/graph_gen/output -platform 1 -gk 0 -option 5 -vertex 0 -gauss 0.1
clean:
	rm -rf *~ $(EXECUTABLE)
