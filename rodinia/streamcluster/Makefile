include ../common/common.mk

SOURCE_FILE=streamcluster.cpp
EXECUTABLE=streamcluster.exe

all: 
	PIMEMUENV=$(CONFIG_FILE):1:$(PWD)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE) 

test:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./streamcluster.exe 10 20 256 65536 65536 1000 none output.txt 1 -t gpu -d 0
	
clean:
	rm -f *.o *~ $(EXECUTABLE) *.linkinfo




