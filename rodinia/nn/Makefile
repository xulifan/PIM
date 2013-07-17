include ../common/common.mk

SOURCE_FILE=nearestNeighbor.cpp clutils.cpp utils.cpp
EXECUTABLE=nn.exe

all: 
	PIMEMUENV=$(CONFIG_FILE):1:$(PWD)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE) 

test:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./nn.exe filelist.txt -r 10 -lat 30 -lng 90
	
clean:
	rm -f *.o *~ $(EXECUTABLE) *.linkinfo
