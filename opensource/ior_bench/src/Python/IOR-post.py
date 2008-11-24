#!/usr/bin/env python
#
#/*****************************************************************************\
#*                                                                             *
#*       Copyright (c) 2003, The Regents of the University of California       *
#*     See the file COPYRIGHT for a complete copyright notice and license.     *
#*                                                                             *
#\*****************************************************************************/
#
# CVS info:
#   $RCSfile: IOR-post.py,v $
#   $Revision: 1.3 $
#   $Date: 2003/12/03 00:09:38 $
#   $Author: loewe $

import getopt, re, sys

test_desc = re.compile(r"""
    \s*
    (\d+:\s*)?      # Possible poe/psub prefix
    Test\s+         (?P<test>          \d+)   \s*:\s*
    Block\s*=\s*    (?P<block_size>    \d+)   \s*,\s*
    Transfer\s*=\s* (?P<transfer_size> \d+)   \s*,\s*
    Stride\s*=\s*   (?P<stride>        \d+)
    """, re.VERBOSE)
time_stamp = re.compile(r"""
    \s*
    (\d+:\s*)?    # Possible poe/psub prefix
    Test\s+       (?P<test> \d+)          \s*:\s*
    Iter\s*=\s*   (?P<iter> \d+)          \s*,\s*
    Task\s*=\s*   (?P<task> \d+)          \s*,\s*
    Time\s*=\s*   (?P<time> \d+(\.\d+)?)  \s*,\s*
                  (?P<op>   [ \w]+)
    """, re.VERBOSE)
stats = [{}]   # a list of dictionaries

def main():
    flags = 0
    # Parse the command line options
    try:
        optpairs, leftover = getopt.getopt(sys.argv[1:], "eghs",
                                           ["excel", "gnuplot",
                                            "help", "summary"])
    except getopt.GetoptError:
        sys.exit(2)
    for opt, val in optpairs:
        if opt in ("-h", "--help"):
            print_help()
            sys.exit(0)
        elif opt in ("-e", "--excel"):
            flags = flags | (1<<1)
        elif opt in ("-g", "--gnuplot"):
            flags = flags | (1<<2)
        elif opt in ("-s", "--summary"):
            flags = flags | (1<<0)
    if not optpairs: flags = flags | (1<<0)

    if leftover and (leftover[0] != '-'):
        tracefile = open(leftover[0], 'r')
    else:
        tracefile = sys.stdin
    parse_trace_file(tracefile)

    if flags & (1<<0): print_summary()
    if flags & (1<<1): print_excel()
    if flags & (1<<2): print_gnuplot()

def parse_trace_file(tracefile):
    global test_desc
    global time_stamp
    global stats
    stats = [{}]
    ops = ['write open', 'write', 'write close',
           'read open', 'read', 'read close']

    for line in tracefile.readlines():
        m = time_stamp.match(line)
        if m:
            add_time_stamp(int(m.group('test')), int(m.group('iter')),
                           int(m.group('task')), float(m.group('time')),
                           m.group('op'))
            continue
        m = test_desc.match(line)
        if m:
            add_test_desc(int(m.group('test')),
                          int(m.group('block_size')),
                          int(m.group('transfer_size')),
                          int(m.group('stride')))
            continue

    for test in range(len(stats)):
        i_list = stats[test]['iter']
        num_iter = len(i_list)
        # Initialize the averages
        for op in ops:
            stats[test][op + ' avg'] = 0
        for iter in range(num_iter):
            t_list = i_list[iter]['task']
            for op in ops:
                # Elasped times for every operation
                i_list[iter][op] = (max_value(t_list, op + ' stop')-
                                    min_value(t_list, op + ' start'))
                # Keep a running total of times from each iteration
                # so we can calculate the average later
                stats[test][op + ' avg'] = stats[test][op + ' avg'] + i_list[iter][op]
            # Calculate the elapsed times for every operation
            op_map = i_list[iter]
            op_map['write total'] = (op_map['write open'] +
                                     op_map['write'] +
                                     op_map['write close'])
            op_map['read total'] = (op_map['read open'] +
                                    op_map['read'] +
                                    op_map['read close'])
            # Calculate the read and write bandwidth(BW)
            filesize = float(stats[test]['block_size']
                             * len(t_list) # number of tasks
                             * (stats[test]['stride']+1))
            op_map['write BW'] = filesize / op_map['write total']
            op_map['read BW'] = filesize / op_map['read total']
        # Calculate the average time for all ops
        for op in ops:
            stats[test][op + ' avg'] = stats[test][op + ' avg'] / num_iter
        # Find the max bandwidth(BW)
        stats[test]['max write BW'] = max_value(stats[test]['iter'], 'write BW')
        stats[test]['max read BW'] = max_value(stats[test]['iter'], 'read BW')

def add_time_stamp(test, iter, task, time, op):
    global stats

    stats = expand_list(stats, test, {})

    if not stats[test].has_key('iter'):
        stats[test]['iter'] = [{}]
    iter_list = stats[test]['iter']
    iter_list = expand_list(iter_list, iter, {})
    stats[test]['iter'] = iter_list

    if not stats[test]['iter'][iter].has_key('task'):
        stats[test]['iter'][iter]['task'] = [{}]
    task_list = stats[test]['iter'][iter]['task']
    task_list = expand_list(task_list, task, {})
    stats[test]['iter'][iter]['task'] = task_list

    stats[test]['iter'][iter]['task'][task][op] = time

def add_test_desc(test, block_size, transfer_size, stride):
    global stats

    stats = expand_list(stats, test, {})
    stats[test]['block_size'] = block_size
    stats[test]['transfer_size'] = transfer_size
    stats[test]['stride'] = stride

def print_summary():
    global stats

    print "           Blocksize   Transfer |                     Write                   |                     Read"
    print "Test Iter    (bytes)    (bytes) |    Open(s)   Write(s)   Close(s)   BW(MB/s) |    Open(s)    Read(s)   Close(s)   BW(MB/s)"
    for test in range(len(stats)):
        for iter in range(len(stats[test]['iter'])):
            op_map = stats[test]['iter'][iter]
            print pad(4, `test`),
            print pad(4, `iter`),
            print pad(10, `stats[test]['block_size']`),
            print pad(10, `stats[test]['transfer_size']`),
            print " ",
            print pad(10, `round_time(op_map['write open'])`),
            print pad(10, `round_time(op_map['write'])`),
            print pad(10, `round_time(op_map['write close'])`),
            print pad(10, `round((op_map['write BW'] / 1048576.0), 2)`),
            print " ",
            print pad(10, `round_time(op_map['read open'])`),
            print pad(10, `round_time(op_map['read'])`),
            print pad(10, `round_time(op_map['read close'])`),
            print pad(10, `round((op_map['read BW'] / 1048576.0), 2)`)

def print_gnuplot():
    global stats

    #print " Blocksize   Transfer |                     Write                   |                     Read"
    #print "   (bytes)    (bytes) |    Open(s)   Write(s)   Close(s)   BW(MB/s) |    Open(s)    Read(s)   Close(s)   BW(MB/s)"
    previous = stats[0]['block_size']
    for test in range(len(stats)):
        if stats[test]['block_size'] != previous:
            print
            previous = stats[test]['block_size']
        print pad(10, `stats[test]['block_size']`),
        print pad(10, `stats[test]['transfer_size']`),
        print " ",
        print pad(10, `round_time(stats[test]['write open avg'])`),
        print pad(10, `round_time(stats[test]['write avg'])`),
        print pad(10, `round_time(stats[test]['write close avg'])`),
        print pad(10, `round((stats[test]['max write BW'] / 1048576.0), 2)`),
        print " ",
        print pad(10, `round_time(stats[test]['read open avg'])`),
        print pad(10, `round_time(stats[test]['read avg'])`),
        print pad(10, `round_time(stats[test]['read close avg'])`),
        print pad(10, `round((stats[test]['max read BW'] / 1048576.0), 2)`)

# Prints Excel-friendly ascii tables
# The values in the tables are currently the AVERAGEs of values over
#   all iterations, except for the bandwidths(BW) which are maximums
def print_excel():
    global stats

    ops = ['write open', 'write', 'write close', 'max write BW',
           'read open', 'read', 'read close', 'max read BW']

    for op in ops:
        # Print the op name as the name of the table
        print '"' + op + '"'

        # Print out the headings for the columns
        trans = []
        for test in range(len(stats)):
            if stats[test]['transfer_size'] not in trans:
                trans.append(stats[test]['transfer_size'])
        print ' ' * 12,
        for size in trans:
            print pad(12, '"'+`size`+'"'),
        print

        # Print the row names and values
        previous_block_size = stats[0]['block_size']
        print pad(12, '"'+`stats[0]['block_size']`+'"'),
        for test in range(len(stats)):
            if stats[test]['block_size'] != previous_block_size:
                previous_block_size = stats[test]['block_size']

                # Start the next row of the table...
                print
                # ...with the block size
                print pad(12, '"'+`stats[test]['block_size']`+'"'),
            if op == 'max write BW' or op == 'max read BW':
                print pad(12, `round(stats[test][op]/1048576,2)`),
            else:
                print pad(12, `round_time(stats[test][op + ' avg'])`),
        print
        print

def round_time(time):
    if time < 1:
        return round(time, 6)
    elif time < 100:
        return round(time, 3)
    elif time < 3600:
        return round(time, 2)
    else:
        return round(time, 0)

# Find the the maximum value for the given "key" in a "list" of mappings
def max_value(list, key):
    max = list[0][key]
    for task in list:
        if task[key] > max:
            max = task[key]
    return max

# Find the the mininum value for the given "key" in a "list" of mappings
def min_value(list, key):
    min = list[0][key]
    for task in list:
        if task[key] < min:
            min = task[key]
    return min

# If "list" has length shorter than "length", then it is concatenated
# with enough elements of the "initializer" to make up the difference
def expand_list(list, length, initializer):
    if len(list)-1 < length:
        return list + ([initializer] * (length - (len(list)-1)))
    return list

# If the string "str" is shorter than "width", pad the left side with spaces
def pad(width, str):
    if len(str) < width:
        return (" " * (width - len(str))) + str
    return str

def print_help():
    print "IOR-post.py: Post processor for IOR trace output"
    print "              $Revision: 1.3 $"
    print
    print "Usage: IOR-post.py [-h] [-s] [-e] [-g] [tracefile]"
    print
    print "   -h, --help      Print this help message and exit"
    print "   -s, --summary   Print a summary table of the trace file output (default)"
    print "   -e, --excel     Print excel-ready ascii tables"
    print "   -g, --gnuplot   Print a table in gnuplot form"
    print "   tracefile       File from which an IOR trace should be read."
    print "                   If \"tracefile\" is \"-\", or if no file is specified,"
    print "                   IOR-post will read from stdin."
    
if __name__ == "__main__":
    main()
