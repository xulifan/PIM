include ../common/common.mk

SOURCE_FILE=hotspot.c hotspot.h OpenCL_helper_library.c OpenCL_helper_library.h
EXECUTABLE=hotspot

all: 
	PIMEMUENV=$(CONFIG_FILE_1GPU):1:$(PWD)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE) 

test:  
	run_pim_emulation.sh -c $(CONFIG_FILE_3GPU) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./hotspot 512 2 2 $(DATA_DIRECTORY)/hotspot/temp_512 $(DATA_DIRECTORY)/hotspot/power_512 output.out
	
clean:
	rm -f *.o *~ $(EXECUTABLE) *.linkinfo



