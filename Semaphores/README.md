# Semaphores

This program will demonstrate simple interprocess communication by using system calls(fork, exec, shared memory, and semaphores)

oss main executable use command line arguments and following command line arguments are availabe using getopt:

	-h		// Help statement
	-s x		// Maximum number of processes between 20 and 1
	-l filename	// File name
	-t z		// Timer interrupt


Program will start the operating system simulator (oss) as one main process which will start by forking a number of processes 
and will then fork new children as some terminate. Program makes sure that it never exceeds a specified number of processes 
in system which is why it forks new processes as existing ones terminate.

oss will start by first allocating shared memory for a clock that gets incremented only by itself. The child processes 
should be able to view this memory but will not increment it. This shared memory clock should be two integers, with one 
integer to hold seconds and the other integer to hold nanoseconds. So if one integer has 5 and the other has 10000 then 
that would mean that the clock is showing 5 seconds and 10000 nanoseconds. This clock should initially be set to 0.

In addition to this shared memory clock, there is an additional area of shared memory allocated to allow the child
processes to send information to the parent (oss). Let us call this area shmMsg.

Program will output data after each child termination into default file (log.dat) or to specified file by following format:

	"Master: Child pid is terminating at my time xx.yy because it reached mm.nn in child process"

There are 3 different possiblities for oss to terminate:
	
	I) By system timer interrupt
	II) When it reaches spawning total of 100 processes
	III) When it reaches system semulated time 2 seconds

For each case there will be print statement at the end of the program.
