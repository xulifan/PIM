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

#include "pimAModel.hpp"

/*************************************************************************/

static void updateConf(gpu_configuration_t *actual_conf_, const gpu_configuration_t *input_conf_)
{
	
    actual_conf_->freq = (input_conf_->freq != 0) ?input_conf_->freq : actual_conf_->freq; 
	actual_conf_->cus = (input_conf_->cus != 0) ?input_conf_->cus : actual_conf_->cus;
    actual_conf_->width = (input_conf_->width != 0) ?input_conf_->width : actual_conf_->width;
    actual_conf_->num_instr_perclck = (input_conf_->num_instr_perclck != 0) ?input_conf_->num_instr_perclck : actual_conf_->num_instr_perclck;
}

static void updateModelConf(gpu_configuration_t *actual_conf_, const gpu_configuration_t * model_config_, const gpu_conf_t *input_conf_)
{
 // accumulate static model config with current ruuning config passed through application arguments
       *actual_conf_ = *model_config_;
const gpu_configuration_t *arg_conf = findById((gpu_conf_t *)input_conf_, -1);
// generic gpu config?

       if ( arg_conf )
	   {
		    updateConf(actual_conf_, arg_conf );
	   }

// specific gpu id?
	   arg_conf = findById((gpu_conf_t *)input_conf_, actual_conf_->id);

       if ( arg_conf )
	   {
		    updateConf(actual_conf_, arg_conf );
	   }	
}
static void updateConf(cpu_configuration_t *actual_conf_, const cpu_configuration_t *input_conf_)
{
	
    actual_conf_->freq = (input_conf_->freq != 0) ?input_conf_->freq : actual_conf_->freq; 
}

static void updateModelConf(cpu_configuration_t *actual_conf_, const cpu_configuration_t * model_config_, const cpu_conf_t *input_conf_)
{
 // accumulate static model config with current ruuning config passed through application arguments
       *actual_conf_ = *model_config_;
const cpu_configuration_t *arg_conf = findById((cpu_conf_t *)input_conf_, -1);
// generic gpu config?

       if ( arg_conf )
	   {
		    updateConf(actual_conf_, arg_conf );
	   }

// specific gpu id?
	   arg_conf = findById((cpu_conf_t *)input_conf_, actual_conf_->id);

       if ( arg_conf )
	   {
		    updateConf(actual_conf_, arg_conf );
	   }	
}


static void updateConf(dram_configuration_t *actual_conf_, const dram_configuration_t *input_conf_)
{
	
    actual_conf_->freq = (input_conf_->freq != 0) ?input_conf_->freq : actual_conf_->freq; 
	actual_conf_->num_channels = (input_conf_->num_channels != 0) ?input_conf_->num_channels : actual_conf_->num_channels;
    actual_conf_->channel_width = (input_conf_->channel_width != 0) ?input_conf_->channel_width : actual_conf_->channel_width;
    actual_conf_->bits_percycle = (input_conf_->bits_percycle != 0) ?input_conf_->bits_percycle : actual_conf_->bits_percycle;
    actual_conf_->latency = (input_conf_->latency != 0) ?input_conf_->latency : actual_conf_->latency;
    actual_conf_->off_stack_latency = (input_conf_->off_stack_latency != 0) ?input_conf_->off_stack_latency : actual_conf_->off_stack_latency;
    actual_conf_->utilization = (input_conf_->utilization != 0) ?input_conf_->utilization : actual_conf_->utilization;
    actual_conf_->off_stack_bw = (input_conf_->off_stack_bw != 0) ?input_conf_->off_stack_bw : actual_conf_->off_stack_bw;
}

static void updateModelConf(dram_configuration_t *actual_conf_, const dram_configuration_t * model_config_, const dram_conf_t *input_conf_)
{
 // accumulate static model config with current ruuning config passed through application arguments
       *actual_conf_ = *model_config_;  
	   dram_configuration_t *arg_conf = findById((dram_conf_t *)input_conf_, -1);
// generic gpu config?

       if ( arg_conf )
	   {
		    updateConf(actual_conf_, arg_conf );
	   }

// specific gpu id?
	   arg_conf = findById((dram_conf_t *)input_conf_, actual_conf_->id);

       if ( arg_conf )
	   {
		    updateConf(actual_conf_, arg_conf );
	   }	
}

double getPimDramBandwidth(const dram_configuration_t * dram_confug_)
{
double mem_bandwidth_avail = ((double)dram_confug_->freq * 1000000. * 
                               dram_confug_->num_channels *
							   dram_confug_->channel_width * 
							   dram_confug_->bits_percycle *
							   (double)dram_confug_->utilization/100.
							   ) / 8; // bytes
   
     return(mem_bandwidth_avail);
}

double getPimDramOffChipBandwidth(const dram_configuration_t * dram_confug_)
{
double mem_bandwidth_avail = 
	 getPimDramBandwidth(dram_confug_) * (double)dram_confug_->off_stack_bw /100.;  
     return(mem_bandwidth_avail);
}

double getEHPMemBandwidth(const pim_emu_configuration_t *configuration_)
{
double mem_bandwidth_avail = 0;
    for (int p = 0; p < (int)configuration_->num_pim_cores; p++)  {
        // one context per PIM
        pim_configuration_t *pim;
        
        pim = configuration_->pims + p;
		mem_bandwidth_avail += getPimDramOffChipBandwidth(pim->dram);

    }      
	return(mem_bandwidth_avail);
}

double getPimDramLatency(const dram_configuration_t * conf) {

	double latency = (double) conf->latency;
	return latency;

}
double getPimDramOffStackLatency(const dram_configuration_t *conf ) {

	double latency = conf->latency * (1+ (double)conf->off_stack_latency/100.);
	return (latency);
}

double getEHPMemLatency(const pim_emu_configuration_t *configuration_)
{
double mem_latency = 0;
    for (int p = 0; p < (int)configuration_->num_pim_cores; p++)  {
        // one context per PIM
        pim_configuration_t *pim;
        
        pim = configuration_->pims + p;
		mem_latency += getPimDramOffStackLatency(pim->dram);

    }      
	return(mem_latency/(int)configuration_->num_pim_cores);
}
double getPimDramBandwidth(const pim_configuration_t *pim_, const ehp_conf_t *input_conf_)
{
pim_conf_t * gerneric_arg_pim = findById((pims_conf_t *)&input_conf_->pims, -1);
dram_configuration_t actual_conf;
pim_conf_t * specific_arg_pim =  findById((pims_conf_t *)&input_conf_->pims, pim_->id);

		actual_conf = *pim_->dram;
// generic
		if ( gerneric_arg_pim  )
		{
			updateModelConf(&actual_conf, pim_->dram, &gerneric_arg_pim->dram);
		}
// specific
		if ( specific_arg_pim )
		{
			updateModelConf(&actual_conf, &actual_conf, &specific_arg_pim->dram);
		}

double mem_bandwidth_avail = getPimDramBandwidth(&actual_conf);
   
     return(mem_bandwidth_avail);
}

double getPimDramLatency(const pim_configuration_t *pim_, const ehp_conf_t *input_conf_)
{
	pim_conf_t * gerneric_arg_pim = findById((pims_conf_t *)&input_conf_->pims, -1);
	dram_configuration_t actual_conf;
	pim_conf_t * specific_arg_pim =  findById((pims_conf_t *)&input_conf_->pims, pim_->id);

		actual_conf = *pim_->dram;
// generic
		if ( gerneric_arg_pim  )
		{
			updateModelConf(&actual_conf, pim_->dram, &gerneric_arg_pim->dram);
		}
// specific
		if ( specific_arg_pim )
		{
			updateModelConf(&actual_conf, &actual_conf, &specific_arg_pim->dram);
		}

	return (getPimDramLatency(&actual_conf));
   
}
 
double getPimDramOffStackLatency(const pim_configuration_t *pim_, const ehp_conf_t *input_conf_)
{
pim_conf_t * gerneric_arg_pim = findById((pims_conf_t *)&input_conf_->pims, -1);
dram_configuration_t actual_conf;
pim_conf_t * specific_arg_pim =  findById((pims_conf_t *)&input_conf_->pims, pim_->id);

		actual_conf = *pim_->dram;
// generic
		if ( gerneric_arg_pim  )
		{
			updateModelConf(&actual_conf, pim_->dram, &gerneric_arg_pim->dram);
		}
// specific
		if ( specific_arg_pim )
		{
			updateModelConf(&actual_conf, &actual_conf, &specific_arg_pim->dram);
		}

	return(getPimDramOffStackLatency(&actual_conf));
}

double getPimGpuComputeThroughput(const pim_configuration_t *pim_, int gpu_id, const ehp_conf_t *input_conf_)
{
pim_conf_t * gerneric_arg_pim = findById((pims_conf_t *)&input_conf_->pims, -1);
gpu_configuration_t actual_conf;
pim_conf_t * specific_arg_pim =  findById((pims_conf_t *)&input_conf_->pims, pim_->id);
 
		actual_conf = pim_->gpus[gpu_id];
// generic
		if ( gerneric_arg_pim )
		{
			updateModelConf(&actual_conf, &pim_->gpus[gpu_id], &gerneric_arg_pim->gpus);
		}
// specific
		if ( specific_arg_pim )
		{
			updateModelConf(&actual_conf, &actual_conf, &specific_arg_pim->gpus);
		}

double compute_bandwidth_avail = getGPUComputeTroughput(&actual_conf);
   
     return(compute_bandwidth_avail);
}


double getEHPMemBandwidth(const pim_emu_configuration_t *configuration_, const ehp_conf_t *input_conf_)
{
double mem_bandwidth_avail = 0;
    pim_conf_t * gerneric_arg_pim = findById((pims_conf_t *)&input_conf_->pims, -1);



    for (int p = 0; p < (int)configuration_->num_pim_cores; p++)  {

        dram_configuration_t actual_conf;
        pim_configuration_t *pim;
        
        pim = configuration_->pims + p;
		pim_conf_t * specific_arg_pim =  findById((pims_conf_t *)&input_conf_->pims, pim->id);


		actual_conf = *pim->dram;
// generic
		if ( gerneric_arg_pim )
		{
			updateModelConf(&actual_conf, pim->dram, &gerneric_arg_pim->dram);
		}
// specific
		if ( specific_arg_pim )
		{
			updateModelConf(&actual_conf, &actual_conf, &specific_arg_pim->dram);
		}

		mem_bandwidth_avail += getPimDramOffChipBandwidth(&actual_conf);

    }      
	return(mem_bandwidth_avail);
}

double getEHPLatency(const pim_emu_configuration_t *configuration_, const ehp_conf_t *input_conf_)
{
double mem_latency = 0;
    pim_conf_t * gerneric_arg_pim = findById((pims_conf_t *)&input_conf_->pims, -1);



    for (int p = 0; p < (int)configuration_->num_pim_cores; p++)  {

        dram_configuration_t actual_conf;
        pim_configuration_t *pim;
        
        pim = configuration_->pims + p;
		pim_conf_t * specific_arg_pim =  findById((pims_conf_t *)&input_conf_->pims, pim->id);


		actual_conf = *pim->dram;
// generic
		if ( gerneric_arg_pim )
		{
			updateModelConf(&actual_conf, pim->dram, &gerneric_arg_pim->dram);
		}
// specific
		if ( specific_arg_pim )
		{
			updateModelConf(&actual_conf, &actual_conf, &specific_arg_pim->dram);
		}

		mem_latency += getPimDramOffStackLatency(&actual_conf);

    }      
	return(mem_latency/(int)configuration_->num_pim_cores);
}
double getGPUComputeTroughput(const gpu_configuration_t * gpu_config_)
{
 double engine_clock_avail;
	   engine_clock_avail =  (double)gpu_config_->freq * 1000000. * gpu_config_->cus * gpu_config_->width;
	   return(engine_clock_avail);
}

double getGPUComputeTroughput(const gpu_configuration_t * model_config_, const gpu_conf_t *input_conf_)
{
 double engine_clock_avail;
 gpu_configuration_t actual_conf;
       updateModelConf(&actual_conf, model_config_, input_conf_);
	   engine_clock_avail =  (double)actual_conf.freq * 1000000. * actual_conf.cus * actual_conf.width;
	   return(engine_clock_avail);
}

double getCPUFrequency(const cpu_configuration_t * cpu_config_, const cpu_conf_t *input_conf_)
{
 	cpu_configuration_t actual_conf;
	
       	updateModelConf(&actual_conf, cpu_config_, input_conf_);

	return((double)actual_conf.freq);
	
}

/**************************************************************************/
void tokenize( std::vector<std::string>& tokens,
        const std::string& text,
        const std::string& delimiter )
{
    size_t next_pos = 0;
    size_t init_pos = text.find_first_not_of( delimiter, next_pos );

    while ( next_pos != std::string::npos &&
            init_pos != std::string::npos ) {
        // Get next delimiter position
        next_pos = text.find( delimiter, init_pos );

        std::string token = text.substr( init_pos, next_pos - init_pos );
        tokens.push_back( token );

        init_pos = text.find_first_not_of( delimiter, next_pos );
    }
}

int getIdIndex(const char **id_array_, string & Id_)
{
    int ret = 0;
    for(int i = 0; id_array_[i][0] != 0; i++) {
        if ( !Id_.compare(id_array_[i])) {
            ret = i;
            break;
        }
    }
    return(ret);
}

string getStatLine(const char **id_array_, vector<string> & Set_, string & id)
{
    string ret;
    int indx = getIdIndex(id_array_, id);
    ret = Set_[indx];
    return(ret);
}

long long getStatLong(const char **id_array_, vector<string> & Set_, string & id)
{
    long long ret;
    int indx = getIdIndex(id_array_, id);
    string ret_str = Set_[indx];
    ret = stoll(ret_str);
    return(ret);
}

double getStatDouble_helper(vector<string>& Set_, int index)
{
    double ret = stod(Set_[index]);
#ifndef WIN32
    if (!isfinite(ret)) {
        cerr << "Error: trying to bring in a double statistic that is not finite." << endl;
        cerr << "Column " << index+1 << " in the line: " << endl;
        for (unsigned int i = 0; i < Set_.size(); i++) {
            cerr << Set_[i] << " ";
        }
        cerr << endl;
        exit(-1);
    }
#endif
    return ret;
}

double getStatDouble(const char **id_array_, vector<string> & Set_, string & id)
{
    double ret;
    int indx = getIdIndex(id_array_, id);
    ret = getStatDouble_helper(Set_, indx);
    return(ret);
}

char *concat_directory_and_name(const char *directory, const char *name)
{
    size_t directory_length, name_length;
    char* return_str;

    // Directory is guaranteed to have a slash after it.
    directory_length = strlen(directory);
    name_length = strlen(name);

    return_str = (char*)malloc(sizeof(char) * (directory_length + name_length + 1));
    if (return_str == NULL) {
        fprintf(stderr, "Unable to allocate Directory+Name Concat. Die.\n");
        exit(-1);
    }

    return_str[(directory_length + name_length)] = '\0';
    strcpy(return_str, directory);
    strcpy((return_str+directory_length), name);
    return return_str;
}


char* fix_directory(char *input)
{
    char *directory;
    size_t directory_length;

    directory_length = strlen(input);

    if (input[directory_length-1] != '/' && input[directory_length-1] != '\\')
        directory_length += 1;

    directory = (char*)malloc(sizeof(char) * (directory_length + 1));
    if (directory == NULL) {
        fprintf(stderr, "Unable to allocate directory string. Die.\n");
        exit(-1);
    }
    directory[directory_length] = '\0';

    strcpy(directory, input);
    if (input[directory_length] != '/')
        directory[directory_length-1] = '/';
	return(directory);
}


/************************************************************************************/




cpu_configuration_t * findById(cpu_conf_t * conf_, int dev_id_)
{
cpu_configuration_t * ret = 0;

   for(int i = 0; i < (int)conf_->size(); i++)
   {
	    if ( (*conf_)[i].id == (uint32_t)dev_id_ )
		{
			  ret = &((*conf_)[i]);
			  break;
		}
   }
   return(ret);
}


gpu_configuration_t * findById(gpu_conf_t * conf_, int dev_id_)
{
gpu_configuration_t * ret = 0;


   for(int i = 0; i < (int)conf_->size(); i++)
   {
	    if ( (*conf_)[i].id == (uint32_t)dev_id_ )
		{
			  ret = &((*conf_)[i]);
			  break;
		}
   }
   return(ret);
}


dram_configuration_t * findById(dram_conf_t * conf_, int dev_id_)
{
dram_configuration_t * ret = 0;

   for(int i = 0; i < (int)conf_->size(); i++)
   {
	    if ( (*conf_)[i].id == (uint32_t)dev_id_ )
		{
			  ret = &((*conf_)[i]);
			  break;
		}
   }
   return(ret);
}



pim_conf_t * findById(pims_conf_t *conf_, int dev_id_)
{
pim_conf_t * ret = 0;

   for(int i = 0; i < (int)conf_->size(); i++)
   {
	    if ( (*conf_)[i].id == (uint32_t)dev_id_ )
		{
			  ret = &((*conf_)[i]);
			  break;
		}
   }
   return(ret);
}


cpu_configuration_t * findConfPtr(cpu_conf_t * conf_, int dev_id_)
{
cpu_configuration_t * ret = 0;
cpu_configuration_t tmp_cnf;
					// find
    ret = findById(conf_, dev_id_);
	memset(&tmp_cnf, 0, sizeof(cpu_configuration_t));

// if specific and not found: new
	if ( !ret )
	{
		conf_->push_back(tmp_cnf);
		ret = &((*conf_)[conf_->size() - 1]);
	}

	ret->id = dev_id_;

	return ret;
}

gpu_configuration_t * findConfPtr(gpu_conf_t * conf_, int dev_id_)
{
gpu_configuration_t * ret = 0;
gpu_configuration_t tmp_cnf;
					// find
    ret = findById(conf_, dev_id_);
	memset(&tmp_cnf, 0, sizeof(gpu_configuration_t));

// if specific and not found: new
	if ( !ret )
	{
		conf_->push_back(tmp_cnf);
		ret = &((*conf_)[conf_->size() - 1]);
	}

	ret->id = dev_id_;

	return ret;
}

dram_configuration_t * findConfPtr(dram_conf_t * conf_, int dev_id_)
{
dram_configuration_t * ret = 0;
dram_configuration_t tmp_cnf;
					// find
    ret = findById(conf_, dev_id_);
	memset(&tmp_cnf, 0, sizeof(dram_configuration_t));

// if specific and not found: new
	if ( !ret )
	{
		conf_->push_back(tmp_cnf);
		ret = &((*conf_)[conf_->size() - 1]);
	}

	ret->id = dev_id_;

	return ret;
}

pim_conf_t * findConfPtr(pims_conf_t * conf_, int dev_id_)
{
pim_conf_t * ret = 0;
pim_conf_t tmp_cnf;
					// find
    ret = findById(conf_, dev_id_);

// if specific and not found: new
	if ( !ret )
	{
		conf_->push_back(tmp_cnf);
		ret = &((*conf_)[conf_->size() - 1]);
	}

	ret->id = dev_id_;

	return ret;
}


// -ehp-c[id] -freq val
// -ehp-g[id] -freq val -cus val
// -pim[id]-c[id] -freq
// -pim[id]-g[id] -freq val -cus val
// -pim[id]-m[id] -freq val -num_channels val -channel_width val -bits_percycle val
//  -latency val -utilization val -off_stack_bw val -off_stack_letency val

static
void usage()
{
            printf( "How to run:\n");
            printf( "app -options\n");
            printf( "\nOptions:\n");
            printf( "   -total                     total simulator output based on ordering file, CPU, and GPU perf traces\n");
            printf( "   -cpu_perf                  performance output from the scaled cpu model\n");
            printf( "   -gpu_perf                  performance output from the scaled gpu model\n");
            printf( "   -order_trace               ordering trace for this benchmark run\n");
            printf( "   -cpu or -gpu               device type.\n" );
 //           printf( "   -cnf <name>                xml config file location.\n" );
            printf( "   -prf <name>                perf counters file location.\n" );
            printf( "   -evf <name>                events trace file location.\n" );
            printf( "   -outf <name>               output file location.\n" );
	    printf( "   -profiled_freq value       CPU core freq in MHz of the machine where profile was collected, default is 2500 MHz \n");
	    printf( "   -profiled_latency value    memory access latency in nanoseocnds  where profile was collected, default is 52.5 ns\n"); 
	    printf( "   -ehp-c[c-id] options       ehp parmeters for a c-id cpu core. omit id to replicate parmeters to all cpus.\n" );
	    printf( "   -ehp-g[g-id] options       ehp parmeters for a g-id gpu core. omit id to replicate parmeters to all gpus.\n" );
	    printf( "   -pim[id]-c[c-id] options   pim[id] parmeters for a c-id cpu core. omit id to replicate parmeters.\n" );
	    printf( "   -pim[id]-g[g-id] options   pim[id] parmeters for a g-id gpu core. omit id to replicate parmeters.\n" );
	    printf( "   -pim[id]-m[m-id] options   pim[id] parmeters for a m-id dram stack. omit id to replicate parmeters.\n\n" );
	    printf( "    cpu options:\n" );
	    printf( "   -freq value                core frequency in MHz.\n\n" );
	    printf( "    gpu options:\n" );
            printf( "   -freq value                core frequency in MHz.\n" );
            printf( "   -cus value               number of CUs.\n\n" );
	    printf( "    memory options:\n" );
            printf( "   -freq value                frequency in MHz.\n" );
            printf( "   -num_channels value        number of channels.\n" );
            printf( "   -channel_width value       channel width (bits).\n" );
            printf( "   -bits_percycle value       bits per cycle.\n" );
            printf( "   -latency value             estimated latency in nsec (%%).\n" );
            printf( "   -off_stack_latency value   estimated off_stack latency degradation (%%).\n" );
            printf( "   -utilization value         estimated utilization from the peak (%%).\n" );
            printf( "   -off_stack_bw value        estimated off_chip bandwidth degradation (%%).\n\n" );

            printf( "   -h                         print this message.\n\n");

            exit(0);
}

static
int isOptions(void * option_, int *option_sz_,
	             char * option_nm_,
				 int argc_, char * argv_[])
{
int ret = 0;

    for(int indx = 1; indx < argc_;indx++) 
    {
      if ( !strcmp(option_nm_,argv_[indx]) )
	{
        if ( !strcmp(option_nm_, "-cpu") || !strcmp(option_nm_, "-gpu") ||
                !strcmp(option_nm_, "-total") )
	    {
	    }
	  if ( (!strcmp(option_nm_, "-cnf") 
		|| !strcmp(option_nm_, "-prf")
		|| !strcmp(option_nm_, "-outf")
		|| !strcmp(option_nm_, "-evf")
        || !strcmp(option_nm_, "-cpu_perf")
        || !strcmp(option_nm_, "-gpu_perf")
        || !strcmp(option_nm_, "-order_trace"))
	       && (indx+1) < argc_ && option_ )
	    {
	      strcpy((char*)option_, argv_[indx + 1]);
	    }
	  if(!strcmp(option_nm_, "-profiled_freq"))
	    {
	      *(uint32_t *)option_ = stoul(argv_[indx+1]);
	    }
	  if( !strcmp(option_nm_,"-profiled_latency"))
	    {
	      *(double *)option_ = stod(argv_[indx+1]);
	    }
	  ret = 1;
	  break;
	  
	}
    }
    return(ret);
}



static
int parseModelOptions(ehp_conf_t * ehp_conf_,
	     			  int argc, char * argv[])
{
int ret = 0;
int indx;
int ehp = 0, cpu = 0, gpu = 0, dram = 0, pim = 0, pim_id = -1;
int dev_id = -1;
    for(indx = 2; indx < argc;indx++) 
    {   

		if ( strncmp(argv[indx], (char*)"-ehp", 4) == 0 )
		{

		    cpu = 0;
		    gpu = 0;
		    if (  (indx+1 >= argc)
				  ||
				  (
				  !(cpu = (strncmp(argv[indx] + 4, (char*)"-c", 2)) == 0)
				       && !(gpu = (strncmp(argv[indx] + 4, (char*)"-g", 2)) == 0)
				  )
				)
		      {
			fprintf(stderr,
				"Error: %s (%d): in -ehp argument %d 4\n",
				__func__, __LINE__, indx);
			usage();
		      }
		    // ehp processing
		    ehp = 1;
		    pim = 0;
		    // generic ehp sub id = -1
		    dev_id = -1;

		    if ( argv[indx][6] != 0 )
		      {
			dev_id = atoi(argv[indx] + 6);
		      }
		}
		else if ( strncmp(argv[indx], (char*)"-pim", 4) == 0 )
		  {
		    // pim processing
		    ehp = 0;
		    pim = 1;
		    // generic pim id == -1
		    pim_id = -1;
		    // generic = -1
		    dev_id = -1;
		    // pim id
		    char * dash_ptr = strchr ( argv[indx] + 4, '-' );
		    int dash_pos; 
		    if ( indx+1 >= argc || !dash_ptr )
		      {
			fprintf(stderr,
				"Error: %s (%d): in -pim argument %d 4\n",
				__func__, __LINE__, indx);
			usage();
		      }
		    if ( (dash_pos = (int)(dash_ptr - argv[indx] - 4)) > 0)
		      { 
			char tmp_buf[1024];
			memcpy(tmp_buf, argv[indx] + 4, dash_pos);
			tmp_buf[dash_pos] = 0;
			pim_id = atoi(tmp_buf);
		      }

		    cpu = 0;
		    gpu = 0;
		    dram = 0;
		    // sub systems
		    if ( !(cpu = (argv[indx][dash_pos + 5] == 'c'))
			 && !(gpu = (argv[indx][dash_pos + 5] == 'g'))
			 && !(dram = (argv[indx][dash_pos + 5] == 'm')) )
		      {
			fprintf(stderr,
				"Error: %s (%d): in -pim argument %d %d\n",
				__func__, __LINE__, indx, dash_pos + 1);
			usage();
		      }
		    if ( argv[indx][dash_pos + 6] != 0 )
		      {
			dev_id = atoi(argv[indx] + dash_pos + 6);
		      }
		  }
		else if ( strcmp(argv[indx], (char*)"-freq") == 0 )
		  {
		    if ( (!ehp && !pim) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }
		    // it's ehp argument
		    if ( ehp )
		      {
			// it's cpu
			if ( cpu )
			  {
			    cpu_configuration_t *conf_ptr = 
			      findConfPtr(&ehp_conf_->cpus, dev_id);
			    
			    conf_ptr->freq = atoi(argv[indx+1]);
			  }
			// it's gpu
			if ( gpu )
			  {
			    gpu_configuration_t *conf_ptr = 
			      findConfPtr(&ehp_conf_->gpus, dev_id);
			    
			    conf_ptr->freq = atoi(argv[indx+1]);
			    
			  }
			
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's cpu
			if ( cpu )
			  {
			    cpu_configuration_t *conf_ptr = 
			      findConfPtr(&pim_conf->cpus, dev_id);
			    
			    conf_ptr->freq = atoi(argv[indx+1]);
			  }
			// it's gpu
			if ( gpu )
			  {
			    gpu_configuration_t *conf_ptr = findConfPtr(&pim_conf->gpus, dev_id);
			    
			    conf_ptr->freq = atoi(argv[indx+1]);
			    
			  }
			  // it's gram
			if (dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->freq = atoi(argv[indx+1]);

			  }

		      }

		  }
		else if ( strcmp(argv[indx], (char*)"-cus") == 0 )
		  {
		    if ( (!ehp && !pim) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }
		    // it's ehp argument
		    if ( ehp )
		      {
			// it's cpu
			if ( cpu )
			  {
			    // error right now
			    fprintf(stderr,
				    "Error: %s (%d): invalid argument %d: \n",
				    __func__, __LINE__, indx);
			    usage();
			  }
			// it's gpu
			if ( gpu )
			  {
			    gpu_configuration_t *conf_ptr = findConfPtr(&ehp_conf_->gpus, dev_id);
			    conf_ptr->cus = atoi(argv[indx+1]);
			  }

		      }
		    
		    // it's pim argument
		    if ( pim )
		      {
 
			// it's cpu or dram
			if ( cpu || dram )
			  {
			    // error right now
			    fprintf(stderr,
				    "Error: %s (%d): invalid argument %d: \n",
				    __func__, __LINE__, indx);
			    usage();
			  }
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( gpu )
			  {
			    gpu_configuration_t *conf_ptr = findConfPtr(&pim_conf->gpus, dev_id);

			    conf_ptr->cus = atoi(argv[indx+1]);
			    
			  }
			
		      }


		  }
		else if ( strcmp(argv[indx], (char*)"-num_channels") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->num_channels = atoi(argv[indx+1]);

			  }

		      }


		  }
		else if ( strcmp(argv[indx], (char*)"-channel_width") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
				{
				  dram_configuration_t *conf_ptr =  findConfPtr(&pim_conf->dram, dev_id);

				  conf_ptr->channel_width = atoi(argv[indx+1]);

				}

		      }


		  }
		else if ( strcmp(argv[indx], (char*)"-bits_percycle") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->bits_percycle = atoi(argv[indx+1]);

			  }

		      }


		  }
		
		else if ( strcmp(argv[indx], (char*)"-latency") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->latency = atoi(argv[indx+1]);

			  }

		      }

		  }
		else if ( strcmp(argv[indx], (char*)"-utilization") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->utilization = atoi(argv[indx+1]);

			  }
			
		      }


		  }
		else if ( strcmp(argv[indx], (char*)"-off_stack_bw") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->off_stack_bw = atoi(argv[indx+1]);

			  }

		      }


		  }
		else if ( strcmp(argv[indx], (char*)"-off_stack_latency") == 0 )
		  {
		    if ( !pim || ehp || (pim && !dram) || (indx+1 >= argc) )
		      {
			fprintf(stderr,
				"Error: %s (%d): invalid argument %d\n",
				__func__, __LINE__, indx);
			usage();
		      }

		    // it's pim argument
		    if ( pim )
		      {
			pim_conf_t * pim_conf = findConfPtr(&ehp_conf_->pims, pim_id);
			// it's gpu
			if ( dram )
			  {
			    dram_configuration_t *conf_ptr = findConfPtr(&pim_conf->dram, dev_id);

			    conf_ptr->off_stack_latency = atoi(argv[indx+1]);

			  }

		      }


		  }


    }
    return(ret);
}




int main(int argc, char **argv)
{

  int gpu_model = isOptions(NULL, NULL, (char*)"-gpu", argc, argv);
  int cpu_model = isOptions(NULL, NULL, (char*)"-cpu", argc, argv);
  int order_model = isOptions(NULL, NULL, (char*)"-total", argc, argv);

  uint32_t profiled_freq = 2500;
  double profiled_latency = 52.5;
  int pfreq = isOptions(&profiled_freq, NULL, (char*)"-profiled_freq", argc, argv);
  int platency = isOptions(&profiled_latency, NULL, (char*)"-profiled_latency", argc, argv);


    if ( !gpu_model && !cpu_model && !order_model)
     {
       fprintf(stderr,
	       "Error: %s (%d): type of device has not been set: -cpu, -gpu, or -total\n",
	       __func__, __LINE__);
       usage();
     }
   ehp_conf_t ehp_conf;
   parseModelOptions(&ehp_conf, argc, argv);

   const char *conf_file;
   const char * tmp_file_loc;
   char * perf_file = 0;
   char * out_file = 0;
   char * cpu_perf_file = 0;
   char * gpu_perf_file = 0;
   char * order_file = 0;
   char * tmp_buf = 0;

   const pim_emu_configuration_t *pim_configuration;
#if 0
   int failure, xml_filed;

    // Parse out information from conf_file into this configuration
   xml_filed = open(conf_file, O_RDONLY);
   if (xml_filed < 0)
     return PIM_INIT_FILE_ACCESS_ERROR;
   // parse_pim_config() does an roxml_close() which actually fcloses the fd
   failure = parse_pim_config(xml_filed, &pim_configuration);

   if (failure != PIM_SUCCESS)
     {
       return failure;
     }
#endif

   pim_configuration = getEmuArchitectureConfig();
   conf_file = getEmuArchitectureConfigFile();
   tmp_file_loc = getEmuTempFilesLocation();

   perf_file = (char*)malloc(2048);
   out_file = (char*)malloc(2048);
   cpu_perf_file = (char*)malloc(2048);
   gpu_perf_file = (char*)malloc(2048);
   order_file = (char*)malloc(2048);
   tmp_buf = (char*)malloc(2048);

   if (!pim_configuration || !conf_file || !tmp_file_loc || !perf_file ||  !out_file || !tmp_buf )
     {
       fprintf(stderr,
	       "Error: %s (%d): working files have not been define\n",
	       __func__, __LINE__);
       
       usage();
     }    


   int prf = isOptions(perf_file, NULL,
		       (char*)"-prf",
		       argc, argv);

   int outf = isOptions(out_file, NULL,
			(char*)"-outf",
			argc, argv);
   int cpu_perf = isOptions(cpu_perf_file, NULL,
           (char*)"-cpu_perf", argc, argv);
   int gpu_perf = isOptions(gpu_perf_file, NULL,
           (char*)"-gpu_perf", argc, argv);
   int order_trace = isOptions(order_file, NULL,
           (char*)"-order_trace", argc, argv);

   if ( order_model ) {
       if (!order_trace || !cpu_perf) {
           fprintf(stderr,
                   "Error: %s (%d): working files have not been define\n",
                   __func__, __LINE__);
           usage();
       }
   }
   else if ( !prf  || !outf ) {
       fprintf(stderr,
               "Error: %s (%d): working files have not been define\n",
               __func__, __LINE__);
       usage();
   }
// if passed with directory, use it
// otherwise use temp from library

   {
	char * pos1 = strchr(perf_file, '/');
	char * pos2 = strrchr(perf_file, '\\');

	   if ( !pos1 && !pos2 )
	   {
//FIXME !!! stupid but does not work otherwise
           strcpy(tmp_buf,tmp_file_loc);
           strcat(tmp_buf, perf_file );
	       strcpy(perf_file,tmp_buf);
	   }
   }

   {
	char * pos1 = strchr(out_file, '/');
	char * pos2 = strrchr(out_file, '\\');

	   if ( !pos1 && !pos2 )
	   {
           strcpy(tmp_buf,tmp_file_loc);
           strcat( tmp_buf, out_file );
	       strcpy(out_file,tmp_buf);
	   }
   }

     if ( gpu_model )
     {	   
       gpuAModel (conf_file, perf_file, out_file, pim_configuration, &ehp_conf);
     }

     if ( cpu_model )
     {	   
       cpuAModel (conf_file, perf_file, out_file, pim_configuration, &ehp_conf, profiled_freq, profiled_latency);
     }
     if ( order_model )
     {
         if (gpu_perf)
             pimOrderingModel(conf_file, order_file, cpu_perf_file, gpu_perf_file, out_file, pim_configuration, &ehp_conf);
         else
             pimOrderingModel(conf_file, order_file, cpu_perf_file, NULL, out_file, pim_configuration, &ehp_conf);
     }
   
     if (perf_file)
	 {
		 free(perf_file);
	 }

     if (out_file)
	 {
		 free(out_file);
	 }
	 if (tmp_buf )
	 {
		 free(tmp_buf );
	 }
 }
