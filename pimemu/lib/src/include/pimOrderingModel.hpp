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

#ifndef __PIMORDERINGMODEL_HPP_
#define __PIMORDERINGMODEL_HPP_

#include "pimAModel.hpp"
#include <limits.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <queue>

// Segment ending events. Each of these is a type of event that can cause a
// segment to complete (in other words, these are the types of events that
// cause ordering constraints bewteen parallel execution in our model).
// These match up with events that are printed to the ordering trace.
enum SegmentType
{
    PIM_SEGMENT_ERROR, // 0
    PIMEMU_BEGIN, // 1
    PIMEMU_END, // 2
    PIMEMU_LAST, // 3
    PT_START, // 4
    PT_EXIT, // 5
    PT_LAUNCH, // 6
    PT_LAUNCH_PIM, // 7
    PT_JOIN, // 8
    OCL_LAUNCH_PIM, // 9
    OCL_LAUNCH, // 10
    OCL_EVT_WAIT // 11
};

// OpenCL enqueues are launched by a "fake" thread and have some kernel name
// that matches them to a SysTID & kernel name in the OpenCL performance trace
// OpenCL enqueues put work on a GPU, but they can't happen until a certain
// (optional) list of events is completed. All of these events are created by
// other OpenCL enqueues completing (which is what post_event holds -- the
// event number created when this GPU work completes.
struct ocl_event_t
{
    string kernel_name;
    set<unsigned long long> wait_events;
    unsigned long long post_event;
};

// Every segment in our ordering model holds a System Thread ID and a unique
// identifier that differentiates it from the other segments with that SysTID.
// Every segment has a 'Type', which says what kind of event stopped this
// thread. They also have a runtime, which is the post-scaling model.
//
// Every event that causes the segment to end has a few other pieces of data
// associated with it. For instance, if you stop because you launched another
// pthread, you store the child's pthread_id.
struct segment_info
{
    unsigned long tid;
    unsigned long long uid;
    SegmentType type;
    unsigned long long min_start_time_ns;
    unsigned long long runtime_ns; // Correct one at dynamic schedule time.
    unsigned long long ehp_runtime_ns;
    unsigned long long pim_runtime_ns;
    unsigned long pthread_id;
    unsigned long systid_to_wait;
    ocl_event_t ocl_evt;

    // Default sorting based on a combination of SysTID and UID.
    // SysTID first then UID on ties.
    bool operator<( const segment_info& val) const {
        if (tid != val.tid)
            return tid < val.tid;
        else
            return uid < val.uid;
    }
};

/* These are around just in case we want them in the future.
    bool operator==( const segment_info& val) const {
        return (tid == val.tid && uid == val.uid);
    }
    bool operator>( const segment_info &val) const {
        if (tid != val.tid)
            return tid > val.tid;
        else
            return uid > val.uid;
    }
    bool operator!=(const segment_info &val) const {
        return (tid != val.tid || uid != val.uid);
    }
    bool operator>=( const segment_info &val) const {
        if (tid != val.tid)
            return tid > val.tid;
        else
            return uid >= val.uid;
    }
    bool operator<=( const segment_info &val) const {
        if (tid != val.tid)
            return tid < val.tid;
        else
            return uid <= val.uid;
    }
*/

// Sometimes we will want to sort a segment information list based on the segments
// minimum start time.
struct compare_start_time {
    bool operator()(const segment_info& a, const segment_info& b)
    {
        return (a.min_start_time_ns < b.min_start_time_ns);
    }
};

/* Lists of tasks that are waiting to run or are ready to run.
 * These lists are "unordered" in the sense that they bear no relation to the
 * order that things will run at the end.
 * waiting_for_timestep: unordered list of segments that are waiting for some
 *      other segment to finish before they are "launched". This could be, for
 *      instance, a threa segments ends so the subsequent segment is now ready
 *      to run.
 * waiting_for_event: A list of the segments that could run at this time,
 *      except some other event must happen first. This exists because we
 *      don't want to have to search through all timestep segments whenever
 *      one of these events happens.
 * ehp_ready_list: List of segments that can run any time now on one of the
 *      EHP's CPUs.
 * pim_ready_list: List of lists. There is a ready list for each PIM, much
 *      like the above ehp_ready_list. However, there is only one EHP in
 *      the system. There are potentially many PIMs.
 */
set<segment_info> waiting_for_timestep;
set<segment_info> waiting_for_event;
set<segment_info> waiting_for_ocl;
multiset<segment_info, compare_start_time> ehp_ready_list;
multiset<segment_info, compare_start_time> ehp_gpu_ready_list;
vector<multiset<segment_info, compare_start_time> > pim_ready_list;
vector<multiset<segment_info, compare_start_time> > pim_gpu_ready_list;
set<segment_info> orphan_ocl_segments;

/* "cpu_time" is the next time that this particular device, whether an EHP CPU
 * or CPU on a PIM, can run a task. It is either set to the current global time
 * (meaning that it is ready to run right now) or some future time (meaning
 * that the device is busy doing work until some point in the future). */
vector<unsigned long long> ehp_cpu_time;
vector<unsigned long long> ehp_gpu_time;
vector<vector<unsigned long long> > pim_cpu_time;
vector<vector<unsigned long long> > pim_gpu_time;

struct output_trace {
    unsigned long thread_id;
    string kernel_name;
    unsigned long long start_time;
    unsigned long long end_time;
};
vector<queue<output_trace> > ehp_cpu_traces;
vector<queue<output_trace> > ehp_gpu_traces;
vector<vector<queue<output_trace> > > pims_cpus_traces;
vector<vector<queue<output_trace> > > pims_gpus_traces;

/* A map of threads that have finished to the global time they finished.
 * This way, we can know if it's OK to schedule a follow-on thread yet.
 * This is useful to know whether it's possible to schedule someone who is
 * waiting on a thread to end(e.g. pthread_join()) */
map<unsigned long, unsigned long long> finished_threads;
map<unsigned long long, unsigned long long> completed_ocl_events;

/* The minimum time that something is happening at in the system.
 * In other words, while we're building up the timeline, this is the time
 * that we're at "right now" when laying together everything. */
unsigned long long global_time;

/* Thread that does the PIMEMU_BEGIN work, so we know who to schedule
 * first. Has no other official parent. Immaculate thread conception. */
unsigned long starting_thread;

/* The final PIMEMU_END segment in the program. It does not have a follow-on
 * thread, so don't try to schedule anything that comes after it, unlike
 * all other PIMEMU_END segments, which are temporary ends. */
unsigned long long final_segment;

/* We need to keep a map from PThread ID to system Thread ID in order to
 * match up pthread launches with their child thread IDs.
 * The pthread_create() call only gets back a PTID, while the PT_START
 * that happens at some other time in the performance trace contains the
 * actual PTID->SysTID mapping.
 * This map is especially needed for PT_JOIN, which causes a parent thread
 * to wait on a particular PTID. Our ordering model needs to know
 * which SysTID this is, because it "schedules' based on SysTID. */
//Map of pthread_id:uid to the pthread_ids they create.
struct ptid_map
{
    unsigned long long uid; // parent UID
    unsigned long system_tid; // child system TID
    unsigned long pthread_id; // child PTID

    // Default sorting based on a combination of SysTID and UID.
    // SysTID first then UID on ties.
    bool operator<( const ptid_map& val) const {
        return uid < val.uid;
    }
};
map<unsigned long, set<ptid_map> > ptids_to_activate;
map<unsigned long, unsigned long> ptid_to_systid;

/* Certain threads are assigned to particular PIM numbers. (i.e. those threads
 * created with a pim_spawn(pthread) or pthread_create() from a thread already
 * on a PIM.
 * We therefore need to know which of these is sent to a particular PIM.
 * That's what systid_to_pim is used for.*/
map<unsigned long, unsigned long> systid_to_pim;
/* The map below is used because PT_LAUNCH_PIM and PT_LAUNCH are separate in
 * the incoming trace. This tells you at the PT_LAUNCH if you're actually a
 * PT_LAUNCH to a PIM. */
map<segment_info, unsigned long> launch_to_pim;
/* Tasks launched to a PIM from the EHP can have an overhead associatd with
 * being created across the fabric. Similarly, tasks ending have some other
 * delay. This gives us that mapping. */
map<unsigned long, unsigned long long> pim_launch_delay;
map<unsigned long, unsigned long long> pim_end_delay;

/* A map that tells us whether a particular system TID is actually an
 * OpenCL call in disguise. */
map<unsigned long, bool> systid_is_opencl;

/* Backwards map so that a child can know who their parent was.
 * This is useful for finding out if your parent was on a PIM, for instance. */
map<unsigned long, unsigned long> child_tid_to_parent_tid;

/* It's possible to call this function with no GPU performance file. We should
 * gracefully fail out without crashing if this happens.
 * Also, if we have a GPU segment but no GPU performance counter data, we
 * should also gracefully fail out. */
bool gpu_perf_exists;
bool have_seen_gpu_segment;

/* This is to help error checking. If we ever search for a UID higher than
 * this, we are doing something wrong. */
unsigned long long maximum_uid_found;

#endif  /* __PIMORDERINGMODEL_HPP_ */
