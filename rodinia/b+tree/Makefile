include ../common.mk

OPENCL_DIR = /opt/AMDAPP
OPENCL_INC = $(OPENCL_DIR)/include/ 
OPENCL_LIB = $(OPENCL_DIR)/lib/x86_64/ -lOpenCL
ifeq ($(shell uname -m), x86_64)
     ifeq ($(shell if test -d $(OPENCL_DIR)/lib/x86_64/; then echo T; else echo F; fi), T)
     	OPENCL_LIB = $(OPENCL_DIR)/lib/x86_64/
     endif
endif

C_C = gcc
OCL_LIB = -lOpenCL


b+tree.out:	./main.o \
		./kernel/kernel_gpu_opencl_wrapper.o \
		./kernel/kernel_gpu_opencl_wrapper_2.o \
		./util/timer/timer.o \
		./util/num/num.o \
		./util/opencl/opencl.o
		PIMEMUENV=$(CONFIG_FILE):1:$(PWD)
	$(C_C)	./main.o \
			./kernel/kernel_gpu_opencl_wrapper.o \
			./kernel/kernel_gpu_opencl_wrapper_2.o \
			./util/timer/timer.o \
			./util/num/num.o \
			./util/opencl/opencl.o \
			-lm \
			-L$(OPENCL_LIB) \
			-lOpenCL \
			-D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) \
	                -o b+tree.out
main.o:	./common.h \
		./main.h \
		./main.c
	$(C_C)	./main.c \
			-c \
			-I$(AMDAPPSDKROOT)/include -std=c99\
			-o ./main.o \
			-O3


./kernel/kernel_gpu_opencl_wrapper.o:	./common.h \
										./kernel/kernel_gpu_opencl_wrapper.h \
										./kernel/kernel_gpu_opencl_wrapper.c
	$(C_C)	./kernel/kernel_gpu_opencl_wrapper.c \
			-c \
			-o ./kernel/kernel_gpu_opencl_wrapper.o \
			-O3 \
			-I$(OPENCL_INC)

./kernel/kernel_gpu_opencl_wrapper_2.o:	./common.h \
										./kernel/kernel_gpu_opencl_wrapper_2.h \
										./kernel/kernel_gpu_opencl_wrapper_2.c
	$(C_C)	./kernel/kernel_gpu_opencl_wrapper_2.c \
			-c \
			-o ./kernel/kernel_gpu_opencl_wrapper_2.o \
			-O3 \
			-I$(OPENCL_INC)


./util/timer/timer.o:	./common.h \
						./util/timer/timer.h \
						./util/timer/timer.c
	$(C_C)	./util/timer/timer.c \
			-c \
			-o ./util/timer/timer.o \
			-O3

./util/num/num.o:	./common.h \
					./util/num/num.h \
					./util/num/num.c
	$(C_C)	./util/num/num.c \
			-c \
			-o ./util/num/num.o \
			-O3

./util/opencl/opencl.o:	./common.h \
						./util/opencl/opencl.h \
						./util/opencl/opencl.c
	$(C_C)	./util/opencl/opencl.c \
			-c \
			-o ./util/opencl/opencl.o \
			-O3 \
			-I$(OPENCL_INC)


test:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./b+tree.out file $(DATA_DIRECTORY)/b+tree/mil.txt command $(DATA_DIRECTORY)/b+tree/command.txt


clean:
	rm	*~ *.o *.out \
		./kernel/*.o \
		./kernel/*~ \
		./util/timer/*.o \
		./util/num/*.o \
		./util/opencl/*.o \
                output.txt

