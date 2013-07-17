CC=g++

OPENCL_DIR = /opt/AMDAPP
OPENCL_INC = $(OPENCL_DIR)/include/ 
OPENCL_LIB = $(OPENCL_DIR)/lib/x86_64/ -lOpenCL
PIMEMU=/home/lifxu/Desktop/pimemu/lib
LIBROXML_DIR=/usr/lib64
AMDAPPSDKROOT=/opt/AMDAPP
DATA_DIRECTORY=/home/lifxu/Desktop/Benchmarks/rodinia_2.3/data
LIBPAPI_DIR=/usr/local/lib
LDLIBS=-ldl -lOpenCL
CFLAGS=-rdynamic -pthread -Wall -O3 -I$(AMDAPPSDKROOT)/include -I$(PIMEMU)/src/include -I../common
OUTPUT_LIB=pim
CONFIG_FILE=../../PIM_config/PIM_config.xml
CONFIG_FILE_1GPU=../../PIM_config/PIM_config_1gpu.xml
CONFIG_FILE_2GPU=../../PIM_config/PIM_config_2gpu.xml
CONFIG_FILE_3GPU=../../PIM_config/PIM_config_3gpu.xml

PIMEMU_DIR=${PIMEMU}/src/pimemu
MODEL_DIR=${PIMEMU}/src/model
UTILS_DIR=${PIMEMU}/src/utils
TEST_DIR=$(PIMEMU)/test
CONFIG_DIR=$(PIMEMU)/config
BIN_DIR=${PIMEMU}/bin
LIB_DIR=${PIMEMU}/lib
INC_DIR=${PIMEMU}/include
