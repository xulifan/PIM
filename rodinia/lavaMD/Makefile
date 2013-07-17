include ../common/common.mk

SOURCE_FILE=main.c
EXECUTABLE=lavaMD

all: 
	PIMEMUENV=$(CONFIG_FILE):1:$(PWD)
	$(CC) -D_PROPERTIES_ -D_NO_CPU_COUNTERS_ $(CFLAGS) $(LDLIBS) -I$(INC_DIR) -I./ -L$(LIB_DIR) -l$(OUTPUT_LIB) $(SOURCE_FILE)  -o $(EXECUTABLE) 

test:  
	run_pim_emulation.sh -c $(CONFIG_FILE_3GPU) -e ~/Desktop/pimemu/lib -t ./tmp -o ./result --run ./lavaMD -boxes1d 10

clean:
	rm -f *.o *~ $(EXECUTABLE) *.linkinfo


