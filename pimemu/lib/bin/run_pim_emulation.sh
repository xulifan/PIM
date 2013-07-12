#! /bin/bash
#
# Copyright (c) 2013 Advanced Micro Devices, Inc.
#
# RESTRICTED RIGHTS NOTICE (DEC 2007)
# (a)     This computer software is submitted with restricted rights under
#     Government Contract No. DE-AC52-8MA27344 and subcontract B600716. It
#     may not be used, reproduced, or disclosed by the Government except as
#     provided in paragraph (b) of this notice or as otherwise expressly
#     stated in the contract.
#
# (b)     This computer software may be --
#     (1) Used or copied for use with the computer(s) for which it was
#         acquired, including use at any Government installation to which
#         the computer(s) may be transferred;
#     (2) Used or copied for use with a backup computer if any computer for
#         which it was acquired is inoperative;
#     (3) Reproduced for safekeeping (archives) or backup purposes;
#     (4) Modified, adapted, or combined with other computer software,
#         provided that the modified, adapted, or combined portions of the
#         derivative software incorporating any of the delivered, restricted
#         computer software shall be subject to the same restricted rights;
#     (5) Disclosed to and reproduced for use by support service contractors
#         or their subcontractors in accordance with paragraphs (b)(1)
#         through (4) of this notice; and
#     (6) Used or copied for use with a replacement computer.
#
#( c)     Notwithstanding the foregoing, if this computer software is copyrighted
#     computer software, it is licensed to the Government with the minimum
#     rights set forth in paragraph (b) of this notice.
#
# (d)     Any other rights or limitations regarding the use, duplication, or
#     disclosure of this computer software are to be expressly stated in, or
#     incorporated in, the contract.
#
#( e)     This notice shall be marked on any reproduction of this computer
#     software, in whole or in part.

# Script to run the PIM emulator and CPU + GPU analytical model.

script_name=$(basename $0)
# default values for optional args
flags=":x:c:e:f:l:ho:p:r::s:t:-:" 
LIBPAPI_DIR=/usr/local/lib/
LIBROXML_DIR=/usr/lib64/
OUTPATH=$PWD
TMPPATH=$PWD
SPROFILE_PATH=/opt/AMD/CodeXL/bin/x86_64 
PIMPATH=/usr/local
CONFIG=$PWD/PIM.xml
RUN_CMD=0 # defaults to 0 and runs all commands, this is set by user with -x option 


# Var declarations
sprofile_file=pimemu_gpucounterdata
counter_gpu=${sprofile_file}.csv
counter_cpu=pimemu_cpucounterdata.csv
model_cpu=model_cpu.csv
model_gpu=model_gpu.csv
timeline_file=pimemu_timelinedata.csv
result_name=emulation_result
result_ascii=${result_name}.csv
result_pdf=${result_name}.pdf
freq=2500
latency=52.5

#Flags for required parameters, these args are required so they must be passed, also we want to check they are passed only once, no duplicates
FOUND_EXE=0
FOUND_CONFIG=0

#Flag for optionaal parameters, defaults are used unless user passes them, in that case they should be passed only once, i.e. no duplicates
FOUND_PAPIPATH=0
FOUND_ROXMLPATH=0
FOUND_OUTPATH=0
FOUND_TMPPATH=0;
FOUND_SPROFILEPATH=0;
FOUND_PIMPATH=0;
FOUND_RUNCMD=0;
FOUND_FREQ=0;
FOUND_LATENCY=0;

#**************************************************************************************************************
#      FUNCTIONS
#**************************************************************************************************************

function print_help {
	echo " "
	if [ "$1" ]; then
		echo "Error : $1 "
    		echo " "
    	fi

	echo "Usage: $script_name [Optional flags] --run <command to run with its arguments>"
	echo "Specify Optional flags before Required flags "
	echo " "
	echo "Required:"
	echo "--run : The command/program to run in PIM emulation"
	echo " "
	echo "Optional Flags:"
	echo "-h : Print this help message "
	echo "-c : Path to XML configuration file for PIM, defaults to PIM.xml in current working directory: $CONFIG"
	echo "-e : Path to ROOT of PIM emulator installation directory, defaults to: $PIMPATH"   
	echo "-f : CPU core freq in MHz of the machine where profile was collected, defaults to: $freq"   
	echo "-l : memory access latency in nanoseconds of the machine where profile was collected, defaults to: $latency"   
	echo "-o : Path to store result of emulation run, defaults to current working directory: $OUTPATH"   
        echo "     following result files are generated: $result_ascii, $result_pdf"
	echo "-p : Path to Papi library, defaults to: $LIBPAPI_DIR "
	echo "-r : Path to ROXML library, defaults to: $LIBROXML_DIR "
	echo "-s : Path to Sprofile binary, part of AMD CodeXL tool, defaults to: $SPROFILE_PATH"
	echo "-t : Path to store temporary files used for tracing and modeling, defaults to current working directory: $TMPPATH"
        echo "     following temporary files are generated on full tool run: $counter_cpu, $counter_gpu, $model_cpu, $model_gpu, $timeline_file"
	echo "-x : Without -x option, by default the tool runs all commands in order : Profile, Modeling, and Timeline Ordering." 
	echo "     Use -x with arguments to run only one of them :"
	echo "     1 - run profiler, this runs the application and generates performance profiles: $counter_cpu, $counter_gpu, $timeline_file"
	echo "     2 - run models, reads peformance profiles and generates performance predictions: $model_cpu, $model_gpu "
	echo "     3 - run timeline ordering, reads performance predictions and generates timeline ordering results: $result_ascii, $result_pdf"
	echo "     Commands 1 and 2 assume read/write from TMPDIR, change it using -t option"
	echo "     Command 3 reads predictions from TMPDIR change with -t option, and writes results to OUTDIR change with -o option"
	echo " "
   	echo "Example: $script_name  -c config.xml -t /tmp -o /result --run my_prog my_args "
	echo " "

	exit 1
}

function print_error {

	echo " "
	echo "Error : $1"
	echo " "
}

function error_exit { 
	echo " "
	echo " Exiting error was : $1"
	echo " " 
	exit 1
}

function check_exitstatus {
	if [ $1 != 0 ] ; then
		error_exit "$2"
	fi
}

function check_duplicate {

	if [ $1 == 1 ]; then
		msg="Option -$2 was specified earlier"
		print_help "$msg"
	fi 

}

function check_valid_arg {
	#dash="-"
	match=`expr match $1 '^-' `

	if [ $match != 0 ] ;   then
		msg="Illegal argument $1 specified for Option -$2 "
		print_help "$msg"
	fi

}

function GetFullPath {

	local __fullpath=$3
	local tmppath=`readlink -e $1`
	local blank=''
	if [ "$tmppath" == "$blank" ]; then
		msg="For option -$2 cannot find path of arg $1 "
		error_exit "$msg"
	fi
	eval $__fullpath="'$tmppath'"
	
}
#**************************************************************************************************************
#      PARSE FLAGS 
#**************************************************************************************************************

while getopts $flags option; 
do
    case "${option}"
	in
	   
	c)  check_duplicate $FOUND_CONFIG $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option CONFIG
	    FOUND_CONFIG=1
            ;;
	e)  check_duplicate $FOUND_PIMINSTALLPATH $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option PIMPATH
	    FOUND_PIMPATH=1
	    ;;
	f)  check_duplicate $FOUND_FREQ $option
	    check_valid_arg $OPTARG $option
	    if [ $OPTARG  < 1  ] ; then
		msg="Incorrect argument for -$option  option, see help for option values " 
		print_help "$msg"
	    fi
	    freq=$OPTARG 
	    FOUND_FREQ=1
	    ;;
	h)  print_help
	    ;;
	l)  check_duplicate $FOUND_LATENCY $option
	    check_valid_arg $OPTARG $option
	    if [ $OPTARG  < 0.0  ] ; then
		msg="Incorrect argument for -$option  option, see help for option values " 
		print_help "$msg"
	    fi
	    latency=$OPTARG 
	    FOUND_LATENCY=1
	    ;;
	o)  check_duplicate $FOUND_OUTPATH $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option OUTPATH
	    FOUND_OUTPATH=1
	    ;;
	p)  check_duplicate $FOUND_PAPIPATH $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option LIBPAPI_DIR 
	    FOUND_PAPIPATH=1
	    ;;
	r)  check_duplicate $FOUND_ROXMLPATH $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option LIBROXML_DIR 
	    FOUND_ROXMLPATH=1
	    ;;
	s)  check_duplicate $FOUND_SPROFILEPATH $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option SPROFILE_PATH 
	    FOUND_SPROFILEPATH=1
	    ;;
	t)  check_duplicate $FOUND_TMPPATH $option
	    check_valid_arg $OPTARG $option
	    GetFullPath $OPTARG $option TMPPATH 
	    FOUND_TMPPATH=1
	    ;;
	x)  check_duplicate $FOUND_RUNCMD $option
	    check_valid_arg $OPTARG $option
	    if [[ $OPTARG  < 1  ||  $OPTARG > 3 ]] ; then
		msg="Incorrect argument for -$option  option, see help for option values " 
		print_help "$msg"
	    fi
	    RUN_CMD=$OPTARG
	    FOUND_RUNCMD=1
            ;;
	:)  msg="Option -$OPTARG requires an argument"
	    print_help "$msg"
	    ;;
	\?) msg="Invalid option: -$OPTARG" 
	    print_help "$msg"
	    ;;
	-)  OPTIND=$((OPTIND-1))
	    break
	    ;;
    esac
done

shift $((OPTIND - 1))
if [ $# -lt 2 ]; then
	msg="Missing command to run"
	print_help "$msg"
else
	if [ $1 == "--run" ]; then
		shift 1
		EXE=$@
		FOUND_EXE=1
	else
		msg="Specify command required with --run flag"
		print_help "$msg"
	fi

fi


if [ $FOUND_EXE == 0 ]; then
	print_help
	exit 1
fi


#Check if dorectories exists , go  through all checks so we can print all errors at once and at the end print one help message
if [ ! -r $LIBPAPI_DIR ]; then 
	msg="Cannot read from DIR $LIBPAPI_DIR "
	print_error "$msg"
	PATH_ERROR=1
fi
if [ ! -r $LIBROXML_DIR ]; then 
	msg="Cannot read from DIR $LIBOXML_DIR "
	print_error "$msg"
	PATH_ERROR=1
fi
if [ ! -r $PIMPATH ]; then
	msg="Cannot read from DIR $PIMPATH "
	print_error "$msg"
	PATH_ERROR=1
fi
if [ ! -r $CONFIG ]; then
	msg="Cannot read from file $CONFIG "
	print_error "$msg"
	PATH_ERROR=1
fi
if [ ! -w $TMPPATH ]; then
	msg="Cannot write to DIR $TMPPATH "
	print_error "$msg"
	PATH_ERROR=1
fi
if [ ! -w $OUTPATH ]; then
	msg="Cannot write to DIR $OUTPATH "
	print_error "$msg"
	PATH_ERROR=1
fi

if [ "$PATH_ERROR" == 1 ]; then
	print_help
fi

#**************************************************************************************************************
#      RUN THE EMULATION TOOLS  
#**************************************************************************************************************

function run_profiler {
	export PIMEMUENV=$CONFIG:1:$TMPPATH/

	#********* Collect performance countersa in emulation mode
	$SPROFILE_PATH/sprofile --perfcounter  -o $TMPPATH/$sprofile_file $EXE 
	EXIT_STATUS=`echo $?`
	msg="Program $EXE ran with errors "
	check_exitstatus "$EXIT_STATUS" "$msg"

	if [ ! -f ${TMPPATH}/${counter_cpu} ] ; then
		msg="$Counter file $counter_cpu was not generated"
		error_exit "$msg"	
	fi
	echo " "
	echo " *************************************************************************************"
	echo " Finished profiling and generated files: "
	echo " $TMPPATH/$counter_cpu "
	if [ -f ${TMPPATH}/${counter_gpu} ] ; then
		echo " $TMPPATH/$counter_gpu "
	fi
	echo " *************************************************************************************"
	echo " "
}

#*********** Call CPU Model prediction
function run_models {
	export PIMEMUENV=$CONFIG:0:$TMPPATH
	
	local _Model_GPU=$1

	if [ ! -r $TMPPATH/$counter_cpu ] ; then 
		msg="cpu perf counter file $TMPPATH/$counter_cpu not found "
		error_exit "$msg"
	fi
	$PIMPATH/bin/pim_amodel.exe -cpu -profiled_freq $freq  -profiled_latency $latency -prf $TMPPATH/$counter_cpu -outf ${TMPPATH}/$model_cpu
	EXIT_STATUS=`echo $?`
	msg="Cpu modeling ran incorrectly "
	check_exitstatus "$EXIT_STATUS" "$msg"

	#*************** Call GPU models if GPU Counters have been collected, call ordering model for final result
	if [ $_Model_GPU == 1 ] ; then

		if [ ! -r $TMPPATH/$counter_gpu ] ; then 
			msg="gpu perf counter file $TMPPATH/$counter_gpu was not found "
			error_exit "$msg"
		fi
		$PIMPATH/bin/pim_amodel.exe -gpu -prf $TMPPATH/$counter_gpu -outf ${TMPPATH}/$model_gpu
		EXIT_STATUS=`echo $?`
		msg="Gpu modeling ran incorrectly "
		check_exitstatus "$EXIT_STATUS" "$msg"
	fi

	echo " "
	echo " *************************************************************************************"
	echo " Finished modeling and generated files: "
	echo " $TMPPATH/$model_cpu "
	if [ $_Model_GPU == 1 ] ; then
		echo " $TMPPATH/$model_gpu "
	fi
	echo " *************************************************************************************"
	echo " "
}

function run_ordering {

	export PIMEMUENV=$CONFIG:0:$TMPPATH
	
	local __Model_GPU=$1
	local __quit=0

	# First check if files exist and print errors and set quit flag 
	if [ ! -r $TMPPATH/$model_cpu ] ; then
		#__quit=1
		msg="model file $TMPPATH/$model_cpu not found"
		print_error "$msg"
	fi
	if [ ! -r $TMPPATH/$timeline_file ] ; then
		__quit=1
		msg="timeline file $TMPPATH/$timeline_file not found"
		print_error "$msg"
	fi
	
	if [ $__Model_GPU == 1 ] ; then
		
		if [ ! -r $TMPPATH/$model_gpu  ] ; then 
			#__quit=1
			msg="model file $TMPPATH/$model_gpu not found "
			print_error "$msg"
		fi
		if [ $__quit == 1 ] ; then
			msg="Required file(s) were missing ..."
			error_exit "$msg"	
		fi
        rm -f ${OUTPATH}/$result_ascii
		$PIMPATH/bin/pim_amodel.exe -total -cpu_perf ${TMPPATH}/$model_cpu -gpu_perf ${TMPPATH}/$model_gpu -order_trace ${TMPPATH}/$timeline_file  -outf ${OUTPATH}/$result_ascii
		EXIT_STATUS=`echo $?`
		msg="Ordering model ran incorrectly "
		check_exitstatus "$EXIT_STATUS" "$msg"
	else
		if [ $__quit  == 1 ]; then
			msg="Required file(s) were missing ..."
			error_exit "$msg";	
		fi
        rm -f ${OUTPATH}/$result_ascii
		$PIMPATH/bin/pim_amodel.exe -total -cpu_perf ${TMPPATH}/$model_cpu -order_trace ${TMPPATH}/$timeline_file  -outf ${OUTPATH}/$result_ascii
		EXIT_STATUS=`echo $?`
		msg="Ordering model ran incorrectly "
		check_exitstatus "$EXIT_STATUS" "$msg"
	fi


	if [ -r ${OUTPATH}/$result_ascii ] ; then
        rm -f ${OUTPATH}/${result_pdf}
		python -W ignore::DeprecationWarning $PIMPATH/bin/ordering_grapher.py ${OUTPATH}/${result_ascii} ${OUTPATH}/${result_pdf}
		EXIT_STATUS=`echo $?`
		msg="Did not generate PDF result file $OUTPATH/$result_pdf"
		check_exitstatus "$EXIT_STATUS" "$msg"
	else
		msg="The ordering model ran incorreectly and did not generate result files $OUTPATH/$result_ascii and $OUTPATH/$result_pdf"
		error_exit "$msg"
	fi
	echo " "
	echo " *************************************************************************************"
	echo " Finished timeline ordering and generated files: "
	echo " $OUTPATH/$result_ascii "
	echo " $OUTPATH/$result_pdf "
	echo " *************************************************************************************"
	echo " "
}

function check_gpucounters {
	local __modelgpu=$1
	local flag=1
	if [ ! -f ${TMPPATH}/${counter_gpu} ] ; then
		flag=0
	fi
	eval $__modelgpu="'$flag'"
}

MODEL_GPU=1
LD_LIBRARY_PATH=$LIBPAPI_DIR:$LIBROXML_DIR:$PWD:$LD_LIBRARY_PATH:$PIMPATH/lib:

case "$RUN_CMD" in 

	0)	run_profiler
		check_gpucounters MODEL_GPU	
		run_models $MODEL_GPU
		run_ordering $MODEL_GPU
		;;	
	1)	run_profiler
		;;
	2)	check_gpucounters MODEL_GPU	
		run_models $MODEL_GPU
		;;
	3)	check_gpucounters MODEL_GPU	
		run_ordering $MODEL_GPU
		;;
	*)      msg="Illegal option $RUN_CMD specified please try again"
	 	error_exit "$msg"
		;;
esac
