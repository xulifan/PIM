#ifndef HOTSPOT_H
#define HOTSPOT_H

#include "OpenCL_helper_library.h"

#include <CL/cl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>



#define BLOCK_SIZE 16
#define STR_SIZE 256
# define EXPAND_RATE 2// add one iteration will extend the pyramid base by 2 per each borderline

/* maximum power density possible (say 300W for a 10mm x 10mm chip)	*/
#define MAX_PD	(3.0e6)
/* required precision in degrees	*/
#define PRECISION	0.001
#define SPEC_HEAT_SI 1.75e6
#define K_SI 100
/* capacitance fitting factor	*/
#define FACTOR_CHIP	0.5


#define MIN(a, b) ((a)<=(b) ? (a) : (b))

#include <pim.h>
#include "pim_utility.cpp"


/* chip parameters	*/
const static float t_chip = 0.0005;
const static float chip_height = 0.016;
const static float chip_width = 0.016;
/* ambient temperature, assuming no package at all	*/
const static float amb_temp = 80.0;

// OpenCL globals
cl_context context;
cl_command_queue command_queue;
cl_device_id device;
cl_kernel kernel;

void writeoutput(float *, int, int, char *);
void readinput(float *, int, int, char *);
int compute_tran_temp(cl_mem, cl_mem[2], int, int, int, int, int, int, int, int, float *, float *);
void usage(int, char **);
void run(int, char **);

void pim_launch_hotspot_kernel(int iteration, void *power, void *temp_src, void *temp_dst, int grid_cols, int grid_rows, int border_cols, int border_rows, float Cap, float Rx, float Ry, float Rz, float step, int startx, int starty, int own_num_pointsx, int own_num_pointsy, pim_device_id target, cl_event *complete);
float * compute_tran_temp_pim(cl_mem MatrixPower, cl_mem MatrixTemp[2], int grid_cols, int grid_rows, \
		int total_iterations, int num_iterations, int blockCols, int blockRows, int borderCols, int borderRows,
		float *TempCPU, float *PowerCPU) ;

#endif
