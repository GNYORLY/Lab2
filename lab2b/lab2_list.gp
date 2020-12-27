#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#   8. mutex wait time (ns)
#
# output:
#	lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations
#	lab2b_3.png ... successful iterations vs. threads for each synchronization method
#	lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists
#   lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","


set title "List-1: Throughput vs. Thread count for mutex and spin-lock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_1.png'
plot \
    "< grep 'list-none-m,[0-9]+,1000,\" lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]+,1000,\" lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'spin-lock' with linespoints lc rgb 'green'

set title "List-2: Mean mutex wait time & operation time vs Thread count"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
set key left top
# note that unsuccessful runs should have produced no output
plot \
     "< grep list-none-m,2?[0-9],1000,1,|list-none-m,16,1000,1,\" lab2_list.csv" using ($2):($8) \
	title 'mutex wait time' with points lc rgb 'green', \
     "< grep list-none-m,2?[0-9],1000,1,|list-none-m,16,1000,1,\" lab2_list.csv" using ($2):($7) \
	title 'op time' with points lc rgb 'red', \


set title "List-3: Successful iterations vs. Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful iterations"
set logscale y 10
set output 'lab2b_3.png'
set key left top
plot \
    "< grep 'list-id-none,[0-9]+,[0-9]+,4,\" lab2_list.csv" using (2):($3) \
	with points lc rgb "blue" title "unprotected", \
    "< grep 'list-id-m,[0-9]+,[0-9]+,4,\" lab2_list.csv" using (2):($3) \
	with points lc rgb "red" title "mutex", \
    "< grep 'list-id-s,[0-9]+,[0-9]+,4,\" lab2_list.csv" using (2):($3) \
    with points lc rgb "green" title "spin_lock", \


set title "List-4: Throughput vs. Mutex threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_4.png'
plot \
     "< grep -e 'list-none-m,[0-9],1000,1,|list-none-m,12,1000,1,\" lab2_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'green', \
    "< grep -e 'list-none-m,[0-9],1000,4,|list-none-m,12,1000,4,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'blue', \
    "< grep -e 'list-none-m,[0-9],1000,8,|list-none-m,12,1000,8,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-m,[0-9],1000,16,|list-none-m,12,1000,16,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'pink', \

set title "List-4: Throughput vs. Spin-lock threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_5.png'
plot \
    "< grep -e 'list-none-s,[0-9],1000,1,|list-none-m,12,1000,1,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'pink', \
    "< grep -e 'list-none-s,[0-9],1000,4,|list-none-m,12,1000,4,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'purple', \
    "< grep -e 'list-none-s,[0-9],1000,8,|list-none-m,12,1000,8,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'green', \
    "< grep -e 'list-none-s,[0-9],1000,16,|list-none-m,12,1000,16,\" lab2_list.csv" using ($2):(1000000000/($7)) \
    title '1 list' with linespoints lc rgb 'orange', \

