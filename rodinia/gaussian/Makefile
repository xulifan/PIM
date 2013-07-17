include ../common/common.mk

SOURCE_FILE=clutils.cpp gaussianElim.cpp utils.cpp
EXECUTABLE=gaussian.exe

all: 
	PIMEMUENV=$(CONFIG_FILE):1:$(PWD)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE) 

test:  
	run_pim_emulation.sh -c $(CONFIG_FILE) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./gaussian.exe $(DATA_DIRECTORY)/gaussian/matrix208.txt
	
clean:
	rm -f *.o *~ $(EXECUTABLE) *.linkinfo






