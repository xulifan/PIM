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

#define __DEFAULT_FILE__ "..\\..\\test\\Session_12.csv"
#define __DEFUALT_CONF__ "3.xml"
#define __VISUAL_TEST__ 1
#define __NUM_OF_LINES_VISUAL__ 32

typedef struct {
  double time;
} kernel_perf;

typedef struct {
  long long counter;
  long long global_sz;
  double time;
  double v_intrs;
  double s_instr;
  double f_instr;
  double l_instr;
  double w_instr;
  double f_size;
  double w_size;
  double mu_busy;

} kernel_stat;

/*
Method, ExecutionOrder, ThreadID, CallIndex, GlobalWorkSize, WorkGroupSize, Time, LocalMemSize, VGPRs,
SGPRs, ScratchRegs, FCStacks, Wavefronts, VALUInsts, SALUInsts, VFetchInsts, SFetchInsts, VWriteInsts,
LDSInsts, VALUUtilization, VALUBusy, SALUBusy, FetchSize, CacheHit, MemUnitBusy, MemUnitStalled,
WriteUnitStalled, LDSBankConflict, WriteSize, GDSInsts
*/
static const char * sHeaders[] = {
    "Method" ,
	"ExecutionOrder" ,
	"ThreadID" , 
	"CallIndex" ,
	"GlobalWorkSize" ,
	"WorkGroupSize" , 
	"Time" , 
	"LocalMemSize" ,
	"VGPRs" ,
	"SGPRs" ,
	"ScratchRegs" ,
	"FCStacks" ,
	"Wavefronts",
	"VALUInsts" ,
	"SALUInsts" ,
	"VFetchInsts" , 
	"SFetchInsts" , 
	"VWriteInsts" , 
	"LDSInsts" ,
	"VALUUtilization",
	"VALUBusy" , 
	"SALUBusy" , 
	"FetchSize" , 
	"CacheHit" , 
	"MemUnitBusy" ,
	"MemUnitStalled" ,
	"WriteUnitStalled" ,
	"LDSBankConflict" ,
	"WriteSize" ,
	"GDSInsts",
	""
};

static long long getGlobalSzVal(vector<string> & Set_)
{
long long ret = 0;
string tmp("GlobalWorkSize");
string glbl_str = getStatLine(sHeaders, Set_, tmp);
vector<string> dims;
int pos = (int)glbl_str.find_first_of('{');
   glbl_str = glbl_str.substr(pos+1, glbl_str.size() - pos - 1);
   pos = (int)glbl_str.find_last_of('}');
   glbl_str = glbl_str.substr(0, pos);
   tokenize( dims, glbl_str, " " );
   ret = stoll(dims[0]) * stoll(dims[1]) * stoll(dims[1]);
   return(ret);
}


/*******************************************************************************************/

static void getScaledPerformance(kernel_perf * perf_estimate_,
	                             const kernel_stat * kern_,
								 const gpu_configuration_t * gpu_config_,
								 const dram_configuration_t * dram_confug_)
{
 double engine_time;
 double engine_clock_req;
 double engine_clock_avail;
 double mem_time;

 double alu_ins = (kern_->v_intrs < kern_->s_instr)? kern_->s_instr : kern_->v_intrs;
       engine_clock_req = (double)kern_->global_sz * alu_ins / (double)gpu_config_->num_instr_perclck;
	   engine_clock_avail =  getGPUComputeTroughput(gpu_config_);
	   engine_time = engine_clock_req / engine_clock_avail;

 double mem_bandwith_req = (kern_->f_size + kern_->w_size) * 1024; // bytes
 double mem_bandwidth_avail = getPimDramBandwidth(dram_confug_); // bytes
	     mem_time = mem_bandwith_req / mem_bandwidth_avail;
		 
		 perf_estimate_->time = ( engine_time < mem_time) ? mem_time : engine_time;

}


static void getScaledPerformance(kernel_perf * perf_estimate_,
	                             const kernel_stat * kern_,
								 const pim_configuration_t *pim_,
								 int gpu_id,
								 const ehp_conf_t *input_conf_)
{
 double engine_time;
 double engine_clock_req;
 double engine_clock_avail;
 double mem_time;

 double alu_ins = (kern_->v_intrs < kern_->s_instr)? kern_->s_instr : kern_->v_intrs;

       engine_clock_req = (double)kern_->global_sz * alu_ins / (double)pim_->gpus[gpu_id].num_instr_perclck;
	   engine_clock_avail =  getPimGpuComputeThroughput(pim_, gpu_id,input_conf_);
	   engine_time = engine_clock_req / engine_clock_avail;

 double mem_bandwith_req = (kern_->f_size + kern_->w_size) * 1024; // bytes
 double mem_bandwidth_avail =  getPimDramBandwidth(pim_,input_conf_); // bytes
	     mem_time = mem_bandwith_req / mem_bandwidth_avail;
		 
		 perf_estimate_->time = ( engine_time < mem_time) ? mem_time : engine_time;

}


static void getScaledPerformance(kernel_perf * perf_estimate_,
	                             const kernel_stat * kern_,
								 double engine_clock_avail_,
								 int32_t num_instr_perclck_,
								 double mem_bandwidth_avail_)
{
 double engine_time;
 double engine_clock_req;
 double engine_clock_avail;
 double mem_time;

 double alu_ins = (kern_->v_intrs < kern_->s_instr)? kern_->s_instr : kern_->v_intrs;
       engine_clock_req = (double)kern_->global_sz * alu_ins / (double)num_instr_perclck_;
	   engine_clock_avail =  engine_clock_avail_;
	   engine_time = engine_clock_req / engine_clock_avail;

 double mem_bandwith_req = (kern_->f_size + kern_->w_size) * 1024; // bytes
 double mem_bandwidth_avail = mem_bandwidth_avail_; // bytes
	     mem_time = mem_bandwith_req / mem_bandwidth_avail;
		 
		 perf_estimate_->time = ( engine_time < mem_time) ? mem_time : engine_time;

}


/*********************************************************************************************/

int  gpuAModel (const char * conf_file_,
	            const char *in_file_,
				const char* out_file_,
				const pim_emu_configuration_t *pim_config_,
				ehp_conf_t * ehp_conf_
				)
{

kernel_stat line_stat;
string key_stat;


const char *in_file = in_file_;
const char *out_file = out_file_;
const char *conf_file = conf_file_;

kernel_perf * gpu_perf_scaling;

// count PIM GPUs
int pim_gpu_count = 0;
    for (int g = 0; g < (int)pim_config_->num_pim_cores; g++)  {
        // one context per PIM
        pim_configuration_t *pim;
        
        pim = pim_config_->pims + g;
		pim_gpu_count += pim->num_gpu_cores;

    }      
  
double EHPMembandwidth = getEHPMemBandwidth(pim_config_, ehp_conf_);
double EHPgpuThroughput =  getGPUComputeTroughput(pim_config_->ehp.gpus, &ehp_conf_->gpus);

   cout << "\nExecuting PIM GPU performance analytical model...\n";
   cout << "---------------------------------------------------\n";
   cout << "model config file: " << conf_file <<".\n";
   cout << "model profiling file: " << in_file <<".\n";
   cout << "model output file: " << out_file <<".\n";
   cout << "Number of PIM GPUs modeled: " << pim_gpu_count <<".\n\n";


    gpu_perf_scaling = (kernel_perf*)calloc(pim_gpu_count + 1, sizeof(kernel_perf));



    ifstream in( in_file );
    if (!in.is_open()) {
        cerr << "Error opening profiling file \"" << in_file << "\"" << endl;
        return -1;
    }
    ofstream out(out_file);
    if (!out.is_open()) {
        cerr << "Error opening output file """ << out_file << "\"" << endl;
        return -1;
    }


vector<string> Set;    
string tLine;

    // ifstream is an input file
    // skip header
    while (in.peek() != EOF ) {
			Set.clear();
            getline(in, tLine);

  //      cout << tLine << endl;
  // tokanaize a stat file line
		    tokenize( Set, tLine, ", " );

		    if ( Set[0].compare(sHeaders[0]) ) 
		    {

		 	    continue;
		    }

            break;
		
     }

#if __VISUAL_TEST__
int n_out = __NUM_OF_LINES_VISUAL__;
	  cout << "First 32 output lines\n\n";
	  cout <<"Kernel Thread  EHP GPU  ";
	  for(int p = 0; p < pim_gpu_count; p++)
	  {
	     cout << " PIM" << p << " GPU0";
	 }
	  cout << "\n";

#endif

	// start getting real statistics
int	i = 0;
    while (in.peek() != EOF ) {
		Set.clear();
        getline(in, tLine);
  //      cout << tLine << endl;
	    tokenize( Set, tLine, ", " );
		string tmp0("Method");
        string kern_nm = getStatLine(sHeaders, Set, tmp0);
		int split_pos = (int)kern_nm.rfind("__");
		kern_nm = kern_nm.substr(0,split_pos);

		string tmp1("Time");
		string tmp2("VALUInsts");
		string tmp3("SALUInsts");
		string tmp4("VFetchInsts");
		string tmp5("LDSInsts");
		string tmp6("VWriteInsts");
		string tmp7("FetchSize");
		string tmp8("WriteSize");
		string tmp9("MemUnitBusy");

		memset(&line_stat, 0, sizeof(line_stat));

		line_stat.counter = 1;
		line_stat.global_sz = getGlobalSzVal(Set);
		line_stat.time = getStatDouble(sHeaders, Set, tmp1);
        line_stat.v_intrs = getStatDouble(sHeaders, Set, tmp2);
        line_stat.s_instr = getStatDouble(sHeaders, Set, tmp3);
        line_stat.f_instr = getStatDouble(sHeaders, Set, tmp4);
        line_stat.l_instr = getStatDouble(sHeaders, Set, tmp5);
		line_stat.w_instr = getStatDouble(sHeaders, Set, tmp6);
        line_stat.f_size = getStatDouble(sHeaders, Set, tmp7);
        line_stat.w_size = getStatDouble(sHeaders, Set, tmp8);
		line_stat.mu_busy = getStatDouble(sHeaders, Set, tmp9);

// calculate scaled performance for every GPU in PIM
        getScaledPerformance(gpu_perf_scaling, &line_stat, EHPgpuThroughput,
		                                               pim_config_->ehp.gpus->num_instr_perclck,
													   EHPMembandwidth);

    for (int c = 0, p = 1; c < (int)pim_config_->num_pim_cores; c++)  {
        // one context per PIM
        pim_configuration_t *pim;
		
        pim = pim_config_->pims + c;



        for (int g = 0; g < (int)pim->num_gpu_cores; g++)
	    {
        // scale performance according with model gpu parameters
            getScaledPerformance((gpu_perf_scaling + p), &line_stat, pim, g, ehp_conf_);
			p++;
        }
    }      


// write out a line in CSV format.
    char out_line[1024];
	string tmp10("ThreadID");
        sprintf(out_line, "%s, %s", kern_nm.c_str(),getStatLine(sHeaders, Set, tmp10).c_str());
		for(int p = 0; p < pim_gpu_count + 1; p++)
		{
			sprintf(out_line,"%s, %12.9f", out_line, gpu_perf_scaling[p].time);
		}

		strcat(out_line, "\n");
		out.write(out_line, strlen(out_line));
#if __VISUAL_TEST__
        if ( i < n_out )
	    {
		  cout << out_line;
		}
#endif
		i++;

    }

	if ( i == 0 )
	{
        fprintf(stderr,
                "Error: %s (%d): nothing has been processed\n",
                __func__, __LINE__);
	}

    in.close();
	out.close();

    if ( gpu_perf_scaling )
	{
		free(gpu_perf_scaling);
	}
	return(1);
}

