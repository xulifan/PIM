CC=g++

PIMEMU=/home/lifxu/Desktop/pimemu/lib
LIBROXML_DIR=/usr/lib64
AMDAPPSDKROOT=/opt/AMDAPP
DATA_DIRECTORY=/home/lifxu/Desktop/Benchmarks/rodinia_2.3/data
LIBPAPI_DIR=/usr/local/lib
LDLIBS=-ldl -lOpenCL
CFLAGS=-rdynamic -pthread -Wall -O3 -I$(AMDAPPSDKROOT)/include -I$(PIMEMU)/src/include
OUTPUT_LIB=pim
CONFIG_FILE=../../PIM_config/PIM_config.xml

PIMEMU_DIR=${PIMEMU}/src/pimemu
MODEL_DIR=${PIMEMU}/src/model
UTILS_DIR=${PIMEMU}/src/utils
TEST_DIR=$(PIMEMU)/test
CONFIG_DIR=$(PIMEMU)/config
BIN_DIR=${PIMEMU}/bin
LIB_DIR=${PIMEMU}/lib
INC_DIR=${PIMEMU}/include
