#########################################
# PIM library and Tool Makefile
#########################################
ifeq ($(CC),cc)
CC=gcc
endif

# defaults 
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

CFLAGS=-rdynamic -pthread -Wall -O3 -I$(AMDAPPSDKROOT)/include -I$(CURDIR)/src/include
CXXFLAGS=$(CFLAGS) -std=c++0x -I$(CURDIR)
LDFLAGS=-lstdc++


# Shared library for CPU papi profiling 
CPUPROFILE_CSRC=${MODEL_DIR}/cpu_profile.c
CPUPROFILE_CPPSRC=
CPUPROFILE_COBJECTS=$(CPUPROFILE_CSRC:.c=.o)
CPUPROFILE_CPPOBJECTS=$(CPUPROFILE_CPPSRC:.cpp=.o)
CPUPROFILE_LINKING_LIBS= 


# Defalt install directory. This can be overwritten at the command line
prefix=/usr/local

user=$(shell whoami)

DESTDIR=
#####################
#    Common Rules   #
#####################
.PHONY: all
all: $(CPUPROFILE_COBJECTS) $(CPUPROFILE_CPPOBJECTS) $(COBJECTS) $(CPPOBJECTS) $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION) $(LIB_DIR)/$(OUTPUT_LIB_NAME).a $(BIN_DIR)/pim_amodel.exe

.PHONY: clean
clean:
	$(RM) -f *.pdf *.csv *.o *.so* *.a *~
	$(RM) -f $(COBJECTS)
	$(RM) -f $(TEST_DIR)/*.exe $(TEST_DIR)/*.o $(TEST_DIR)/*~ 
	$(RM) -f $(PIMEMU_DIR)/*.exe $(PIMEMU_DIR)/*.o $(PIMEMU_DIR)/*~ 
	$(RM) -f $(UTILS_DIR)/*.exe $(UTILS_DIR)/*.o $(UTILS_DIR)/*~ 
	$(RM) -f $(MODEL_DIR)/*.exe $(MODEL_DIR)/*.o $(MODEL_DIR)/*~ 
	$(RM) -f $(BIN_DIR)/*.exe $(BIN_DIR)/*.o $(BIN_DIR)/*~ 
	$(RM) -f $(LIB_DIR)/*
	$(RM) -f $(INC_DIR)/*

.PHONY: install
# Installation of this library is a bit weird.
# We need to make sure the user is either root or is not attempting to install into the default directory (/usr/local/lib requires root access)
# NOTE: This will still fail if the combination of DESTDIR and prefix put the destination into some root-controlled folder.
install: $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION) $(LIB_DIR)/$(OUTPUT_LIB_NAME).a
	install -D -p -m 755 $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION) $(DESTDIR)$(prefix)/lib/$(OUTPUT_LIB_NAME).so.$(VERSION)
	install -D -p -m 644 $(INC_DIR)/$(OUTPUT_LIB).h $(DESTDIR)$(prefix)/include/$(OUTPUT_LIB).h
	install -d $(DESTDIR)$(prefix)/bin
	install -D -p -m 755 $(BIN_DIR)/* $(DESTDIR)$(prefix)/bin
ifeq ($(DESTDIR)$(prefix),/usr/local)
ifneq ($(user),root)
	$(error Please run 'make install' as root, or change the 'prefix' variable to point to a destination that you control)
endif
	ldconfig -n $(DESTDIR)$(prefix)/lib/
else
	ln -f -s $(DESTDIR)$(prefix)/lib/$(OUTPUT_LIB_NAME).so.$(VERSION) $(DESTDIR)$(prefix)/lib/$(OUTPUT_LIB_NAME).so.$(MAJOR_VERSION)
	ln -f -s $(DESTDIR)$(prefix)/lib/$(OUTPUT_LIB_NAME).so.$(MAJOR_VERSION) $(DESTDIR)$(prefix)/lib/$(OUTPUT_LIB_NAME).so
endif

test: $(TEST_DIR)/pim_test.exe $(TEST_DIR)/test_xml_parse.exe $(TEST_DIR)/test_simple_cpu_pthread.exe $(TEST_DIR)/test_cpu_pthread.exe $(TEST_DIR)/test_gpu_opencl.exe $(BIN_DIR)/pim_amodel.exe  $(TEST_DIR)/test_gpu_modeling.exe $(TEST_DIR)/test_ehpocl_asyn.exe $(TEST_DIR)/test_pthread_modeling.exe $(TEST_DIR)/test_cpu_gpu.exe
	PIMEMUENV=$(CONFIG_DIR)/2.xml:0:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_xml_parse.exe $(CONFIG_DIR)
	PIMEMUENV=$(CONFIG_DIR)/2.xml:0:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_simple_cpu_pthread.exe
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/pim_test.exe -ocl_loc $(TEST_DIR) -sz 1000000 -loops 2 -depth 2 -hc0 -hg0 -hpc0 -hi0 -hpg0 -pg0
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_cpu_pthread.exe
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_gpu_opencl.exe $(TEST_DIR)
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_cpu_gpu.exe $(TEST_DIR) -ccounters 
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_gpu_opencl.exe $(TEST_DIR) -ccounters
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_ehpocl_asyn.exe $(TEST_DIR)
	PIMEMUENV=$(CONFIG_DIR)/2.xml:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(BIN_DIR)/pim_amodel.exe -gpu -prf  $(TEST_DIR)/Session_12.csv -outf performance_model_12.csv -ehp-c -freq 4000 -ehp-c0 -freq 1000 -ehp-g -cores 192 -pim1-c -freq 1500 -pim1-g -freq 800 -cores 12 -pim-m -freq 1000 -num_channels 2
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/test_pthread_modeling.exe

.PHONY: pim_test
pim_test: $(TEST_DIR)/pim_test.exe
	PIMEMUENV=$(CONFIG_DIR)/2.xml:1:$(PWD) LD_LIBRARY_PATH=$(LIBPAPI_DIR):$(LIBROXML_DIR):$(LIB_DIR):$(LD_LIBRARY_PATH) $(TEST_DIR)/pim_test.exe -ocl_loc $(TEST_DIR) -sz 1000000 -loops 2 -depth 2 -hc0 -hg0 -hpc0 -hi0 -hpg0 -pg0

$(TEST_DIR)/pim_test.exe: $(TEST_DIR)/pim_test_gpu.cpp $(TEST_DIR)/pim_test_main.cpp $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CXX) $(CXXFLAGS) -D_PIM_DEBUG_ $(LDLIBS) -I$(TEST_DIR) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -o $@ $^

$(TEST_DIR)/test_xml_parse.exe: $(TEST_DIR)/test_xml_parse.c $(UTILS_DIR)/xml_parse.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CC) $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -o $@ $^

$(TEST_DIR)/test_simple_cpu_pthread.exe: $(TEST_DIR)/test_simple_cpu_pthread.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CC) $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -o $@ $<

$(TEST_DIR)/test_cpu_pthread.exe: $(TEST_DIR)/test_cpu_pthread.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CC) $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -o $@ $<
 
$(TEST_DIR)/test_gpu_opencl.exe: $(TEST_DIR)/test_gpu_opencl.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -o $@ $<

$(TEST_DIR)/test_cpu_gpu.exe: $(TEST_DIR)/test_cpu_gpu.cc $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -o $@ $<

$(TEST_DIR)/test_ehpocl_asyn.exe: $(TEST_DIR)/test_ehpocl_asyn.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CC) $(CFLAGS)  -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -I$(TEST_DIR) -o $@ $<

$(BIN_DIR)/pim_amodel.exe: $(MODEL_DIR)/pimAModel.cpp $(MODEL_DIR)/gpuAModel.cpp $(MODEL_DIR)/cpuAModel.cpp $(MODEL_DIR)/pimOrderingModel.cpp $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)
	$(CXX) $(CXXFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -I$(TEST_DIR) -o $@ $^

$(TEST_DIR)/test_pthread_modeling.exe: $(TEST_DIR)/test_pthread_modeling.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)  
	$(CC) $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -I$(TEST_DIR) -o $@ $<

$(TEST_DIR)/test_gpu_modeling.exe: $(TEST_DIR)/test_gpu_opencl.c $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION)  
	$(CC) $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -L$(LIB_DIR) -l$(OUTPUT_LIB) -I$(TEST_DIR) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

$(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(VERSION): $(COBJECTS) $(CPPOBJECTS) $(CPUPROFILE_COBJECTS) $(CPUPROFILE_CPPOBJECTS)
	mkdir -p $(LIB_DIR)
	mkdir -p $(INC_DIR)
	$(CXX) -shared -Wl,-soname,$(OUTPUT_LIB_NAME).so.$(MAJOR_VERSION) -o $@ $^ $(LDFLAGS) -L$(LIBROXML_DIR) -lroxml -L$(LIBPAPI_DIR) -lpapi 
	ln -f -s $(OUTPUT_LIB_NAME).so.$(VERSION) $(LIB_DIR)/$(OUTPUT_LIB_NAME).so
	ln -f -s $(OUTPUT_LIB_NAME).so.$(VERSION) $(LIB_DIR)/$(OUTPUT_LIB_NAME).so.$(MAJOR_VERSION)
	cp -f $(CURDIR)/src/include/pim.h $(INC_DIR)

$(LIB_DIR)/$(OUTPUT_LIB_NAME).a: $(COBJECTS) $(CPPOBJECTS) $(CPUPROFILE_COBJECTS) $(CPUPROFILE_CPPOBJECTS)
	$(AR) cru $@ $+
