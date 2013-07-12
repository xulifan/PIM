Every XML file must, at its highest level, be a node:
<node>
</node>

Within a node, it is required to have a single EHP and zero or more PIMs
(meaning that you may have zero, or you can have more than one):
<node>
                <ehp>
                </ehp>
                <pim>
                </pim>
</node>

Each EHP can must contain at least one CPU, and can contain zero or more GPUs.
Each PIM can contain any number of CPUs, GPUs, and DRAMs. (we currently don't
require any of these - though it's not much of a processor in memory if you
have no processors and no memory. It's left generic for now).
<node>
                <ehp>
                                <cpu_core>
                                </cpu_core>
                                <gpu_core>
                                </gpu_core>
                </ehp>
                <pim>
                                <cpu_core>
                                </cpu_core>
                                <gpu_core>
                                </gpu_core>
                                <dram>
                                </dram>
				<job_start_time_ns> </job_start_time_ns>
				<job_end_time_ns> </job_end_time_ns>
				
                </pim>
</node>

Each field specified for CPU,GPU, DRAM are required for a valid configuration.
Some fields may not be supported in current models, however we expect future support 
and hence even the fields that are not used in our current version of the perforamnce
models are required. Each PIM also requires some additional properties that
are noted below. 

An example configuration is in PIM.xml for reference.


******************************************************************************
			Per PIM Property:
******************************************************************************

<job_start_time_ns>
Description: The time in nano seconds it takes to launch a job to any PIM device
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<job_end_time_ns>
Description: The time in nano seconds it takes for the host processor to be notified that a PIM job has completed 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

******************************************************************************
			Fields for CPU Core : 
******************************************************************************

<isa> 
Description: Instruction set architecture of the core
Acceptable values: x86-64 , arm
Notes: Currently we dont support modeling based on this field. 

<ordering>: 
Description: Specifies if its an inorder or out-of-order execution core.
Acceptable values; inorder, ooo
Notes: Currently we dont support modeling based on this field. 

<freq>
Description: Nominal frequency of the core, measured in megahertz. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. The value specified is assumed
for an entire emulation and assumes that we assumes that clockspeed frequency  __does
not__ change at runtime. 

<width>
Description:  The issue/decode/commit width of the core. 
Acceptable value: Positive Integer values  > 0
Notes: Currently we dont support modeling based on this field. 


<num_hwthreads>
Description:  The number of simultaneous hardware threads on the core. 
Acceptable value: Positive Integer values  > 0
Notes: Currently we dont support modeling based on this field. 

******************************************************************************
			Fields for GPU Core : 
******************************************************************************

<isa>
Description: The ISA of the GPU core. 
Acceptable values: si for Southern Islands. 
Notes: This field is supported in our models. 

<freq>
Description: The frequency of the GPU core specified in megahertz. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. The value specified is assumed
for an entire emulation and assumes that we assumes that clockspeed frequency  __does
not__ change at runtime. 

<cus>
Description: The number of CUs on this particular GPU chip. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<width>
Description: The number of execution units in a CU, for example a CU with four
16-wide SIMDS will have a width of 64.
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<num_instr_perclck>
Description: The maximum number of instructions that can be completed by one lane of a SIMD unit. Normally this is one, but we can model higher rates.
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 


******************************************************************************
			Fields for DRAM : 
******************************************************************************

<size>
Description: Size of this DRAM stack in MB. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<freq>
Description: Frequency of the memory controller channel to DRAM in MHz. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<num_channels>
Description: Number of memory channels that are available in this stack. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<channel_width>
Description: Number of wires available for each channel. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<bits_percycle>
Description: Number of bits transferred across a wire in each clock cycle (e.g. DDR=2). 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<latency>
Description: Average latency in ns required for a device on the PIM stack to access memory in this DRAM. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<off_stack_latency>
Description: __Percentage increase__ in time required for anyone else to access this DRAM. In other words, the EHP or other PIMs will
take latency*(1+(off_stack_latency/100)) nanoseconds. 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<off_stack_bw>
Description: __Percentage__ of total bandwidth available for EHP to access the DRAM on this PIM. 
Total bandwidth itself is calculated in MB/s as ( freq * 1000000. * num_channels *channel_width *bits_percycle *(utilization/100.) )/ 8 
Acceptable value: Positive Integer values > 0
Notes: This field is supported in our models. 

<bw_between_pims>
Description: __Percentage__ of total bandwidth available for remote PIM accesses. 
Total bandwidth itself is calculated in MB/s as  ( freq * 1000000. * num_channels *channel_width *bits_percycle *(utilization/100.) )/ 8
Acceptable value: Positive Integer values > 0
Notes: Currently we dont support modeling based on this field. 

<utilization>
Description: This is "estimated" utilization from the peak bandwidth". 
Accounts for some amount of protocol overhead, bus turnaround time, RAS-to-CAS delays, memory controller queuing, etc. that will prevent you from 
using 100% of the bandwidth available on the physical wires.
Aceeptable value: Positive Integer values > 0
Notes: Currently we dont support modeling based on this field. 


