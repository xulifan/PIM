/*******************************************************************************
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 *
 * RESTRICTED RIGHTS NOTICE (DEC 2007)
 * (a)     This computer software is submitted with restricted rights under
 *     Government Contract No. DE-AC52-8MA27344 and subcontract B600716. It
 *     may not be used, reproduced, or disclosed by the Government except as
 *     provided in paragraph (b) of this notice or as otherwise expressly
 *     stated in the contract.
 *
 * (b)     This computer software may be -
 *     (1) Used or copied for use with the computer(s) for which it was
 *         acquired, including use at any Government installation to which
 *         the computer(s) may be transferred;
 *     (2) Used or copied for use with a backup computer if any computer for
 *         which it was acquired is inoperative;
 *     (3) Reproduced for safekeeping (archives) or backup purposes;
 *     (4) Modified, adapted, or combined with other computer software,
 *         provided that the modified, adapted, or combined portions of the
 *         derivative software incorporating any of the delivered, restricted
 *         computer software shall be subject to the same restricted rights;
 *     (5) Disclosed to and reproduced for use by support service contractors
 *         or their subcontractors in accordance with paragraphs (b)(1)
 *         through (4) of this notice; and
 *     (6) Used or copied for use with a replacement computer.
 *
 * (c)     Notwithstanding the foregoing, if this computer software is
 *         copyrighted computer software, it is licensed to the Government with
 *         the minimum rights set forth in paragraph (b) of this notice.
 *
 * (d)     Any other rights or limitations regarding the use, duplication, or
 *     disclosure of this computer software are to be expressly stated in, or
 *     incorporated in, the contract.
 *
 * (e)     This notice shall be marked on any reproduction of this computer
 *     software, in whole or in part.
 ******************************************************************************/

#ifndef __PIMAMODEL_HPP_
#define __PIMAMODEL_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <streambuf>  
#include <map>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <functional>
#include <vector>  
#include <cmath>


using namespace std;

#include "pim-private.h"

#ifdef __cplusplus
//extern "C" {
#endif
/******************************************************************************/
typedef vector<cpu_configuration_t> cpu_conf_t;
typedef vector<gpu_configuration_t> gpu_conf_t;
typedef vector<dram_configuration_t> dram_conf_t;
typedef struct {
	uint32_t id;
	cpu_conf_t cpus;
	gpu_conf_t gpus;
	dram_conf_t dram;
} pim_conf_t;
typedef vector<pim_conf_t> pims_conf_t;

typedef struct {
	cpu_conf_t cpus;
	gpu_conf_t gpus;
	pims_conf_t pims;
} ehp_conf_t;



/*****************************************************************************/


int  gpuAModel (const char * conf_file_,
	            const char *in_file_,
				const char* out_file_,
				const pim_emu_configuration_t *pim_config_,
				ehp_conf_t *ehp_conf_);
int  cpuAModel (const char * conf_file_,
	            const char *in_file_,
				const char* out_file_,
				const pim_emu_configuration_t *pim_config_,
				ehp_conf_t *ehp_conf_, uint32_t profild_freq, double profiled_latency);
int pimOrderingModel (const char * conf_file_,
        const char *input_order_file_,
        const char *input_cpu_perf_file_,
        const char *input_gpu_perf_file_,
        char *out_file_,
        const pim_emu_configuration_t *pim_config_,
        ehp_conf_t *ehp_conf_);

double getPimGpuComputeThroughput(const pim_configuration_t *pim_, int gpu_id, const ehp_conf_t *input_conf_);
double getPimDramBandwidth(const pim_configuration_t *pim_, ehp_conf_t *input_conf_);
double getPimDramLatency(const pim_configuration_t *pim_, const ehp_conf_t *input_conf_);
double getPimDramOffStackLatency(const pim_configuration_t *pim_, const ehp_conf_t *input_conf_);

double getPimDramBandwidth(const dram_configuration_t * dram_confug_);
double getPimDramOffChipBandwidth(const dram_configuration_t * dram_confug_);
double getEHPMemBandwidth(const pim_emu_configuration_t *configuration_);
double getGPUComputeTroughput(const gpu_configuration_t * gpu_config_);

double getPimDramBandwidth(const pim_configuration_t *pim_, const ehp_conf_t *input_conf_);
double getEHPMemBandwidth(const pim_emu_configuration_t *configuration_, const ehp_conf_t *input_conf_);
double getGPUComputeTroughput(const gpu_configuration_t * model_config_, const gpu_conf_t *input_conf_);

double getEHPLatency(const pim_emu_configuration_t *configuration_);
double getEHPLatency(const pim_emu_configuration_t *configuration_, const ehp_conf_t *input_conf);
double getCPUFrequency(const cpu_configuration_t * model_config_, const cpu_conf_t *input_conf_);

void tokenize( std::vector<std::string>& tokens, const std::string& text, const std::string& delimiter);
int getIdIndex(const char **id_array_, string & Id_);
string getStatLine(const char **id_array_, vector<string> & Set_, string & id);
long long getStatLong(const char **id_array_, vector<string> & Set_, string & id);
double getStatDouble_helper(vector<string>& Set_, int index);
double getStatDouble(const char **id_array_, vector<string> & Set_, string & id);

cpu_configuration_t * findById(cpu_conf_t * conf_, int dev_id_);
gpu_configuration_t * findById(gpu_conf_t * conf_, int dev_id_);
dram_configuration_t * findById(dram_conf_t * conf_, int dev_id_);
pim_conf_t * findById(pims_conf_t *conf_, int dev_id_);
cpu_configuration_t * findConfPtr(cpu_conf_t * conf_, int dev_id_);
gpu_configuration_t * findConfPtr(gpu_conf_t * conf_, int dev_id_);
dram_configuration_t * findConfPtr(dram_conf_t * conf_, int dev_id_);
pim_conf_t * findConfPtr(pims_conf_t * conf_, int dev_id_);

#ifdef __cplusplus
//}
#endif

#endif  /* __PIMAMODEL_HPP_ */
