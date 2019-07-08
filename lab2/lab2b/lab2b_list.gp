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
#	8. wait for lock time (ns)
#
# output:
#	lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations.
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
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

# throughput of mutex and spin-lock protection on different number of threads
set title "List-1: Throughput vs Threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_1.png'

# grep out 1000 iterations, 1 list, sync, non-yield results
plot \
   	"< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin-lock' with linespoints lc rgb 'blue'


# wait-for-lock time and average time per operations vs threads
set title "List-2: Mutex Lock Performance"
set xlabel "Threads"
set logscale x 2
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'

# grep out 1000 iterations, 1 list, sync m, non-yield results
plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
    title 'wait-for-lock time' with linespoints lc rgb 'green', \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
    title 'completion time/operation' with linespoints lc rgb 'blue'


# wait-for-lock time and average time per operations vs threads
set title "List-3: Successful Runs for --lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'

# grep out 4 list, yield=id results
plot \
    "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    title 'None-protected' with points lc rgb 'red', \
    "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    title 'Mutex' with points lc rgb 'green', \
	"< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    title 'Spin-lock' with points lc rgb 'orange'


# throughput of mutex protection on different number of lists
set title "List-4: Throughput of Mutex for Different Number of Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_4.png'

# grep out 1000 iterations, sync=m, non-yield results
plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=1' with linespoints lc rgb 'red', \
	"< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=4' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=8' with linespoints lc rgb 'blue', \
	"< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=16' with linespoints lc rgb 'orange'


# throughput of spin lock protection on different number of lists
set title "List-5: Throughput of Spin-lock for Different Number of Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_5.png'

# grep out 1000 iterations, sync=s, non-yield results
plot \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=1' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=4' with linespoints lc rgb 'green', \
    "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=8' with linespoints lc rgb 'blue', \
    "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'lists=16' with linespoints lc rgb 'orange'
