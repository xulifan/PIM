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

#include "cpuAModel.hpp"
#include "common_defs.hpp"

typedef struct {
  double Total_Time;
  double Commit_Time;
  double Stall_Time;
} thread_perf;
        
typedef struct {
  long long ThreadId;
  double Time;
  long long Commit_Cyc;
  long long Instructions;
  long long Clock_Cyc;
  long long L2_Cache_Miss;
  long long L2_Writebacks;
} thread_stat;

static const char * sHeaders[] = {
  "Exec_Order" ,
  "ThreadId" ,
  "elapsed_us" ,
  "elapsed_uc" ,
  "COMMIT_CYC" ,
  "INSTRUCTIONS" , 
  "CLOCK_CYC" , 
  "L2_CACHE_MISS" ,
  ""
};

static double getTime(vector<string>& Set_, string & id, double resolution)
{
  double time;
  int indx = getIdIndex(sHeaders, id);
  string t = Set_[indx];
  time = stol(t) * resolution; //return time in seconds
  return(time);
}
  
static long long getThreadId(vector<string>& Set_, string & id )
{
  long long ThreadId;
  int indx = getIdIndex(sHeaders, id);
  string tid = Set_[indx];
  int pos = (int)tid.find_first_of("xX");
  

  if(pos !=  tid.npos) { //we assume that values prefixed by 0x or 0X are hex other wise treat them as decimal values 
    tid = tid.substr(pos+1, tid.npos);
    std::stringstream hh;
    hh << std::hex << tid;
    hh >> ThreadId;
  }
  else {
    ThreadId = stoll(tid);
  }
  //   cout << " DEBUG " << " ThreadId Index = " << indx  << " pos = " << pos << " tid substr = " << tid << " hex tid = " << tid << " ThreadId = " << ThreadId << endl;
  return (ThreadId);
}

static void getBaseLinePerformance(thread_perf * perf_estimate, const thread_stat *thread, cpu_configuration_t *cpu_conf,int cpu_id, ehp_conf_t *input, uint32_t profiled_freq, double profiled_latency, double mem_bandwidth_avail, double ehp_latency)
{

  // Get perf on profiled hardware first 
  double Total_Time = thread->Time;
  double Commit_Time = ((double)thread->Commit_Cyc)/(profiled_freq*1.0e06);
  double Stall_Time = Total_Time - Commit_Time;

  // Get Perf estimate on Baseline EHP 
  double ehp_freq = 0.0;
  ehp_freq = getCPUFrequency(cpu_conf, &input->cpus);
  Commit_Time = Commit_Time * ((double)profiled_freq/(double)ehp_freq );
  Stall_Time = Stall_Time * ((double)ehp_latency/(double)profiled_latency);
  Total_Time = Commit_Time + Stall_Time;
 
  long long mem_requests = thread->L2_Cache_Miss;
  int Xfer_per_mem_request = 64;
  long long bytes = mem_requests * Xfer_per_mem_request;
  double mem_bandwidth_requested  = bytes/Total_Time;
 
  if (mem_bandwidth_requested > mem_bandwidth_avail) {
    perf_estimate->Total_Time = bytes/mem_bandwidth_avail; // total time is bandwidth limited
    perf_estimate->Commit_Time = Commit_Time;
    perf_estimate->Stall_Time = perf_estimate->Total_Time - perf_estimate->Commit_Time;
  }
  else {
    perf_estimate->Commit_Time = Commit_Time;
    perf_estimate->Stall_Time = Stall_Time;
    perf_estimate->Total_Time = Total_Time;
  } 
  

}

static void getScaledPerformance(double EHP_Stall_Time, thread_perf *perf_estimate, const thread_stat *thread, const pim_configuration_t * pim, int cpu_id, const ehp_conf_t *input_conf)
{
  double mem_bandwidth_avail = getPimDramBandwidth(pim, input_conf)/pim->num_cpu_cores;
  double pim_latency = (double) getPimDramLatency(pim, input_conf);

  double ehp_latency_to_PIM = getPimDramOffStackLatency(pim, input_conf);
  double latency_speedup = pim_latency/ehp_latency_to_PIM;

  
  double Commit_Time = thread->Commit_Cyc /( pim->cpus[cpu_id].freq*1.0e06);
  double Stall_Time = EHP_Stall_Time * latency_speedup;
 
  double Total_Time = Commit_Time  + Stall_Time;


  long long mem_requests = thread->L2_Cache_Miss;
  int Xfer_per_mem_request = 64;
  long long bytes = mem_requests * Xfer_per_mem_request;
  double mem_bandwidth_requested  = bytes/Total_Time;
 
  if (mem_bandwidth_requested > mem_bandwidth_avail) {
    perf_estimate->Total_Time = bytes/mem_bandwidth_avail; // total time is bandwidth limited
    perf_estimate->Commit_Time = Commit_Time;
    perf_estimate->Stall_Time = perf_estimate->Total_Time - Commit_Time;
  }
  else {
    perf_estimate->Commit_Time = Commit_Time;
    perf_estimate->Stall_Time = Stall_Time;
    perf_estimate->Total_Time = Total_Time;
  } 
  
}
/*********************************************************************************************/

int  cpuAModel (const char * conf_file_,
		const char *in_file_,
		const char* out_file_,
		const pim_emu_configuration_t *pim_config_,
		ehp_conf_t * ehp_conf_,
		uint32_t profiled_freq,
		double profiled_latency
		)
{

  
  const char *in_file = in_file_;
  const char *out_file = out_file_;
  const char *conf_file = conf_file_;
  thread_stat line_stat;
  string key_stat;

  thread_perf *thread_perf_estimate;

  double EHPMembandwidth = getEHPMemBandwidth(pim_config_, ehp_conf_)/pim_config_->ehp.num_cpu_cores;
  double EHPLatency = getEHPLatency(pim_config_, ehp_conf_);  //FIXME this returns average EHP memory access Latency across to PIMs, but possibly needs to be done per PIM, so that means we need to calculate EHPPerformance per PIM, if each PIM has different DRAM specs 

  cout << "\n\nExecuting PIM CPU performance analytical model...\n";
  cout << "---------------------------------------------------\n\n";
  cout << "model config file: " << conf_file <<".\n";
  cout << "model profiling file: " << in_file <<".\n";
  cout << "model output file: " << out_file <<".\n\n";


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

  //Count EHP and PIM CPUS
  int ehp_cpu_count = (int) pim_config_->ehp.num_cpu_cores;
 
  int pim_cpu_count=0;
   

  for(int g=0; g < (int) pim_config_->num_pim_cores; g++) {
    pim_configuration_t *pim;

    pim = pim_config_->pims  + g;
    pim_cpu_count+= pim->num_cpu_cores;
  }

  char out_line[2048];
  thread_perf_estimate = (thread_perf *) calloc(pim_cpu_count + ehp_cpu_count, sizeof(thread_perf));
   
  sprintf(out_line,"Exec_Order, ThreadId, Time(s) \n"); 
  PRINTSCREEN("%s",out_line);
  out.write(out_line, strlen(out_line));
  
  // ifstream is an input file
  // skip header
int printed_line_counter = 0;
  while (in.peek() != EOF ) {
    Set.clear();
    getline(in, tLine);

    // tokanaize a stat file line
    tokenize( Set, tLine, ", " );
     
      
    if ( Set[0].compare(sHeaders[0]) ==0) 
      {
	continue;
      }

    int i=0;
    string t1("Exec_Order");
    string exec_order = getStatLine(sHeaders, Set, t1);

    string tmp0("ThreadId");
    string tmp1("elapsed_us");
    string tmp2("COMMIT_CYC");
    string tmp3("INSTRUCTIONS");
    string tmp4("CLOCK_CYC");
    string tmp5("L2_CACHE_MISS");
      
    memset(&line_stat, 0, sizeof(line_stat));
    line_stat.ThreadId = getThreadId(Set, tmp0);
    line_stat.Time = getTime(Set,tmp1,1.0e-06);
    line_stat.Commit_Cyc = getStatLong(sHeaders, Set,tmp2);
    line_stat.Instructions = getStatLong(sHeaders, Set,tmp3);
    line_stat.Clock_Cyc = getStatLong(sHeaders, Set, tmp4);
    line_stat.L2_Cache_Miss = getStatLong(sHeaders, Set, tmp5);

    
    PRINT_DEBUG("Read input for thread %d \n",line_stat.ThreadId);
    cpu_configuration_t *ehp =pim_config_->ehp.cpus ;

    for(int c=0; c < ehp_cpu_count; c++) {
      getBaseLinePerformance( (thread_perf_estimate + c),&line_stat, ehp, c, ehp_conf_, profiled_freq, profiled_latency, EHPMembandwidth, EHPLatency);
      ehp++;
    }

   
   for(int c=0,p = ehp_cpu_count; c < (int) pim_config_->num_pim_cores; c++) {
	pim_configuration_t *pim;
	pim = pim_config_->pims + c;
	for(int g=0; g < (int) pim->num_cpu_cores; g++)
	{
	  getScaledPerformance(thread_perf_estimate->Stall_Time, (thread_perf_estimate+p), &line_stat, pim, g, ehp_conf_);
	  p++;
	}


   }

   // Print times, one line per cpu
  /* 
   for(int p=0; p < ehp_cpu_count; p++) {
   sprintf(out_line,"%s, %d, EHP, %d, %f\n",exec_order.c_str(), (uint32_t) line_stat.ThreadId, p, thread_perf_estimate[p].Total_Time);
     PRINTSCREEN("%s",out_line);
     out.write(out_line, strlen(out_line));
   }
   

   
   int p = ehp_cpu_count;
   for( int pimid=0; pimid < (int) pim_config_->num_pim_cores; pimid++ ) {
     for( int cpu=0; cpu < (int) pim_config_->pims[pimid].num_cpu_cores; cpu++)
       {
	 sprintf(out_line,"%s, %d, PIM%d, %d, %f\n",exec_order.c_str(), (uint32_t) line_stat.ThreadId, pimid, cpu, thread_perf_estimate[p].Total_Time);	 
	 PRINTSCREEN("%s",out_line);
	 out.write(out_line, strlen(out_line));
	 p++;
       }
   }
  
   */
   // Print times all one one line in order CPU0 to CPUN for EHP, followed be CPU0 to CPUN for PIM0, CPU0 to CPUN for PIM1, ....
   
   sprintf(out_line,"%s, %d",exec_order.c_str(), (uint32_t) line_stat.ThreadId);
   if ( printed_line_counter < 32 )
   {
       PRINTSCREEN("%s",out_line);
   }
   out.write(out_line, strlen(out_line));

   for(int p=0; p < ehp_cpu_count + pim_cpu_count; p++) {
     sprintf(out_line,", %.9f",thread_perf_estimate[p].Total_Time);
     if ( printed_line_counter < 32 )
     {
        PRINTSCREEN("%s",out_line);
	 }
     out.write(out_line, strlen(out_line));

   }

   sprintf(out_line,"\n");
   if ( printed_line_counter < 32 )
   {
      PRINTSCREEN("%s",out_line);
   }
   out.write(out_line, strlen(out_line));
   printed_line_counter++;


  } // end of while 

   in.close();
   out.close();
   if(thread_perf_estimate)
     free(thread_perf_estimate);

   return(1);

}

