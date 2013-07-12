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

#include "validate_config.h"

int validate_cpu(char *devID, cpu_configuration_t *cpu) {

  int error_flag=0;

  if(cpu->freq < 1) {
    fprintf(stderr,"Error: %s CPU %d freq is invalid\n",devID,cpu->id);
    error_flag=1;
  }
  if(cpu->width < 1) {
    fprintf(stderr,"Error: %s CPU %d width is invalid \n",devID, cpu->id);
    error_flag=1;
  }
  if(cpu->num_hwthreads < 1) {
    fprintf(stderr,"Error: %s CPU %d num_hwthreads is invalid\n",devID, cpu->id);
    error_flag=1;
  }
  if( cpu->isa!=ISA_X86_64 && cpu->isa!=ISA_ARM) {
    fprintf(stderr,"Error: %s CPU %d isa is invalid\n",devID, cpu->id);
    error_flag=1;
  }
  if(cpu->ordering !=ORDERING_INORDER && cpu->ordering!=ORDERING_OOO) {
    fprintf(stderr,"Error: %s CPU %d ordering is invalid\n",devID, cpu->id);
    error_flag=1;
  }

  if(error_flag)
    return PIM_INVALID_CPU_CONFIGURATION_PARAMETER;
  else 
    return PIM_SUCCESS;
}

int validate_gpu(char *devID, gpu_configuration_t *gpu) {

  int error_flag=0;
  
  if(gpu->freq < 1) {
    fprintf(stderr,"Error: %s GPU %d freq is invalid\n",devID,gpu->id);
    error_flag=1;
  }
  if(gpu->cus < 1) {
    fprintf(stderr,"Error: %s GPU %d number of CUs is invalid\n",devID,gpu->id);
    error_flag=1;
  }
  if(gpu->width < 1) {
    fprintf(stderr,"Error: %s GPU %d width is invalid\n",devID,gpu->id);
    error_flag=1;
  }
  if(gpu->num_instr_perclck < 1) {
    fprintf(stderr,"Error: %s GPU %d num_instr_perclck is invalid\n",devID,gpu->id);
    error_flag=1;
  }
  if(gpu->isa !=ISA_SI) {
    fprintf(stderr,"Error: %s GPU %d isa is invalid\n",devID,gpu->id);
    error_flag=1;
  }

  if(error_flag)
    return PIM_INVALID_GPU_CONFIGURATION_PARAMETER;
  else 
    return PIM_SUCCESS;

}

int validate_dram(char *devID, dram_configuration_t *dram) {

  int error_flag=0;

  if(dram->size_in_mb < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d size is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->off_stack_bw < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d off_stack_bw is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->between_pim_bw < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d bewteen_pim_bw is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->freq < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d freq is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->num_channels < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d num_channels is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->bits_percycle < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d bits_percycle is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->latency < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d latency is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->off_stack_latency < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d off_stack_latency is invalid\n",devID,dram->id);
    error_flag=1;
  }
  if(dram->utilization < 1) {
    fprintf(stderr,"Error: %s DRAM stack %d utilization is invalid\n",devID,dram->id);
    error_flag=1;
  }

  
  if(error_flag)
    return PIM_INVALID_DRAM_CONFIGURATION_PARAMETER;
  else 
    return PIM_SUCCESS;

}

int validate_ehp(ehp_configuration_t ehp) {
  
  int cpuid=0, gpuid=0;
  int ret=0;
  int error_flag=0; 
  char processorID[24];
    
  snprintf(processorID,24,"EHP");

  if(ehp.num_cpu_cores < 1 ) { //The EHP must have at least one CPU
    fprintf(stderr,"Error: EHP must have at least one CPU \n");
    error_flag=1;
  }
    
  for(cpuid=0; cpuid < ehp.num_cpu_cores; cpuid++) {
    ret=validate_cpu(processorID,&ehp.cpus[cpuid]);
    if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"Error: EHP CPU %d had error in configuration \n",cpuid);
      error_flag=1;
    }
  }

  for(gpuid=0; gpuid < ehp.num_gpu_cores; gpuid++) {
    ret=validate_gpu(processorID,&ehp.gpus[gpuid]);
    if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"Error: EHP GPU %d had error in configuration \n",gpuid);
      error_flag=1;
    }

  }


  if(error_flag)
    return PIM_INVALID_EHP_CONFIGURATION_PARAMETER;
  else 
    return PIM_SUCCESS;

}

int validate_pim(pim_configuration_t *pim){

  int cpuid=0, gpuid=0, dramid=0;
  int ret=0;
  int error_flag=0; 
  char processorID[24];

  snprintf(processorID,24,"PIM %d",pim->id);

  if(pim->ns_to_launch_to_pim < 1) {
    fprintf(stderr,"Error: Time to launch job to PIM %d is incorrect, value should be > 0\n",pim->id);
    error_flag=1;
  }
  if(pim->ns_to_complete_from_pim < 1) {
    fprintf(stderr,"Error: Time to complete job from PIM %d is incorrect, value should be > 0\n",pim->id);
    error_flag=1;
  }

  // Each PIM should have a minimum of one DRAM stack and can have zero or more devices (cpu, gpu)
  if(pim->num_dram_stacks < 1) {
    fprintf(stderr,"Error: PIM %d has no DRAM stacks defined \n",pim->id);
    error_flag=1;
  }

  for(cpuid=0; cpuid < pim->num_cpu_cores; cpuid++) {
    ret=validate_cpu(processorID,&pim->cpus[cpuid]);
    if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"Error: PIM %d had error in CPU %d configuration \n",pim->id, cpuid);
      error_flag=1;
    }
  }

  for(gpuid=0; gpuid < pim->num_gpu_cores; gpuid++) {
    ret=validate_gpu(processorID, &pim->gpus[gpuid]);
    if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"Error: PIM %d had error in GPU %d configuration \n",pim->id, gpuid);
      error_flag=1;
    }
  }

  for(dramid=0; dramid < pim->num_dram_stacks; dramid++) {
    ret=validate_dram(processorID,&pim->dram[dramid]);
    if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"Error: PIM %d had error in DRAM stack %d configuration \n",pim->id, dramid);
      error_flag=1;
    }
  }

  if(error_flag)
    return PIM_INVALID_PIM_CONFIGURATION_PARAMETER;
  else 
    return PIM_SUCCESS;
  
  

}

int validate_configuration(pim_emu_configuration_t *configuration) {


  int pimid=0;
  int ret=0;
  int error_flag=0;

  if(configuration->num_pim_cores < 1) {
    fprintf(stderr,"Error: did not find any PIM, at least one PIM needs to be defined \n");
    error_flag=1;
  }

  ret=validate_ehp(configuration->ehp);

  if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"EHP configuration had errors\n");
      error_flag=1;
  }


  for(pimid=0; pimid < configuration->num_pim_cores; pimid++) {
    pim_configuration_t *pim;
    pim=configuration->pims + pimid;
    ret=validate_pim(pim);
    if(ret!=PIM_SUCCESS) {
      fprintf(stderr,"PIM %d configuration had errors\n",pimid);
      error_flag=1;
    }
  }


  if(error_flag)
    return PIM_INVALID_CONFIGURATION;
  else 
    return PIM_SUCCESS;

}
