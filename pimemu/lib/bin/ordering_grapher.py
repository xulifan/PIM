#!/usr/bin/python
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

# Program to graphically output the output of the PIM performance simulator
# using the Python matplotlib drawing functions.
# Quick and dirty, will break down if you have a tremendous number of segments
# in your program.

import matplotlib.pyplot as plt
import re
import sys

if ((len(sys.argv)) < 2 or (len(sys.argv) > 3)):
    print 'Please give either one or two arguments'
    print '\t1st argument: the input file, which is the output ',
    print 'of the ordering tool.'
    print '\t2nd argument [optional]: desired PDF output file for the',
    print 'graph generated by this program.'
    quit(-1)
try:
    f = open(sys.argv[1], 'r')
except IOError:
    print 'Cannot open input file', sys.argv[1]
    quit(-1)

# Probably want to add some "if this file exists, verify you want to do that"
# checks in here at some point
if (len(sys.argv) == 3):
    print_me = 1
    try:
        out_f = open(sys.argv[2], 'w')
    except IOError:
        print 'Cannot open output file', sys.argv[2]
        quit(-1)
else:
    print_me = 0

# Variable declarations
max_val = 0     # Maximum time used in the timeline.
cur_y = 0       # Temporary y-value for the chart.
                # Incremented for each new device
cur_name = ''   # Name of the device we're currently parsing
cur_times = []  # Segment runtimes for the deivce we're parsing
labels = []     # All of the devices we've seen throughout the program
ticks = []      # Y-Axis location of each device we've seen

fig = plt.figure()
ax = fig.add_subplot(111)

skip_me = 0

for line in f:
    # Newlines are blanks between each device. If you see one of these,
    # It tells you that we need to print out info about the last device
    # into the graphics system.
    if (line == '\n'):
        # Add these two lines back in if you want our output to instead
        # contain only the devices with segments assigned to them
        #if skip_me:
            #continue
        print_color = 'green' # Default
        # Change color depending on what the device is
        found_ehp = re.match('ehp-', cur_name)
        found_cpu = re.match('.+-cpu', cur_name)
        if found_ehp is not None:
            if found_cpu is not None:
                # Found an EHP CPU
                print_color = 'green'
            else:
                # Found an EHP GPU
                print_color = 'red'
        else:
            if found_cpu is not None:
                # Found a PIM CPU
                print_color = 'navy'
            else:
                # Found a PIM GPU
                print_color = 'darkorange'
        ax.broken_barh(cur_times, ((cur_y + 1), 6), facecolor=print_color, edgecolors='none')
        cur_times = [] # Clear the list so the next device can safely append
        labels.append(cur_name)
        ticks.append(cur_y + 4) # +2 puts is in the middle of the line
        cur_y += 8
        continue

    # "Total runtime: {runtime}" is the last line in our CSV output file
    if (line.rsplit()[0] == 'Total'):
        ax.set_ylim(0, cur_y)
        if (int(line.rsplit()[2]) != max_val):
            print 'Problem detected when parsing output file. '
            print 'The maximum calculated runtimeis ', max_val, 'ns'
            print 'However, the CSV file itself says that the maximum is',
            print int(line.rsplit()[2]), 'ns.'
            quit()
        ax.set_xlim(0, max_val)
        ax.set_xlabel('nanoseconds')
        ax.set_yticks(ticks)
        ax.set_yticklabels(labels)
        ax.grid(True)
        if print_me:
            plt.savefig(out_f, format='pdf')
        else:
            plt.show()
        break

    # Everything else in the file is a straight up CSV. Time to get the values
    linevals = line.rsplit(', ')

    # First line in the CSV is a header. Ignore it.
    if (linevals[0] == 'DeviceID'):
        continue

    # If your segmenent ID says "NA", there were no segments ever assigned to
    # this device. Set its name and just continue on to the following newline
    if (linevals[1] == 'NA'):
        cur_name = linevals[0]
        skip_me = 1
        continue

    skip_me = 0
    # First entry is the device's name
    cur_name = linevals[0]
    # Second entry is the segment ID number. We ignore it.
    # Third entry is the start time of this segment in ns
    start_time = int(linevals[3])
    # 4th entry is the end time. Print library cares about length of box tho
    len = int(linevals[4]) - start_time
    cur_times.append((start_time, len))
    if (max_val < (start_time + len)):
        max_val = start_time + len

f.close()
if print_me:
    out_f.close()
