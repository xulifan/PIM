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

#include "pimOrderingModel.hpp"

#define DEBUG_TRACE 0

// Helper function to go from different strings to
// unsigned longs. Includes support for hex!
static unsigned long strtoul_helper(string strval)
{
    return strtoul(strval.c_str(), NULL, 0);
}

// Helper function to go from different strings to
// unsgined long longs. Includes support for hex!
static unsigned long long strtoull_helper(string strval)
{
#ifdef WIN32
	return(_strtoui64(strval.c_str(), NULL, 0));
#else
    return strtoull(strval.c_str(), NULL, 0);
#endif
}

void print_malformed_line(vector<string>& Set)
{
    cerr << "However, the file only has " << Set.size() << " columns." << endl;
    cerr << "Malformed line is: ";
    for (unsigned int i = 0; i < Set.size(); i++) {
        cerr << Set[i] << " ";
    }
    cerr << endl;
}

static unsigned long get_sys_tid(vector<string>& Set_)
{
    // System Thread ID is always the second number in our ordering model line
    if (Set_.size() <= 1) {
        cerr << "Attempting to access a ThreadID in column " << 1+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[1]);
}

static unsigned long get_ocl_launch_waitcount(vector<string>& Set_)
{
    if (Set_.size() <= 4) {
        cerr << "Attempting to access an OpenCL Launch Waitcount in column " << 4+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[4]);
}

static unsigned long get_ocl_pim_launch_waitcount(vector<string>& Set_)
{
    if (Set_.size() <= 5) {
        cerr << "Attempting to access an OpenCL PIM Launch Waitcount in column " << 5+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[5]);
}

static unsigned long long get_ocl_launch_events(vector<string>& Set_, int which)
{
    if (Set_.size() <= (unsigned int)(5+which)) {
        cerr << "Attempting to access an OpenCL Launch Event in column " << (5+which)+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoull_helper(Set_[(5+which)]);
}

static unsigned long long get_ocl_pim_launch_events(vector<string>& Set_, int which)
{
    if (Set_.size() <= (unsigned int)(6+which)) {
        cerr << "Attempting to access an OpenCL PIM Launch Event in column " << (6+which)+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoull_helper(Set_[(6+which)]);
}

static unsigned long get_ocl_waitcount(vector<string>& Set_)
{
    if (Set_.size() <= 3) {
        cerr << "Attempting to access an OpenCL Waitcount in column " << 3+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[3]);
}

static unsigned long long get_ocl_wait_events(vector<string>& Set_, int which)
{
    if (Set_.size() <= (unsigned int)(4+which)) {
        cerr << "Attempting to access an OpenCL Wait Events in column " << (4+which)+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoull_helper(Set_[(4+which)]);
}

static unsigned long long get_segment_uid(vector<string>& Set_)
{
    // Segment UID is always the third number in our ordering model line
    if (Set_.size() <= 2) {
        cerr << "Attempting to access a Segment UID in column " << 2+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoull_helper(Set_[2]);
}

static unsigned long get_pthread_id(vector<string>& Set_)
{
    // For pthread segments, the last value is the pthread ID of e.g. children
    if (Set_.size() <= 3) {
        cerr << "Attempting to access a Pthread ID in column " << 3+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[3]);
}

static unsigned long get_pim_id(vector<string>& Set_)
{
    // For  pthread segments, the 4th value is the PIM ID to launch to
    if (Set_.size() <= 3) {
        cerr << "Attempting to access a PIM ID in column " << 3+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[3]);
}

// Get the PThread ID from an Ordering Model line, put it into the correct
// segment location.
static void get_pthread_segment(segment_info& this_seg, vector<string>& Set_)
{
    this_seg.pthread_id = get_pthread_id(Set_);
    return;
}

// Get the System Thread ID from a CPU performance line.
static unsigned long get_cpu_perf_systid(vector<string>& Set_)
{
    if (Set_.size() <= 1) {
        cerr << "Attempting to access a SysTID in the CPU performance file" << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[1]);
}

// Get the Segment's UID from a CPU performance line.
static unsigned long long get_cpu_perf_uid(vector<string>& Set_)
{
    if (Set_.size() <= 0) {
        cerr << "Attempting to access a UID in the CPU performance file" << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoull_helper(Set_[0]);
}

// Get the runtime in seconds (as a double) of an EHP CPU
// from a CPU performance trace line.
static double get_perf_ehp(vector<string>& Set_, int cpu_number)
{
    // Start at 2 because the first is Exec_Order, the second is ThreadId
    unsigned int col_to_access = 2+cpu_number;
    if (Set_.size() <= col_to_access) {
        cerr << "Attempting to access a EHP performance in column " << col_to_access+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return getStatDouble_helper(Set_, col_to_access);
}

// Get the runtime in seconds (as a double) of a PIM CPU
// from a CPu performance trace line.
static double get_perf_pim(vector<string>& Set_, int cpus_to_skip, int pim_num, int cpu_per_pim, int cpu_num)
{
    // Start at 2 because the first is Exec_Order, the second is ThreadId.
    // Then EHP CPUs (please passs in the total number).
    unsigned int col_to_access = (unsigned int)(2+(ehp_cpu_time.size())+(pim_num*cpu_per_pim)+cpu_num);
    if (Set_.size() <= col_to_access) {
        cerr << "Attempting to access a PIM CPU performance in column " << col_to_access+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return getStatDouble_helper(Set_, col_to_access);
}

// Get the System Thread ID from a GPU performance line.
static unsigned long get_gpu_perf_systid(vector<string>& Set_)
{
    if (Set_.size() <= 1) {
        cerr << "Attempting to access a GPU SysTID in column " << 1+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return strtoul_helper(Set_[1]);
}

// Get the OpenCL kernel name from a GPU performance line.
static string get_gpu_perf_kernel(vector<string>& Set_)
{
    if (Set_.size() <= 0) {
        cerr << "Attempting to access a GPU kernel name in column " << 0+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    return Set_[0];
}

// An ordering model line contains informationa bout the type of event
// that bookmarked this segment. This changes that string into an
// internal number used within this tool.
static SegmentType get_SegmentType(vector<string>& Set_)
{
    // Segment type is always the first entry in our ordering model
    if (Set_.size() <= 0) {
        cerr << "Attempting to access a Segment Type in column " << 0+1 << endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    string segstr = Set_[0];

    if (!segstr.compare("PIMEMU_BEGIN"))
        return PIMEMU_BEGIN;
    else if (!segstr.compare("PIMEMU_END"))
        return PIMEMU_END;
    else if (!segstr.compare("PIMEMU_LAST"))
        return PIMEMU_LAST;
    else if (!segstr.compare("PT_START"))
        return PT_START;
    else if (!segstr.compare("PT_EXIT"))
        return PT_EXIT;
    else if (!segstr.compare("PT_LAUNCH"))
        return PT_LAUNCH;
    else if (!segstr.compare("PT_LAUNCH_PIM"))
        return PT_LAUNCH_PIM;
    else if (!segstr.compare("PT_JOIN"))
        return PT_JOIN;
    else if (!segstr.compare("OCL_LAUNCH"))
        return OCL_LAUNCH;
    else if (!segstr.compare("OCL_LAUNCH_PIM"))
        return OCL_LAUNCH_PIM;
    else if (!segstr.compare("OCL_EVT_WAIT"))
        return OCL_EVT_WAIT;
    else {
        cerr << "Error! Unknown segment ending type found." << endl;
        cerr << segstr << endl;
        return PIM_SEGMENT_ERROR;
    }
}

void do_pt_start(vector<string>& Set_)
{
    // PT_START is a bit weird.
    // 0         1      2      3             4             5
    // PT_START, SysId, Order, Parent_Sysis, Parent_Order, pthreadid
    unsigned long sys_tid = get_sys_tid(Set_);
    if (Set_.size() <= 5) {
        cerr << "Attempting to access a PT_START segment." <<  endl;
        print_malformed_line(Set_);
        exit(-1);
    }
    unsigned long pthread_id = strtoul_helper(Set_[5]);
    unsigned long parent_systid = strtoul_helper(Set_[3]);
    unsigned long long parent_uid = strtoull_helper(Set_[4]);

    ptid_map to_insert;
    to_insert.uid = parent_uid;
    to_insert.system_tid = sys_tid;
    to_insert.pthread_id = pthread_id;
    ptids_to_activate[parent_systid].insert(to_insert);
    return;
}

void do_pt_pim_launch(vector<string>& Set_)
{
    unsigned long sys_tid = get_sys_tid(Set_);
    unsigned long long uid = get_segment_uid(Set_);
    unsigned long destination_pim = get_pim_id(Set_);
    segment_info temp_segment;
    temp_segment.tid = sys_tid;
    temp_segment.uid = uid;
    launch_to_pim[temp_segment] = destination_pim;
    // Later, we will see a PT_LAUNCH that will tell us the pthread_id 
    // associated with this launch to a PIM.
    return;
}

void do_pt_launch(segment_info& segment)
{
    // Come in with SysTID, UID, and ChildTID all in the segment structure.
    // What's to change? Well, if your SysTID:UID is in the OCL_LAUNCH
    // structure, then you're not actually a PT_LAUNCH.
    set<segment_info>::iterator find_ocl_seg;
    find_ocl_seg = orphan_ocl_segments.find(segment);
    if (find_ocl_seg != orphan_ocl_segments.end()) {
        // This is the PT_LAUNCH associated with an OCL_LAUNCH
        // that already happened.
        segment.type = find_ocl_seg->type;
        segment.ocl_evt = find_ocl_seg->ocl_evt;
        orphan_ocl_segments.erase(find_ocl_seg);
    }
}

void do_ocl_launch_helper(segment_info& segment)
{
    set<segment_info>::iterator find_seg;
    find_seg = waiting_for_timestep.find(segment);
    if (find_seg != waiting_for_timestep.end()) {
        // Found the calling PT_LAUNCH. Modify it to make it an OCL_LAUNCH
        // event. You'll also need the OCL event information as well.
        segment_info temp = *find_seg;
        temp.type = segment.type;
        temp.ocl_evt = segment.ocl_evt;
        waiting_for_timestep.erase(find_seg);
        waiting_for_timestep.insert(temp);
    }
    else {
        // PT_LAUNCH hasn't happened yet. Put it in this structure to make
        // it possible to find during the subsequent PT_LAUNCH
        orphan_ocl_segments.insert(segment);
    }
}

void do_ocl_pim_launch(vector<string>& Set_)
{
    unsigned long destination_pim = get_pim_id(Set_);
    int num_wait_events, i;
    segment_info find_me;
    find_me.tid = get_sys_tid(Set_);
    find_me.uid = get_segment_uid(Set_);
    find_me.type = OCL_LAUNCH_PIM;
    find_me.ocl_evt.kernel_name = Set_[4];
    num_wait_events = get_ocl_pim_launch_waitcount(Set_);
    for (i = 0; i < num_wait_events; i++)
        find_me.ocl_evt.wait_events.insert(get_ocl_pim_launch_events(Set_, i));
    find_me.ocl_evt.post_event = get_ocl_pim_launch_events(Set_, i);

    launch_to_pim[find_me] = destination_pim;
    do_ocl_launch_helper(find_me);
}

void do_ocl_launch(vector<string>& Set_)
{
    // See if this segment already exists in the parsed segments list.
    // If so, the PT_LAUNCH has already happened, and that segment needs
    // to have its type changed and ocl_data filled in
    int num_wait_events, i;
    segment_info find_me;
    find_me.tid = get_sys_tid(Set_);
    find_me.uid = get_segment_uid(Set_);
    find_me.type = OCL_LAUNCH;
    find_me.ocl_evt.kernel_name = Set_[3];
    num_wait_events = get_ocl_launch_waitcount(Set_);
    for (i = 0; i < num_wait_events; i++)
        find_me.ocl_evt.wait_events.insert(get_ocl_launch_events(Set_, i));
    // Event this segment will post.
    find_me.ocl_evt.post_event = get_ocl_launch_events(Set_, i);

    do_ocl_launch_helper(find_me);
}

void do_ocl_wait(segment_info& segment, vector<string>& Set_)
{
    int num_wait_events, i;
    segment.type = OCL_EVT_WAIT;
    num_wait_events = get_ocl_waitcount(Set_);
    for (i = 0; i < num_wait_events; i++)
        segment.ocl_evt.wait_events.insert(get_ocl_wait_events(Set_, i));
}

void print_intro_information(const char * conf_file_,
        const char *input_order_file_,
        const char *input_cpu_perf_file_,
        const char *input_gpu_perf_file_,
        char *out_file_)
{
    cout << "\n\nExecuting PIM ordering model...\n";
    cout << "---------------------------------------------------\n\n";
    cout << "model config file: " << conf_file_ << ".\n";
    cout << "model ordering file: " << input_order_file_ << ".\n";
    cout << "model cpu profiling file: " << input_cpu_perf_file_ << ".\n";
    if (input_gpu_perf_file_ != NULL)
        cout << "model gpu profiling file: " << input_gpu_perf_file_ << ".\n";
    else
        cout << "no gpu profiling file." << endl;
    cout << "model output file: " << out_file_ << ".\n\n";
}

void hardware_setup(const pim_emu_configuration_t *pim_config_)
{
    int ehp_cpu_count;
    int ehp_gpu_count;
    int number_of_pims;
    number_of_pims = pim_config_->num_pim_cores;
    pim_cpu_time.resize(number_of_pims);
    pim_gpu_time.resize(number_of_pims);
    pims_cpus_traces.resize(number_of_pims);
    pims_gpus_traces.resize(number_of_pims);
    pim_ready_list.resize(number_of_pims);
    pim_gpu_ready_list.resize(number_of_pims);
    for(int g=0; g < (int) number_of_pims; g++) {
        pim_configuration_t *pim;
        pim = pim_config_->pims + g;
        pim_cpu_time[g].resize(pim->num_cpu_cores);
        pim_gpu_time[g].resize(pim->num_gpu_cores);
        pims_cpus_traces[g].resize(pim->num_cpu_cores);
        pims_gpus_traces[g].resize(pim->num_gpu_cores);
        pim_launch_delay[g] = (unsigned long long)pim_config_->pims[g].ns_to_launch_to_pim;
        pim_end_delay[g] = (unsigned long long)pim_config_->pims[g].ns_to_complete_from_pim;
    }
    ehp_cpu_count = (int) pim_config_->ehp.num_cpu_cores;
    ehp_gpu_count = (int) pim_config_->ehp.num_gpu_cores;

    ehp_cpu_time.resize(ehp_cpu_count);
    ehp_gpu_time.resize(ehp_gpu_count);
    ehp_cpu_traces.resize(ehp_cpu_count);
    ehp_gpu_traces.resize(ehp_gpu_count);
}

int parse_ordering_input(ifstream& order_file, set<segment_info> &parsed_segments)
{
    unsigned long long line_number = 0;
    vector<string> Set;
    string tLine;

    while (order_file.peek() != EOF) {
        segment_info current_segment;

        Set.clear();
        getline(order_file, tLine);
        line_number++;

        //tokenize an order file line
        tokenize(Set, tLine, ",\t");

        // Get global data about this segment. SysTID and unique segment ID
        current_segment.tid = get_sys_tid(Set);
        current_segment.uid = get_segment_uid(Set);
        if (current_segment.uid > maximum_uid_found)
            maximum_uid_found = current_segment.uid;

        // What type of event ends this segment?
        current_segment.type = get_SegmentType(Set);
        current_segment.min_start_time_ns = ULLONG_MAX; // default to "can never run"

        if (current_segment.type == PIM_SEGMENT_ERROR) {
            cerr << "Error was on line " << line_number << endl;
            return -1;
        }
        switch (current_segment.type) {
            case PIMEMU_BEGIN:
            case PIMEMU_LAST:
                // Only main thread in the program can call this
                starting_thread = current_segment.tid;
                final_segment = current_segment.uid;
                // Continue here because we don't want this trace in the
                // final output. Anything that ended with BEGIN was
                // stuff we weren't paying attention to.
                // PIMEMU_LAST never has performance data associated
                // with it.
                continue;
            case PT_START:
                do_pt_start(Set);
                // NOTE: continue. See above for reason why.
                continue;
            case PT_LAUNCH_PIM:
                do_pt_pim_launch(Set);
                // NOTE: continue. See above for reason why
                continue;
            case PT_LAUNCH:
                get_pthread_segment(current_segment, Set);
                do_pt_launch(current_segment);
                break;
            case OCL_LAUNCH:
                have_seen_gpu_segment = true;
                do_ocl_launch(Set);
                // NOTE: continue. The PT_LAUNCH associated with
                // this OCL_LAUNCH is the actual work segment
                continue;
            case OCL_LAUNCH_PIM:
                have_seen_gpu_segment = true;
                do_ocl_pim_launch(Set);
                // NOTE: continue. See OCL_LAUNCH explanation
                continue;
            case OCL_EVT_WAIT:
                have_seen_gpu_segment = true;
                do_ocl_wait(current_segment, Set);
                break;
            case PT_JOIN:
                get_pthread_segment(current_segment, Set);
                // Fallthrough
            case PIMEMU_END:
            case PT_EXIT:
                // Get runtime info later. This segment will be put into the
                // "too run" list.
                break;
            default:
                cerr << "Hit a line we don't support on line: " << line_number << endl;
                return -1;
        }
        // This segment goes into the unordered set of all events,
        // waiting to get pulled and and done in order.
        parsed_segments.insert(current_segment);
    }
    return 0;
}

int parse_cpu_performance_input(ifstream& cpu_in_,
        set<segment_info>& parsed_segments)
{
    unsigned long long line_number = 0;
    vector<string> Set;
    string tLine;

#if DEBUG_TRACE
    cout << "Parsing CPU performance trace." << endl;
#endif
    while (cpu_in_.peek() != EOF) {
        double ehp_runtime;
        double pim_runtime;
        unsigned long long ehp_runtime_in_ns;
        unsigned long long pim_runtime_in_ns;
        bool found_pim_cpu = false;

        Set.clear();
        getline(cpu_in_, tLine);
        line_number++;
        if (line_number == 1) // first line is header
            continue;

        tokenize(Set, tLine, ", ");
        unsigned long tid = get_cpu_perf_systid(Set);
        unsigned long long uid = get_cpu_perf_uid(Set);

        ehp_runtime = get_perf_ehp(Set, 0);
        ehp_runtime_in_ns = (unsigned long long)((ehp_runtime * 1000000000) + 0.5);

        if (pim_cpu_time.size() > 0) {
            unsigned int first_pim_with_cpu;
            for (unsigned int i = 0; i < pim_cpu_time.size(); i++) {
                if (pim_cpu_time[i].size() > 0) {
                    found_pim_cpu = true;
                    first_pim_with_cpu = i;
                    break;
                }
            }
            if (found_pim_cpu) {
                pim_runtime = get_perf_pim(Set, (int)ehp_cpu_time.size(), first_pim_with_cpu, 1, 0);
                pim_runtime_in_ns = (unsigned long long)((pim_runtime * 1000000000) + 0.5);
            }
        }
        
        segment_info find_me;
        find_me.tid = tid;
        find_me.uid = uid;
        set<segment_info>::iterator segment = parsed_segments.find(find_me);

        if (segment == parsed_segments.end()) {
            cerr << "There is a segment in the CPU performance trace that";
            cerr << " was not found in the ordering model." << endl;
            cerr << "TID: " << tid << ". UID: " << uid << endl;
            return -1;
        }
        // Set items are immutable, so we have to make a temp copy,
        // delete old copy, and re-add.
        find_me = *segment;
        parsed_segments.erase(segment);
        find_me.ehp_runtime_ns = ehp_runtime_in_ns;
        if (found_pim_cpu)
            find_me.pim_runtime_ns = pim_runtime_in_ns;
#if DEBUG_TRACE
        cout << "\tSegment " << find_me.tid << ":" << find_me.uid << " runtime in ns: " << endl;
        cout << "\t\tehp: " << find_me.ehp_runtime_ns;
        if (found_pim_cpu) {
            cout << " pim: " << find_me.pim_runtime_ns << endl;
        }
        else {
            cerr << " no pim cpu found" << endl;
        }
#endif
        parsed_segments.insert(find_me);
    }
    return 0;
}

int parse_gpu_performance_input(ifstream& gpu_in_,
        set<segment_info>& parsed_segments)
{
    unsigned long long line_number = 0;
    vector<string> Set;
    string tLine;

#if DEBUG_TRACE
    cout << "Parsing GPU performance trace." << endl;
#endif
    while (gpu_in_.peek() != EOF) {
#if DEBUG_TRACE
        unsigned long long old_uid;
#endif
        double ehp_runtime;
        double pim_runtime;
        unsigned long long ehp_runtime_in_ns;
        unsigned long long pim_runtime_in_ns;
        bool found_pim_gpu = false;

        Set.clear();
        getline(gpu_in_, tLine);
        line_number++;

        tokenize(Set, tLine, ", ");
        unsigned long tid = get_gpu_perf_systid(Set);
        string kernel_name = get_gpu_perf_kernel(Set);

        ehp_runtime = get_perf_ehp(Set, 0);
        ehp_runtime_in_ns = (unsigned long long)((ehp_runtime * 1000000000) + 0.5);
        
        if (pim_gpu_time.size() > 0) {
            for (unsigned int i = 0; i < pim_gpu_time.size(); i++) {
                if (pim_gpu_time[i].size() > 0) {
                    found_pim_gpu = true;
                    break;
                }
            }
            if (found_pim_gpu) {
                if (Set.size() <= 3) {
                    cerr << "Attempting to access a GPU performance trace input " << endl;
                    print_malformed_line(Set);
                    exit(-1);
                }
                pim_runtime = getStatDouble_helper(Set, 3);
                pim_runtime_in_ns = (unsigned long long)((pim_runtime * 1000000000) + 0.5);
            }
        }

        set<segment_info>::iterator set_it = parsed_segments.begin();
        set<segment_info>::iterator found_it = parsed_segments.end();
        while (set_it != parsed_segments.end()) {
            if (set_it->tid == tid) {
                if (set_it->type == PT_EXIT) {
                    // Found the actual "GPU" work done for this thread.
                    // This is the only segment that exists in this
                    // thread. All others will be removed and their
                    // work assigned to the parent thread.
                    found_it = set_it;
                    set_it++;
                }
                else {
                    // All of the other segments in this thread should
                    // be removed. They are artifacts of OpenCL calls
                    // that we do not yet support. Their time should
                    // not be charged to anyone currently.
                    set<segment_info>::iterator temp_it = set_it;
                    set_it++;
                    parsed_segments.erase(temp_it);
                }
                continue;
            }
            set_it++;
        }
        if (found_it == parsed_segments.end()) {
#if DEBUG_TRACE
            cerr << "There is a segment in the GPU performance trace that";
            cerr << " was not found in the ordering model." << endl;
            cerr << "TID: " << tid << endl;
#endif
            continue;
        }
        // Set items are immutable, so we have to make a temp copy,
        // delete old copy, and re-add.
        segment_info temp = *found_it;
        parsed_segments.erase(found_it);
#if DEBUG_TRACE
        old_uid = temp.uid;
#endif
        temp.uid = 1; // Set to 1 because is only real segmen for GPU thread.
        temp.ehp_runtime_ns = ehp_runtime_in_ns;
        if (found_pim_gpu)
            temp.pim_runtime_ns = pim_runtime_in_ns;
        temp.ocl_evt.kernel_name = kernel_name;
#if DEBUG_TRACE
        cout << "\tSegment " << temp.tid << ":" << temp.uid;
        cout << " [was " << temp.tid << ":" << old_uid << "]";
        cout << " (" << temp.ocl_evt.kernel_name << ") runtime in ns: " << endl;
        cout << "\t\tehp: " << temp.ehp_runtime_ns;
        if (found_pim_gpu) {
            cout << " pim: " << temp.pim_runtime_ns << endl;
        }
        else {
            cerr << " no pim gpu found" << endl;
        }
#endif
        parsed_segments.insert(temp);
    }
    return 0;
}

void find_starting_segment(set<segment_info>& unordered_segments,
        multiset<segment_info, compare_start_time>& ready_list)
{
    unsigned long long lowest_segment_id = ULLONG_MAX;
    set<segment_info>::iterator starting_segment;
    segment_info start_info, find_me;

    find_me.tid = starting_thread;
    find_me.uid = 0;

    set<segment_info>::iterator find_seg_iter =
        unordered_segments.lower_bound(find_me);

    // Manually walk through to make sure we find the lowest UID within this TID's schedule
    while (find_seg_iter != unordered_segments.end() &&
            find_seg_iter->tid <= starting_thread) {
        if (find_seg_iter->tid == starting_thread && find_seg_iter->uid < lowest_segment_id) {
            lowest_segment_id = find_seg_iter->uid;
            starting_segment = find_seg_iter;
        }
        find_seg_iter++;
    }
    // Min start time is 0 since this is the first segment in the system.
    start_info = *starting_segment;
    start_info.min_start_time_ns = 0;
    start_info.runtime_ns = start_info.ehp_runtime_ns;
#if DEBUG_TRACE
    cout << "Readying starting segment " << start_info.tid << ":" << start_info.uid << endl;
#endif
    ready_list.insert(start_info);
    unordered_segments.erase(starting_segment);
}

void print_lowest_segment(segment_info segment)
{
    set<segment_info>::iterator find_iter;
    unsigned long long lowest = ULLONG_MAX;
    for (find_iter = waiting_for_timestep.begin();
            find_iter != waiting_for_timestep.end(); find_iter++) {
        if (find_iter->tid == segment.tid) {
            if (find_iter->uid < lowest)
                lowest = find_iter->uid;
        }
    }
    if (lowest != ULLONG_MAX) {
        cerr << "Lowest UID for a segment in that thread is " <<
            lowest << endl;
    }
    else
        cerr << "No other segments from that thread were found.." << endl;
    return;
}

void activate_segment(set<segment_info>::iterator segment,
        unsigned long long min_start_time,
        set<segment_info>& to_remove)
{
    map<unsigned long, unsigned long>::iterator pim_mapping;
    pim_mapping = systid_to_pim.find(segment->tid);
    segment_info temp_segment = *segment;
    temp_segment.min_start_time_ns = min_start_time;
#if DEBUG_TRACE
    cout << "\tActivating segment " << segment->tid << ":" << segment->uid << endl;
#endif
    if (pim_mapping == systid_to_pim.end()) {
        // Put the child onto the EHP
#if DEBUG_TRACE
        cout << "\t\tInserting " << temp_segment.tid << ":";
        cout << temp_segment.uid << "@" << temp_segment.min_start_time_ns;
        cout << " onto EHP ";
        if (systid_is_opencl[temp_segment.tid])
            cout << "GPU ";
        cout << "ready list." << endl;
#endif
        temp_segment.runtime_ns = temp_segment.ehp_runtime_ns;
        if (systid_is_opencl[temp_segment.tid]) // Is OpenCL.
            ehp_gpu_ready_list.insert(temp_segment);
        else // Else is a CPU
            ehp_ready_list.insert(temp_segment);
#if DEBUG_TRACE
        cout << "\tEHP CPU Ready List is now:" << endl;
        multiset<segment_info, compare_start_time>::iterator print_iter =
            ehp_ready_list.begin();
        while(print_iter != ehp_ready_list.end()) {
            cout << "\t\t" << print_iter->tid << ":" << print_iter->uid << "@";
            cout << print_iter->min_start_time_ns << endl;
            print_iter++;
        }
        cout << "\tEHP GPU Ready List is now:" << endl;
        print_iter = ehp_gpu_ready_list.begin();
        while(print_iter != ehp_gpu_ready_list.end()) {
            cout << "\t\t" << print_iter->tid << ":" << print_iter->uid << "@";
            cout << print_iter->min_start_time_ns << endl;
            print_iter++;
        }
#endif
    }
    else {
        // Put the child onto that PIM
#if DEBUG_TRACE
        cout << "\t\tInserting " << temp_segment.tid << ":";
        cout << temp_segment.uid << "@" << temp_segment.min_start_time_ns;
        cout << " onto PIM " << pim_mapping->second << " ";
        if (systid_is_opencl[temp_segment.tid])
            cout << "GPU ";
        cout <<"ready list." << endl;
#endif
        if (pim_mapping->second > pim_ready_list.size()) {
            cerr << "Attempting to schedule onto a PIM that does not exist." << endl;
            cerr << "Segment " << temp_segment.tid << ":" << temp_segment.uid;
            cerr << " requests to be schedule on PIM " << pim_mapping->second << endl;
            cerr << "However, there are only " << pim_ready_list.size();
            cerr << " PIMs configured with this XML file." << endl;
            exit(-1);
        }
        temp_segment.runtime_ns = temp_segment.pim_runtime_ns;
        if (systid_is_opencl[temp_segment.tid]) // Is OpenCL
            (pim_gpu_ready_list[pim_mapping->second]).insert(temp_segment);
        else
            (pim_ready_list[pim_mapping->second]).insert(temp_segment);
#if DEBUG_TRACE 
        cout << "\tPIM " << pim_mapping->second << " CPU Ready List is now: " << endl;
        multiset<segment_info, compare_start_time>::iterator print_iter =
            (pim_ready_list[pim_mapping->second]).begin();
        while(print_iter != (pim_ready_list[pim_mapping->second]).end()) {
            cout << "\t\t" << print_iter->tid << ":" << print_iter->uid << "@" << print_iter->min_start_time_ns << endl;
            print_iter++;
        }
        cout << "\tPIM " << pim_mapping->second << " GPU Ready List is now: " << endl;
        print_iter = (pim_gpu_ready_list[pim_mapping->second]).begin();
        while(print_iter != (pim_gpu_ready_list[pim_mapping->second]).end()) {
            cout << "\t\t" << print_iter->tid << ":" << print_iter->uid << "@" << print_iter->min_start_time_ns << endl;
            print_iter++;
        }
#endif
    }
    to_remove.erase(segment);
}

void waitlist_segment(set<segment_info>::iterator segment, unsigned long long min_start_time_ns, unsigned long systid_to_wait)
{
    segment_info temp;
    temp = *segment;
    temp.min_start_time_ns = min_start_time_ns;
    temp.systid_to_wait = systid_to_wait;
    waiting_for_event.insert(temp);
    waiting_for_timestep.erase(segment);
}

void waitlist_next_segment(multiset<segment_info, compare_start_time>::iterator segment, unsigned long long min_start_time_ns, unsigned long systid_to_wait)
{
    set<segment_info>::iterator iter;

    // We may need to check multiple segments to find the follow-on.
    // Say we're at UID 1 and the next UID in the list is 3. We'll
    // need "to_check=2".
    unsigned int to_check = 1;
    bool done = false;

    if (segment->tid == starting_thread && segment->uid == final_segment &&
            segment->type != PIMEMU_END) {
        cerr << "Attempting to waitlist a follow-on segment for the last ";
        cerr << "segment in the main thread." << endl;
        cerr << "PIMEMU_LAST should be associated with a PIMEMU_END ";
        cerr << "(Type " << PIMEMU_END << ") segment." << endl;
        cerr << "Instead, it is associated with " << segment->tid << ":";
        cerr << segment->uid << " which is a segment of type " << segment->type;
        cerr << "." << endl;
        exit(-1);
    }

#if DEBUG_TRACE
    cout << "\tWaitlisting the follow-on segment to " << segment->tid;
    cout << ":" << segment->uid << endl;
#endif
    // If there's nobody left to schedule, we're done.
    if (waiting_for_timestep.empty())
        return;

    while (!done) {
        segment_info find_me;
        find_me.tid = segment->tid;
        find_me.uid = segment->uid + to_check;
        if (segment->uid + to_check > maximum_uid_found) {
            cerr << "Attempting to waitlist a follow-on segment to the ";
            cerr << "segment " << segment->tid << ":" << segment->uid << endl;
            cerr << "There exists no segment with a higher UID than this ";
            cerr << "in the trace ordering file. Exiting." << endl;
            exit(-1);
        }
#if DEBUG_TRACE
        cout << "\t\tChecking for " << find_me.tid << ":" << find_me.uid << endl;
#endif
        iter = waiting_for_timestep.find(find_me);
        if (iter != waiting_for_timestep.end()) {
#if DEBUG_TRACE
            cout << "\t\tWaitlisting " << segment->tid << ":";
            cout << (segment->uid + to_check) << " as the follow-on." << endl;
#endif
            waitlist_segment(iter, min_start_time_ns, systid_to_wait);
            done = true;
        }
        else {
#if DEBUG_TRACE
            cout << "\t\tCould not find a segment marked " <<
                find_me.tid << ":" << find_me.uid << endl;
#endif
            to_check++;
        }
    }
}

void schedule_next_segment(multiset<segment_info, compare_start_time>::iterator segment,
        unsigned long long at_time)
{
    set<segment_info>::iterator iter;

    // We may need to check multiple segments to find the follow-on.
    // Say we're at UID 1 and the next UID in the list is 3. We'll
    // need "to_check=2".
    unsigned int to_check = 1;
    bool done = false;

    if (segment->tid == starting_thread && segment->uid == final_segment &&
            segment->type != PIMEMU_END) {
        cerr << "Attempting to schedule a follow-on segment for the last ";
        cerr << "segment in the main thread." << endl;
        cerr << "PIMEMU_LAST should be associated with a PIMEMU_END ";
        cerr << "(Type " << PIMEMU_END << ") segment." << endl;
        cerr << "Instead, it is associated with " << segment->tid << ":";
        cerr << segment->uid << " which is a segment of type " << segment->type;
        cerr << "." << endl;
        exit(-1);
    }

#if DEBUG_TRACE
    cout << "\tReadying the follow-on segment to " << segment->tid;
    cout << ":" << segment->uid << endl;
#endif
    // If there's nobody left to schedule, we're done.
    if (waiting_for_timestep.empty())
        return;

    while (!done) {
        segment_info find_me;
        find_me.tid = segment->tid;
        find_me.uid = segment->uid + to_check;
        if (segment->uid + to_check > maximum_uid_found) {
            cerr << "Attempting to schedule the follow-on segment to the ";
            cerr << "segment " << segment->tid << ":" << segment->uid << endl;
            cerr << "There exists no segment with a higher UID than this ";
            cerr << "in the trace ordering file. Exiting." << endl;
            exit(-1);
        }
#if DEBUG_TRACE
        cout << "\t\tChecking for " << find_me.tid << ":" << find_me.uid << endl;
#endif
        iter = waiting_for_timestep.find(find_me);
        if (iter != waiting_for_timestep.end()) {
#if DEBUG_TRACE
            cout << "\t\tReadying " << segment->tid << ":";
            cout << (segment->uid + to_check) << " as the follow-on." << endl;
#endif
            activate_segment(iter, at_time, waiting_for_timestep);
            done = true;
        }
        else {
#if DEBUG_TRACE
            cout << "\t\tCould not find a segment marked " <<
                find_me.tid << " UID " << find_me.uid << endl;
#endif
            to_check++;
        }
    }
}

// Schedule a PThread launch by activating the child thread's
// first segment and also activating the following segment
// of the parent thread.
void schedule_pt_launch(multiset<segment_info, compare_start_time>::iterator segment)
{
    unsigned long child_ptid = segment->pthread_id;
    unsigned long child_systid;
    unsigned long parent_tid = segment->tid;
    unsigned long long parent_uid = segment->uid;
    set<segment_info>::iterator find_iter;
    segment_info find_me;
    ptid_map temp_ptmap;
    map<segment_info, unsigned long>::iterator pim_mapping;
    unsigned long long launch_delay = 0;

    temp_ptmap.uid = parent_uid;

    set<ptid_map>::iterator ptmap_iter = ptids_to_activate[parent_tid].find(temp_ptmap);
    // This will overwrite any "stale" pthread_id maps.
    ptid_to_systid[ptmap_iter->pthread_id] = ptmap_iter->system_tid;
    if (segment->pthread_id != ptmap_iter->pthread_id) {
        cerr << "Error detected. Trying to PT_LAUNCH, but the child's";
        cerr << " pthread_id does not match the one found with PT_START" << endl;
        cerr << "Returned pthread_id: " << segment->pthread_id;
        cerr << " PT_START pthread_id: " << ptmap_iter->pthread_id << endl;
        cerr << "\tparent_tid: " << parent_tid << " parent_uid: " << parent_uid << endl;
        exit(-1);
    }
    child_systid = ptmap_iter->system_tid;
    systid_is_opencl[child_systid] = false;

    // Is that child being launched onto a PIM?
    // Note, this first if statement will only catch if the parent was on
    // the EHP, which is exactly when we want to add this delay.
    pim_mapping = launch_to_pim.find(*segment);
    if (pim_mapping != launch_to_pim.end()) {
        systid_to_pim[child_systid] = pim_mapping->second;
        launch_delay = pim_launch_delay[pim_mapping->second];
    }
    map<unsigned long, unsigned long>::iterator find_parent = systid_to_pim.find(parent_tid);
    if (find_parent != systid_to_pim.end()) {
        systid_to_pim[child_systid] = find_parent->second;
    }

    child_tid_to_parent_tid[child_systid] = parent_tid;

    // Looking for 1 because "0" is the PT_START that's not in this trace
    find_me.tid = child_systid;
    find_me.uid = 1;

    find_iter = waiting_for_timestep.find(find_me);
    if (find_iter != waiting_for_timestep.end()) // found the child
        activate_segment(find_iter, (global_time + segment->runtime_ns + launch_delay), 
                waiting_for_timestep);
    else {
        cerr << "Could not find the first segment of newly launched PT thread " <<
            child_systid << ", pthread_id: " << child_ptid << endl;
        print_lowest_segment(find_me);
        exit(-1);
    }
    schedule_next_segment(segment, (global_time + segment->runtime_ns));
}

void schedule_pt_join(multiset<segment_info, compare_start_time>::iterator segment)
{
    unsigned long ptid_to_wait = segment->pthread_id;
    unsigned long systid_to_wait = ptid_to_systid[ptid_to_wait];
    map<unsigned long, unsigned long long>::iterator found_thread;

#if DEBUG_TRACE
    cout << "\tScheduled a PT_JOIN. Is thread " << systid_to_wait << " done? ";
#endif
    found_thread = finished_threads.find(systid_to_wait);
    if (found_thread != finished_threads.end()) {
#if DEBUG_TRACE
        cout << "Yes. Can schedule follow-on." << endl;
#endif
        unsigned long long soonest_both_avail = found_thread->second;
        // The thread we're joining has already completed. Now we need to
        // figure out when to schedule the follow-on thread.
        if (soonest_both_avail < (global_time + segment->runtime_ns))
            soonest_both_avail = global_time + segment->runtime_ns;
        schedule_next_segment(segment, soonest_both_avail);
    }
    else {
        // We must wait for that thread to complete. Go onto the
        // wait list.
#if DEBUG_TRACE
        cout << "No. Must waitlist follow-on." << endl;
#endif
        waitlist_next_segment(segment, (global_time + segment->runtime_ns), systid_to_wait);
    }
}

void schedule_ocl_evt_wait(multiset<segment_info, compare_start_time>::iterator segment)
{
#if DEBUG_TRACE
    cout << "Scheduled an OpenCL event wait" << endl;
#endif
    set<segment_info>::iterator iter;
    segment_info temp_seg;

    // We may need to check multiple segments to find the follow-on.
    // Say we're at UID 1 and the next UID in the list is 3. We'll
    // need "to_check=2".
    unsigned int to_check = 1;
    bool done = false;

    // If there's nobody left to schedule, we're done.
    if (waiting_for_timestep.empty())
        return;

    while (!done) {
        segment_info find_me;
        find_me.tid = segment->tid;
        find_me.uid = segment->uid + to_check;
        if (segment->uid + to_check > maximum_uid_found) {
            cerr << "Attempting to schedule the follow-on to an OCL event wait ";
            cerr << "at segment " << segment->tid << ":" << segment->uid << endl;
            cerr << "There exists no segment with a higher UID than this ";
            cerr << "in the trace ordering file. Exiting." << endl;
            exit(-1);
        }
        iter = waiting_for_timestep.find(find_me);
        if (iter != waiting_for_timestep.end())
            done = true;
        else
            to_check++;
    }
    // 'iter' is now the follow-on segment that we want to schedule.
    temp_seg = *iter;
    set<unsigned long long>::iterator ocl_event;
    set<unsigned long long> remaining_events;
    unsigned long long soonest_all_events_avail = global_time + segment->runtime_ns;
#if DEBUG_TRACE
    cout << "This thread takes " << segment->runtime_ns << endl;
    cout << "Time to check out all of this wait's events." << endl;
#endif
    // Check all of the wait's OpenCL events
    for (ocl_event = segment->ocl_evt.wait_events.begin();
            ocl_event != segment->ocl_evt.wait_events.end(); ocl_event++) {
        map<unsigned long long, unsigned long long>::iterator completed;
#if DEBUG_TRACE
        cout << "Event number " << *ocl_event << endl;
#endif
        completed = completed_ocl_events.find(*ocl_event);
        // If this event is NOT completed
        if (completed == completed_ocl_events.end()) {
#if DEBUG_TRACE
            cout << "Not found." << endl;
#endif
            remaining_events.insert(*ocl_event);
        }
        else {
#if DEBUG_TRACE
            cout << "Found!" << endl;
#endif
            if (soonest_all_events_avail < completed->second)
                soonest_all_events_avail = completed->second;
        }
#if DEBUG_TRACE
        cout << "Inserting event " << *ocl_event << " into the wait_events list ";
        cout << "for " << temp_seg.tid << ":" << temp_seg.uid << endl;
#endif
        temp_seg.ocl_evt.wait_events.insert(*ocl_event);
    }
    // If all of the events were satisfied, activate this child.
    if (remaining_events.empty()) {
#if DEBUG_TRACE
        cout << "All events found and completed. Scheduling " << iter->tid;
        cout << ":" << iter->uid << " for time " << soonest_all_events_avail << endl;
#endif
        activate_segment(iter, soonest_all_events_avail, waiting_for_timestep);
    }
    else  {
        temp_seg.min_start_time_ns = global_time + segment->runtime_ns;
#if DEBUG_TRACE
        cout << "Not all events found and completed. Putting " << temp_seg.tid;
        cout << ":" << temp_seg.uid << " on the waiting_for_ocl list." << endl;
        cout << "\tEarliest it can start discounting events: ";
        cout << temp_seg.min_start_time_ns << endl;
#endif
        waiting_for_ocl.insert(temp_seg);
        waiting_for_timestep.erase(iter);
    }
}

void schedule_ocl_launch(multiset<segment_info, compare_start_time>::iterator segment)
{
    unsigned long child_ptid = segment->pthread_id;
    unsigned long child_systid;
    unsigned long parent_tid = segment->tid;
    unsigned long long parent_uid = segment->uid;
    set<segment_info>::iterator find_iter;
    segment_info find_me;
    ptid_map temp_ptmap;
    map<segment_info, unsigned long>::iterator pim_mapping;
    unsigned long long launch_delay = 0;

    temp_ptmap.uid = parent_uid;

    set<ptid_map>::iterator ptmap_iter = ptids_to_activate[parent_tid].find(temp_ptmap);
    // This will overwrite any "stale" pthread_id maps.
    ptid_to_systid[ptmap_iter->pthread_id] = ptmap_iter->system_tid;
    if (segment->pthread_id != ptmap_iter->pthread_id) {
        cerr << "Error detected. Trying to OCL_LAUNCH, but the child's";
        cerr << " pthread_id does not match the one found with PT_START" << endl;
        cerr << "Returned pthread_id: " << segment->pthread_id;
        cerr << " PT_START pthread_id: " << ptmap_iter->pthread_id << endl;
        cerr << "\tparent_tid: " << parent_tid << " parent_uid: " << parent_uid << endl;
        exit(-1);
    }
    child_systid = ptmap_iter->system_tid;

    // Is that child being launched onto a PIM?
    // Note, this first if statement will only catch if the parent was on
    // the EHP, which is exactly when we want to add this delay.
    pim_mapping = launch_to_pim.find(*segment);
    if (pim_mapping != launch_to_pim.end()) {
        systid_to_pim[child_systid] = pim_mapping->second;
        launch_delay = pim_launch_delay[pim_mapping->second];
    }
    map<unsigned long, unsigned long>::iterator find_parent = systid_to_pim.find(parent_tid);
    if (find_parent != systid_to_pim.end()) {
        systid_to_pim[child_systid] = find_parent->second;
    }

    child_tid_to_parent_tid[child_systid] = parent_tid;

    // Looking for 1 because "0" is the PT_START that's not in this trace
    find_me.tid = child_systid;
    find_me.uid = 1;

    find_iter = waiting_for_timestep.find(find_me);
    if (find_iter != waiting_for_timestep.end()) { // found the child
        set<unsigned long long>::iterator ocl_event;
        set<unsigned long long> remaining_events;
        unsigned long long soonest_all_events_avail = global_time + segment->runtime_ns + launch_delay;
        systid_is_opencl[find_me.tid] = true;
        // Parent actually holds the OpenCL events for its child.
        segment_info temp = *find_iter;
        waiting_for_timestep.erase(find_iter);
        temp.ocl_evt = segment->ocl_evt;
        waiting_for_timestep.insert(temp);
        find_iter = waiting_for_timestep.find(temp);

        // Check all of the child's OpenCL events
        for (ocl_event = find_iter->ocl_evt.wait_events.begin();
                ocl_event != find_iter->ocl_evt.wait_events.end(); ocl_event++) {
            map<unsigned long long, unsigned long long>::iterator completed;
            completed = completed_ocl_events.find(*ocl_event);
            // If this event is NOT completed
            if (completed == completed_ocl_events.end())
                remaining_events.insert(*ocl_event);
            else {
                if (soonest_all_events_avail < completed->second)
                    soonest_all_events_avail = completed->second;
            }
        }
        // If all of the events were satisfied, activate this child.
        if (remaining_events.empty())
            activate_segment(find_iter, soonest_all_events_avail, waiting_for_timestep);
        else  {
            // This is the minimum start time without looking at events.
            // We must know this in case all of the events finish before the actual
            // launch segment ends. Otherwise, a child could happen before
            // the parent creates it..
            segment_info temp_seg = *find_iter;
            temp_seg.min_start_time_ns = global_time + segment->runtime_ns + launch_delay;
            waiting_for_ocl.insert(temp_seg);
            waiting_for_timestep.erase(find_iter);
        }
    }
    else {
        cerr << "Could not find the first segment of newly launched OCL thread " <<
            child_systid << ", pthread_id: " << child_ptid << endl;
        print_lowest_segment(find_me);
        exit(-1);
    }
    // We mark the parent thread as complete because, technically, it doesn't
    // exist. It's a zero-time thread, and the next segment, which is a
    // PT_JOIN, can easily schedule now.
    finished_threads[child_systid] = global_time + segment->runtime_ns;
    schedule_next_segment(segment, (global_time + segment->runtime_ns));
}

void schedule_pt_exit(multiset<segment_info, compare_start_time>::iterator segment)
{
    set<segment_info>::iterator iter, temp_iter;
    unsigned long long end_delay = 0;
    // Check to see if this is running on the PIM.
    // If so, and if its parent is NOT on a PIM, then there is a delay
    // before the response can go back.
    map<unsigned long, unsigned long>::iterator our_pim = systid_to_pim.find(segment->tid);
    unsigned long parent_tid = child_tid_to_parent_tid[segment->tid];
    map<unsigned long, unsigned long>::iterator parent_pim = systid_to_pim.find(parent_tid);
    if (our_pim != systid_to_pim.end()) {
        // This was on a PIM.
        if (parent_pim == systid_to_pim.end()) {
            // Parent wasn't.
            end_delay = pim_end_delay[our_pim->second];
        }
    }
    if (!systid_is_opencl[segment->tid]) {
        finished_threads[segment->tid] = (global_time + segment->runtime_ns + end_delay);
#if DEBUG_TRACE
        cout << "\tScheduling real PT_EXIT. Search for threads joining on ";
        cout << segment->tid << ":" << segment->uid << " at time ";
        cout << (global_time + segment->runtime_ns + end_delay) << endl;
#endif
        iter = waiting_for_event.begin();
        while (iter != waiting_for_event.end()) {
            temp_iter = iter;
            temp_iter++;
#if DEBUG_TRACE
            cout << "\tThread " << iter->tid << ":" << iter->uid;
            cout << " is waiting on systid " << iter->systid_to_wait << endl;
#endif
            if (iter->systid_to_wait == segment->tid) {
                unsigned long long soonest_both_avail = iter->min_start_time_ns;
#if DEBUG_TRACE
                cout << "\t\tThread " << iter->tid << " was waiting. It was ready at time ";
                cout << iter->min_start_time_ns << endl;
                cout << "\t\tThis thread ends at time " << (global_time + segment->runtime_ns + end_delay) << endl;
#endif
                if (soonest_both_avail < (global_time + segment->runtime_ns + end_delay))
                    soonest_both_avail = (global_time + segment->runtime_ns + end_delay);
                // Killing off this iterator, so iter will be invalid afterwards
                activate_segment(iter, soonest_both_avail, waiting_for_event);
            }
            iter = temp_iter;
        }
    }

    // Now, if this aws an OpenCL segment, we need to see if this OpenCL
    // event will wake anyone else up.
    if (systid_is_opencl[segment->tid]) {
        // Then this thread ending causes a new event to be finished.
#if DEBUG_TRACE
        cout << "Scheduled OpenCL work. Event " << segment->ocl_evt.post_event;
        cout << " will post at time " << (global_time + segment->runtime_ns + end_delay) << endl;
#endif
        completed_ocl_events[segment->ocl_evt.post_event] =
            (global_time + segment->runtime_ns + end_delay);

        // Walk through the list of all OpenCL segments that are waiting for
        // an event to complete.
#if DEBUG_TRACE
        cout << "Time to see if we should activate any segments that are waiting on an event." << endl;
#endif
        for (iter = waiting_for_ocl.begin(); iter != waiting_for_ocl.end();) {
            set<unsigned long long>::iterator ocl_event;
            set<unsigned long long> remaining_events;
            // We start our "soonest time" at the time that the parent creating this child
            // was done. Now we need to see when all the events were done, as well.
            unsigned long long soonest_all_events_avail = iter->min_start_time_ns;
#if DEBUG_TRACE
            cout << "Segment " << iter->tid << ":" << iter->uid << " is waiting on: " << endl;
#endif
            // Walk through all of the events this segment is waiting on.
            for (ocl_event = iter->ocl_evt.wait_events.begin();
                    ocl_event != iter->ocl_evt.wait_events.end();
                    ocl_event++) {
                map<unsigned long long, unsigned long long>::iterator completed;
                // Has this event completed?
#if DEBUG_TRACE
                cout << "\tEvent #" << *ocl_event << ": ";
#endif
                completed = completed_ocl_events.find(*ocl_event);
                // If this event is NOT completed
                if (completed == completed_ocl_events.end()) {
                    // Only need one not completed to know we're done here.
#if DEBUG_TRACE
                    cout << "NOT FOUND." << endl;
#endif
                    remaining_events.insert(*ocl_event);
                    break;
                }
                else {
#if DEBUG_TRACE
                    cout << "FOUND at time " << completed->second << endl;
#endif
                    if (soonest_all_events_avail < completed->second)
                        soonest_all_events_avail = completed->second;
                }
            }
            set<segment_info>::iterator maybe_delete = iter;
            // We must ++ iter here because if we call activate_segment
            // it will remove it from waiting_for_ocl and invalidate
            // the iterator.
            iter++;
#if DEBUG_TRACE
            cout << "Now, do we activate this segment?" << endl;
#endif
            if (remaining_events.empty()) {
#if DEBUG_TRACE
                cout << "Yes. At time: " << soonest_all_events_avail << endl;
#endif
                activate_segment(maybe_delete, soonest_all_events_avail,
                        waiting_for_ocl);
            }
#if DEBUG_TRACE
            else
                cout << "No." << endl;
#endif
        }
    }
}

void schedule_cpu_task(multiset<segment_info, compare_start_time>::iterator segment)
{
    // This will get much more complicated with
    // OpenCL events, etc. We'll cross that bridge when we
    // get to it.
    switch (segment->type) {
        case PT_LAUNCH:
        case PT_LAUNCH_PIM:
            // It's OK to do both here, because we know through the
            // systid->pim structure where every child is supposed
            // to run.
            schedule_pt_launch(segment);
            break;
        case PT_JOIN:
            schedule_pt_join(segment);
            break;
        case PIMEMU_END:
            if (segment->tid == starting_thread && segment->uid == final_segment) {
#if DEBUG_TRACE
                cout << "\tFinal PIMEMU_END segment in the main thrad. Not scheduling follow-on" << endl;
#endif
                break;
            }
            // Fallthrough
        case PIMEMU_BEGIN:
            // Activate segment that follows this thread
            schedule_next_segment(segment, (global_time + segment->runtime_ns));
            break;
        case PT_EXIT:
        case PIMEMU_LAST:
            // Add this thread to finished_threads.
            // See if anyone in the real waiting structure was waiting on this thread to finish
            // If so, insert that segment into the ready list.
            schedule_pt_exit(segment);
            break;
        case OCL_LAUNCH:
        case OCL_LAUNCH_PIM:
            schedule_ocl_launch(segment);
            break;
        case OCL_EVT_WAIT:
            schedule_ocl_evt_wait(segment);
            break;
        default:
            cerr << "Unknown type when scheduling a CPU task!" << endl;
            cerr << segment->tid << ":" << segment->uid << " " << segment->type << endl;
            exit(-1);
    }
}

void do_ehp_work_at_this_timestep()
{
    multiset<segment_info, compare_start_time>::iterator seg_iter;
    for (unsigned int i = 0; i < ehp_cpu_time.size(); i++) {
        // If the CPU is free at the current tiemstep
        if (ehp_cpu_time[i] == global_time) {
#if DEBUG_TRACE
            cout << "Attempting to assign work to EHP CPU " << i;
            cout << "@" << global_time << endl;
#endif
            // Then is the earliest possible job in the CPU queue able to run at this timestep?
            seg_iter = ehp_ready_list.begin();
#if DEBUG_TRACE
            if (seg_iter != ehp_ready_list.end()) {
                cout << "\tHow about segment " << seg_iter->tid << ":" << seg_iter->uid;
                cout << "@" << seg_iter->min_start_time_ns << "?" <<  endl;
            }
#endif
            if (seg_iter != ehp_ready_list.end() &&
                    seg_iter->min_start_time_ns <= global_time) {
#if DEBUG_TRACE
                cout << "\tScheduling " << seg_iter->tid << ":" << seg_iter->uid;
                cout << " onto EHP CPU " << i << ". ";
                cout << "Time taken: " << seg_iter->runtime_ns << endl;
#endif
                // This segment gets scheduled on this CPU core at the global time.
                output_trace executing_trace;
                executing_trace.thread_id = seg_iter->tid;
                executing_trace.start_time = global_time;
                executing_trace.end_time = global_time + seg_iter->runtime_ns;
                schedule_cpu_task(seg_iter);
                ehp_cpu_time[i] += seg_iter->runtime_ns;
                ehp_cpu_traces[i].push(executing_trace);
#if DEBUG_TRACE
                cout << "\tErasing " << seg_iter->tid << ":" << seg_iter->uid;
                cout << " from the EHP CPU ready-to-run segment list." << endl;
#endif
                ehp_ready_list.erase(seg_iter);
            }
        }
#if DEBUG_TRACE
        else { // Else this processor is busy doing work.
            cout << "\tCannot assign work to EHP CPU " << i << "@";
            cout << global_time << ". Busy until: " << ehp_cpu_time[i] << endl;
        }
#endif
    }
    for (unsigned int i = 0; i < ehp_gpu_time.size(); i++) {
        if (ehp_gpu_time[i] == global_time) {
#if DEBUG_TRACE
            cout << "Attempting to assign work to EHP GPU " << i;
            cout << "@" << global_time << endl;
#endif
            seg_iter = ehp_gpu_ready_list.begin();
#if DEBUG_TRACE
            if (seg_iter != ehp_gpu_ready_list.end()) {
                cout << "\tHow about segment " << seg_iter->tid << ":" << seg_iter->uid;
                cout << "@" << seg_iter->min_start_time_ns << "?" <<  endl;
            }
#endif
            if (seg_iter != ehp_gpu_ready_list.end() &&
                    seg_iter->min_start_time_ns <= global_time) {
#if DEBUG_TRACE
                cout << "\tScheduling " << seg_iter->tid << ":" << seg_iter->uid;
                cout << " onto EHP GPU " << i << ". ";
                cout << "Time taken: " << seg_iter->runtime_ns << endl;
#endif
                output_trace executing_trace;
                executing_trace.thread_id = seg_iter->tid;
                executing_trace.kernel_name = seg_iter->ocl_evt.kernel_name;
                executing_trace.start_time = global_time;
                executing_trace.end_time = global_time + seg_iter->runtime_ns;
                schedule_cpu_task(seg_iter);
                ehp_gpu_time[i] += seg_iter->runtime_ns;
                ehp_gpu_traces[i].push(executing_trace);
#if DEBUG_TRACE
                cout << "\tErasing " << seg_iter->tid << ":" << seg_iter->uid;
                cout << " from the EHP GPU ready-to-run segment list." << endl;
#endif
                ehp_gpu_ready_list.erase(seg_iter);
            }
        }
#if DEBUG_TRACE
        else { // Else this processor is busy doing work.
            cout << "Cannot assign work to EHP GPU " << i << "@";
            cout << global_time << ". Busy until: " << ehp_gpu_time[i] << endl;
        }
#endif

    }
}

void do_pim_work_at_this_timestep()
{
    multiset<segment_info, compare_start_time>::iterator seg_iter;
    for (unsigned int i = 0; i < pim_cpu_time.size(); i++) {
        // For every CPU on this PIM...
        for (unsigned int j = 0; j < pim_cpu_time[i].size(); j++) {
            // Is it able to run at this timestep?
            if (pim_cpu_time[i][j] == global_time) {
#if DEBUG_TRACE
                cout << "Attempting to assign work to PIM " << i << " CPU " << j;
                cout << "@" << global_time << endl;
#endif
                // If so, check it vs the first task that can run on this PIM
                seg_iter = pim_ready_list[i].begin();
#if DEBUG_TRACE
                if (seg_iter != pim_ready_list[i].end()) {
                    cout << "\tHow about segment " << seg_iter->tid << ":" << seg_iter->uid;
                    cout << "@" << seg_iter->min_start_time_ns << "?" <<  endl;
                }
#endif
                if (seg_iter != pim_ready_list[i].end() &&
                        seg_iter->min_start_time_ns <= global_time) {
                    // This segment gets scheduled on the PIM's CPU core at the global time.
#if DEBUG_TRACE
                    cout << "\tScheduling " << seg_iter->tid << ":" << seg_iter->uid;
                    cout << " onto PIM " << i << " CPU " << j << ". ";
                    cout << "Time taken: " << seg_iter->runtime_ns << endl;
#endif
                    output_trace executing_trace;
                    executing_trace.thread_id = seg_iter->tid;
                    executing_trace.start_time = global_time;
                    executing_trace.end_time = global_time + seg_iter->runtime_ns;
                    schedule_cpu_task(seg_iter);
                    pim_cpu_time[i][j] += seg_iter->runtime_ns;
                    pims_cpus_traces[i][j].push(executing_trace);
#if DEBUG_TRACE
                    cout << "\tErasing " << seg_iter->tid << ":" << seg_iter->uid;
                    cout << " from the PIM " << i << " CPU ready-to-run segment list." << endl;
#endif
                    pim_ready_list[i].erase(seg_iter);
                }
            }
#if DEBUG_TRACE
            else { // Else this processor is busy doing work.
                cout << "Cannot assign work to PIM " << i << " CPU " << j << "@";
                cout << global_time << ". Busy until: " << pim_cpu_time[i][j] << endl;
            }
#endif
        }
        // For every GPU on this PIM...
        for (unsigned int j = 0; j < pim_gpu_time[i].size(); j++) {
            // Is it able to run at this timestep?
            if (pim_gpu_time[i][j] == global_time) {
#if DEBUG_TRACE
                cout << "Attempting to assign work to PIM " << i << " GPU " << j;
                cout << "@" << global_time << endl;
#endif
                // If so, check it vs the first task that can run on this PIM
                seg_iter = pim_gpu_ready_list[i].begin();
#if DEBUG_TRACE
                if (seg_iter != pim_gpu_ready_list[i].end()) {
                    cout << "\tHow about segment " << seg_iter->tid << ":" << seg_iter->uid;
                    cout << "@" << seg_iter->min_start_time_ns << "?" <<  endl;
                }
#endif
                if (seg_iter != pim_gpu_ready_list[i].end() &&
                        seg_iter->min_start_time_ns <= global_time) {
                    // This segment gets scheduled on the PIM's GPU core at the global time.S
#if DEBUG_TRACE
                    cout << "\tScheduling " << seg_iter->tid << ":" << seg_iter->uid;
                    cout << " onto PIM " << i << " GPU " << j << ". ";
                    cout << "Time taken: " << seg_iter->runtime_ns << endl;
#endif
                    output_trace executing_trace;
                    executing_trace.thread_id = seg_iter->tid;
                    executing_trace.kernel_name = seg_iter->ocl_evt.kernel_name;
                    executing_trace.start_time = global_time;
                    executing_trace.end_time = global_time + seg_iter->runtime_ns;
                    schedule_cpu_task(seg_iter);
                    pim_gpu_time[i][j] += seg_iter->runtime_ns;
                    pims_gpus_traces[i][j].push(executing_trace);
#if DEBUG_TRACE
                    cout << "\tErasing " << seg_iter->tid << ":" << seg_iter->uid;
                    cout << " from the PIM " << i << " GPU ready-to-run segment list." << endl;
#endif
                    pim_gpu_ready_list[i].erase(seg_iter);
                }
            }
#if DEBUG_TRACE
            else { // Else this processor is busy doing work.
                cout << "Cannot assign work to PIM " << i << " GPU " << j << "@";
                cout << global_time << ". Busy until: " << pim_gpu_time[i][j] << endl;
            }
#endif
        }
    }
}

unsigned long long find_min_ready_work_time_ehp_cpu()
{
    multiset<segment_info, compare_start_time>::iterator seg_iter;
    unsigned long long min_time = ULLONG_MAX;
    seg_iter = ehp_ready_list.begin();
    if (seg_iter != ehp_ready_list.end()) {
        min_time = seg_iter->min_start_time_ns;
    }
    return min_time;
}

unsigned long long find_min_ready_work_time_ehp_gpu()
{
    multiset<segment_info, compare_start_time>::iterator seg_iter;
    unsigned long long min_time = ULLONG_MAX;
    seg_iter = ehp_gpu_ready_list.begin();
    if (seg_iter != ehp_gpu_ready_list.end()) {
        if (seg_iter->min_start_time_ns < min_time) {
            min_time = seg_iter->min_start_time_ns;
        }
    }
    return min_time;
}

unsigned long long find_min_ready_work_time_pim_cpu(int which_pim)
{
    multiset<segment_info, compare_start_time>::iterator seg_iter;
    unsigned long long min_time = ULLONG_MAX;
    seg_iter = pim_ready_list[which_pim].begin();
    if (seg_iter != pim_ready_list[which_pim].end())
        min_time = seg_iter->min_start_time_ns;
    return min_time;
}

unsigned long long find_min_ready_work_time_pim_gpu(int which_pim)
{
    multiset<segment_info, compare_start_time>::iterator seg_iter;
    unsigned long long min_time = ULLONG_MAX;
    seg_iter = pim_gpu_ready_list[which_pim].begin();
    if (seg_iter != pim_gpu_ready_list[which_pim].end()) {
        if (seg_iter->min_start_time_ns < min_time) {
            min_time = seg_iter->min_start_time_ns;
        }
    }
    return min_time;
}

unsigned long long find_min_hw_time_ehp_cpu()
{
    unsigned long long min_time = ULLONG_MAX;
    for (unsigned int i = 0; i < ehp_cpu_time.size(); i++) {
        if (ehp_cpu_time[i] < min_time)
            min_time = ehp_cpu_time[i];
    }
    return min_time;
}

unsigned long long find_min_hw_time_ehp_gpu()
{
    unsigned long long min_time = ULLONG_MAX;
    for (unsigned int i = 0; i < ehp_gpu_time.size(); i++) {
        if (ehp_gpu_time[i] < min_time)
            min_time = ehp_gpu_time[i];
    }
    return min_time;
}

unsigned long long find_min_hw_time_pim_cpu(int which_pim)
{
    unsigned long long min_time = ULLONG_MAX;
    for (unsigned int i = 0; i < pim_cpu_time[which_pim].size(); i++) {
        if (pim_cpu_time[which_pim][i] < min_time)
            min_time = pim_cpu_time[which_pim][i];
    }
    return min_time;
}

unsigned long long find_min_hw_time_pim_gpu(int which_pim)
{
    unsigned long long min_time = ULLONG_MAX;
    for (unsigned int i = 0; i < pim_gpu_time[which_pim].size(); i++) {
        if (pim_gpu_time[which_pim][i] < min_time)
            min_time = pim_gpu_time[which_pim][i];
    }
    return min_time;
}

unsigned long long find_next_work_time(unsigned long long ehp_cpu_work,
        unsigned long long ehp_gpu_work, unsigned long long ehp_cpu_hw,
        unsigned long long ehp_gpu_hw,
        vector<unsigned long long>& pim_cpu_work,
        vector<unsigned long long>& pim_gpu_work,
        vector<unsigned long long>& pim_cpu_hw,
        vector<unsigned long long>& pim_gpu_hw)
{
    unsigned long long temp = ULLONG_MAX;
    unsigned long long min_work = ULLONG_MAX;
    if (ehp_cpu_hw < ehp_cpu_work)
        min_work = ehp_cpu_work;
    else
        min_work = ehp_cpu_hw;

    if (ehp_gpu_hw < ehp_gpu_work)
        temp = ehp_gpu_work;
    else
        temp = ehp_gpu_hw;
    if (temp < min_work)
        min_work = temp;

    for (unsigned int i = 0; i < pim_cpu_work.size(); i++) {
        temp = ULLONG_MAX;
        if (pim_cpu_hw[i] < pim_cpu_work[i])
            temp = pim_cpu_work[i];
        else
            temp = pim_cpu_hw[i];
        if (temp < min_work)
            min_work = temp;
    }
    
    for (unsigned int i = 0; i < pim_gpu_work.size(); i++) {
        temp = ULLONG_MAX;
        if (pim_gpu_hw[i] < pim_gpu_work[i])
            temp = pim_gpu_work[i];
        else
            temp = pim_gpu_hw[i];
        if (temp < min_work)
            min_work = temp;
    }
    return min_work;
}

void update_timestep()
{
    // The next global mintime is the max of the earliest thing on all of
    // the ready lists and the earliest time that you can put it on a piece
    // of hardware.
    unsigned long long min_time_ehp_cpu_work_available = ULLONG_MAX;
    unsigned long long min_time_ehp_gpu_work_available = ULLONG_MAX;
    vector<unsigned long long> min_time_pim_cpu_work_available(pim_cpu_time.size(), ULLONG_MAX);
    vector<unsigned long long> min_time_pim_gpu_work_available(pim_gpu_time.size(), ULLONG_MAX);
    unsigned long long min_time_ehp_cpu_hw_available = ULLONG_MAX;
    unsigned long long min_time_ehp_gpu_hw_available = ULLONG_MAX;
    vector<unsigned long long> min_time_pim_cpu_hw_available(pim_cpu_time.size(), ULLONG_MAX);
    vector<unsigned long long> min_time_pim_gpu_hw_available(pim_gpu_time.size(), ULLONG_MAX);

    min_time_ehp_cpu_work_available = find_min_ready_work_time_ehp_cpu();
    min_time_ehp_gpu_work_available = find_min_ready_work_time_ehp_gpu();
    min_time_ehp_cpu_hw_available = find_min_hw_time_ehp_cpu();
    min_time_ehp_gpu_hw_available = find_min_hw_time_ehp_gpu();
    for (unsigned int i = 0; i < pim_cpu_time.size(); i++) {
        min_time_pim_cpu_work_available[i] = find_min_ready_work_time_pim_cpu(i);
        min_time_pim_cpu_hw_available[i] = find_min_hw_time_pim_cpu(i);
    }
    for (unsigned int i = 0; i < pim_gpu_time.size(); i++) {
        min_time_pim_gpu_work_available[i] = find_min_ready_work_time_pim_gpu(i);
        min_time_pim_gpu_hw_available[i] = find_min_hw_time_pim_gpu(i);
    }
    global_time = find_next_work_time(min_time_ehp_cpu_work_available,
            min_time_ehp_gpu_work_available, min_time_ehp_cpu_hw_available,
            min_time_ehp_gpu_hw_available, min_time_pim_cpu_work_available,
            min_time_pim_gpu_work_available, min_time_pim_cpu_hw_available,
            min_time_pim_gpu_hw_available);

#if DEBUG_TRACE
    cout << "Updating time on processors based on work available." << endl;
    cout << "\tmin time ehp cpu work available " << min_time_ehp_cpu_work_available << endl;
    cout << "\tmin time ehp cpu hardware available " << min_time_ehp_cpu_hw_available << endl;
    cout << "\tmin time ehp gpu work available " << min_time_ehp_gpu_work_available << endl;
    cout << "\tmin time ehp gpu hardware available " << min_time_ehp_gpu_hw_available << endl;
    for (unsigned int i = 0; i < pim_cpu_time.size(); i++) {
        cout << "\tmin time pim " << i << " cpu work available ";
        cout << min_time_pim_cpu_work_available[i] << endl;
        cout << "\tmin time pim " << i << " cpu hardware available ";
        cout << min_time_pim_cpu_hw_available[i] << endl;
        cout << "\tmin time pim " << i << " gpu work available ";
        cout << min_time_pim_cpu_work_available[i] << endl;
        cout << "\tmin time pim " << i << " gpu hardware available ";
        cout << min_time_pim_gpu_hw_available[i] << endl;
    }
    cout << "\tNew Global Time is " << global_time << endl;
#endif

    // Update CPU's min times.
    for (unsigned int i = 0; i < ehp_cpu_time.size(); i++) {
        if (ehp_cpu_time[i] < global_time)
            ehp_cpu_time[i] = global_time;
    }
    for (unsigned int i = 0; i < ehp_gpu_time.size(); i++) {
        if (ehp_gpu_time[i] < global_time)
            ehp_gpu_time[i] = global_time;
    }
    for (unsigned int i = 0; i < pim_cpu_time.size(); i++) {
        for (unsigned int j = 0; j < pim_cpu_time[i].size(); j++) {
            if (pim_cpu_time[i][j] < global_time)
                pim_cpu_time[i][j] = global_time;
        }
        for (unsigned int j = 0; j < pim_gpu_time[i].size(); j++) {
            if (pim_gpu_time[i][j] < global_time)
                pim_gpu_time[i][j] = global_time;
        }
    }
}

void do_ordering_work_timestep()
{
#if DEBUG_TRACE
    static int timestep = 0;
    timestep++;
    cout << "Timestep " << timestep << endl;
#endif
    do_ehp_work_at_this_timestep();
    do_pim_work_at_this_timestep();
    
    // The next global mintime is the earliest thing in all of the ready lists.
    update_timestep();
}

void print_helper(ofstream &out, vector<queue<output_trace> >& print_traces, unsigned long long& total_runtime, string device_name, bool is_gpu)
{
    for (unsigned int i = 0; i < print_traces.size(); i++) {
        output_trace to_output;
        if (!print_traces[i].empty()) {
            while (!print_traces[i].empty()) {
                to_output = print_traces[i].front();
                print_traces[i].pop();
                out << device_name << i << ", ";
                out << to_output.thread_id << ", ";
                if (is_gpu)
                    out << to_output.kernel_name << ", ";
                else
                    out << "NA, ";
                out << to_output.start_time << ", ";
                out << to_output.end_time << endl;
                if (total_runtime < to_output.end_time)
                    total_runtime = to_output.end_time;
            }
        }
        else
            out << device_name << i << ", NA, NA, NA, NA" << endl;
        out << endl;
    }
}

void print_output(ofstream &out)
{
    out << "DeviceID, ThreadID, FunctionName, StartTime_ns, EndTime_ns" << endl;
    unsigned long long total_runtime = 0;
    string pim_name = "pim";
    print_helper(out, ehp_cpu_traces, total_runtime, "ehp-cpu", false);
    print_helper(out, ehp_gpu_traces, total_runtime, "ehp-gpu", true);
    for (unsigned int i = 0; i < pims_cpus_traces.size(); i++) {
        char number[21];
        sprintf(number, "%d", i);
        string this_pim = "pim";
        this_pim += number;
        string cpu_pim = this_pim + "-cpu";
        string gpu_pim = this_pim + "-gpu";
        print_helper(out, pims_cpus_traces[i], total_runtime, cpu_pim, false);
        print_helper(out, pims_gpus_traces[i], total_runtime, gpu_pim, true);
    }
    out << "Total runtime: " << total_runtime << endl;
}

int pimOrderingModel (const char * conf_file_,
        const char *input_order_file_,
        const char *input_cpu_perf_file_,
        const char *input_gpu_perf_file_,
        char *out_file_,
        const pim_emu_configuration_t *pim_config_,
        ehp_conf_t *ehp_conf_)
{
    global_time = 0;

    ifstream order_in(input_order_file_);
    if (!order_in.is_open()) {
        cerr << "Error opening ordering file \"";
        cerr << input_order_file_ << "\"" << endl;
        return -1;
    }
    ifstream cpu_in(input_cpu_perf_file_);
    if (!cpu_in.is_open()) {
        cerr << "Error opening CPU performance trace file \"";
        cerr << input_cpu_perf_file_ << "\"" << endl;
        return -1;
    }
    ifstream gpu_in;
    if (input_gpu_perf_file_ != NULL) {
        gpu_perf_exists = true;
        gpu_in.open(input_gpu_perf_file_);
        if (!gpu_in.is_open()) {
            cerr << "Error opening GPU performance trace file \"";
            cerr << input_gpu_perf_file_ << "\"" << endl;
            return -1;
        }
    }
    else
        gpu_perf_exists = false;
    have_seen_gpu_segment = false;
    ofstream out(out_file_);
    if (!out.is_open()) {
        cerr << "Error opening output file \"";
        cerr << out_file_ << "\"" << endl;
        return -1;
    }

    vector<string> Set;
    string tLine;

    print_intro_information(conf_file_, input_order_file_,
                input_cpu_perf_file_, input_gpu_perf_file_, out_file_);

    // Set up the structures describing the hardware.
    hardware_setup(pim_config_);

    // Bring in the input ordering file. Throw things into the unordered structs.
    if (parse_ordering_input(order_in, waiting_for_timestep)) {
        cerr << "Error parsing ordering file" << endl;
        return -1;
    }

#if DEBUG_TRACE
    cout << "Entire collection of segments that will be dynamically scheduled: " << endl;
    set<segment_info>::iterator wait_iter;
    for (wait_iter = waiting_for_timestep.begin(); wait_iter != waiting_for_timestep.end(); wait_iter++) {
        cout << "\ttid: " << wait_iter->tid << " uid: " << wait_iter->uid;
        cout << " type: " << wait_iter->type << " pthread_id: ";
        cout << wait_iter->pthread_id << endl;
    }
#endif

    // Read performance information from the CPU and GPU ordering traces
    // Put them into the unordered sets.
    if (parse_cpu_performance_input(cpu_in, waiting_for_timestep)) {
        cerr << "Error parsing CPU performance file" << endl;
        return -1;
    }
    if (gpu_perf_exists) {
        if (parse_gpu_performance_input(gpu_in, waiting_for_timestep)) {
            cerr << "Error parsing GPU performance file" << endl;
            return -1;
        }
    }
    else if (have_seen_gpu_segment) {
        cerr << "GPU/OpenCL segment found in ordering file, but no GPU ";
        cerr << "performance information passed into the program." << endl;
        return -1;
    }

    // We've finally loaded all the data in from the CSV files.
    // The waiting_for_timesteps structure contains all of our segments
    // and it is sorted based on Thread ID and UID.

    // Find starting segment and put it into the ready list.
    find_starting_segment(waiting_for_timestep, ehp_ready_list);
    systid_is_opencl[starting_thread] = false;

    while (global_time != ULLONG_MAX) {
        // this will set the global time to ULLONG_MAX when there is no more
        // work to do.
        do_ordering_work_timestep();
    }

    print_output(out);

    return 0;
}
