/************************************************************************************************
	Program Description: This program is designed to demonstrate how resources are managed in 
	this simulated operating system. The deadlock avoidance strategy was implemented for this
	project by using bankers algorithm. 
	This is the main program and serve as the master process. It will start the process by 
	setting up time, and all shared memories as necessary and will begin fork() system call to
	create child processes for them to execute ./user executable using execvp() at random times.
	The randomness is simulated by simulated system clock which stays at shared memory for child
	to read from it.
	Resources will be simulated by having resources in shared memory and each resource will have
	resource descriptors.
	The resource descriptor has total amount of resources and resources allocator per process and
	accessable to child process.
	After resources initialized to starting point fork() will be executed at random time between
	1 and 500 milleseconds of simulated time.
	There will be only 18 process alive at any given time and new process is created only when
	one of the child process terminates.
	There will be no scheduling in this program but all processes will run concurrently and only
	be put in sleep if their request is not granted.
	Any updates in shared memory is done by oss.

	At the end it will produce statistics regarding how program ran and how many times deadlock
	avoidance ran, request granted. Verbose option is also given to turn on more explanatory
	text messages to display.

	In addtion, current allocated resource table will be printed during execution for every
	20 granted resources.

	Author: Bekzod Tolipov
	Date: 11/09/2019
************************************************************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <locale.h>
#include <sys/time.h>
#include <time.h>
#include <sys/queue.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>

#include "global_constants.h"
#include "helpers.h"
#include "shared_memory.h"
#include "message_queue.h"
#include "queue.h"
#include "resource.h"
#include "bankers.h"

//Prototypes
static int setuptimer(int s);
static int setupinterrupt();
static void myhandler(int s);
unsigned int random_time_elapsed();

void wait_for_all_children();
void cleanup_and_exit();
void fork_child(char** execv_arr, unsigned int pid);
struct Clock get_fork_time_new_proc(struct Clock system_clock);

unsigned int get_available_pid();
void unblock_process_if_possible(int resource, struct Message rsc_msg_box, struct Clock* blocked_time);
void print_blocked_queue();
void print_statistics(unsigned int num_requests);
float pct_requests_granted(unsigned int num_requests);

unsigned int num_resources_granted = 0, num_bankers_ran = 0;

// Globals used in signal handler
int clock_id, resource_table_id, resource_msg_id;
struct Clock* clock_point;
struct Clock* total_blocked_time;                                 
struct resource_table* resource_table;
int cleaning_up = 0;
int child_pids[MAX_PROCESS_COUNT + 1];
FILE* fptr;
struct Queue* blocked;
//static unsigned char bit_map[3];
pid_t parent_pid;
unsigned int num_requests = 0;
int process_counter;

struct message {
    int pid;
    char txt[10];
    int resource;
	int numb_of_rsc;
};

int main (int argc, char* argv[]) {

	char file_name[MAXCHAR] = "log.dat";
	int max_time = 5;
	int c;
	bool verbose = 0;
	
	parent_pid = getpid();
	// Read the arguments given in terminal
	while ((c = getopt (argc, argv, "hv")) != -1){
		switch (c)
		{
			case 'h':
				printf("To run the program you have following options:\n\n[ -h for help]\n[ -v verbose ]\nTo execute the file follow the code:\n./%s [ -h ] or any other options", argv[0]);
				return 0;
			case 'v':
				verbose = 1;
				break;
			default:
				fprintf(stderr, "ERROR: Wrong Input is Given!");
				abort();
		}
	}

	if(setuptimer(max_time) == -1){
        fprintf(stderr, "ERROR: Failed set up timer");
        fclose(fptr);
		return 1;
    }
	// System Interrupt set up
    if(setupinterrupt() == -1){
         fprintf(stderr, "ERROR: Failed to set up handler");
         fclose(fptr);
         return 1;
    }

    srand(time(NULL));

    unsigned int i, id = 0, num_messages = 0;
    blocked = malloc(sizeof(struct Queue) * NUMBER_OF_RESOURCES);          // Array of blocked queues (1 for each resource)
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        struct Queue blocked_queue;
        init_queue(&blocked_queue);
        blocked[i] = blocked_queue;
    }

    process_counter = 0;                  // Holds total number of child processes

    struct Clock fork_time = get_clock();    // Holds time to schedule new process
    
    // Shared logical Clock
	key_t key = ftok("./oss.c", 20);
    clock_id = get_shared_memory(key, sizeof(struct Clock));
    clock_point = (struct Clock*) attach_shared_memory(clock_id, 0);
    set_clock(clock_point);
    // Shared Resource Table 
	key = ftok("./oss.c", 21);
    resource_table_id = get_shared_memory(key, sizeof(struct resource_table));
    resource_table = (struct resource_table*) attach_shared_memory(resource_table_id, 0);
    allocate_resource_table(resource_table);
    // Shared resource message box for user processes to request/release resources 
	key = ftok("./oss.c", 22);
    resource_msg_id = get_message_queue(key);
    struct Message rsc_msg_box;

    // Holds all child_pids
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        child_pids[i] = 0;
    }

    // Open log file for writing
    fptr = fopen(file_name, "w");
	// Validate if file opened correctly
	if(fptr == NULL){
		fprintf(stderr, "ERROR: Failed to open the file, terminating program\n");
		return 1;
	}
	setvbuf(fptr, NULL, _IONBF, 0);

    // Get a time to fork first process at
    fork_time = get_fork_time_new_proc(*clock_point);
    
    // System clock now is forkable
    *clock_point = fork_time;

    // Declare more variables needed in main loop
    struct msqid_ds msgq_ds;	//	http://man7.org/linux/man-pages/man2/msgctl.2.html
    msgq_ds.msg_qnum = 0;
	bool bit_accessable;

    struct message msg;
    int resource = 0;
    bool is_resource_granted, is_resource_available;

    // Used for statisitcs
    //unsigned int num_requests = 0;
    struct Clock blocked_time[MAX_PROCESS_COUNT+1];
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        blocked_time[i].sec = 0;
        blocked_time[i].ns = 0;
    }
    total_blocked_time = malloc(sizeof(struct Clock));
    set_clock(total_blocked_time);
	print_rsc_summary(resource_table, fptr);
	//Main program loop
    while (1) {
        // Check if it is time to fork a new user process
        if (compare_clocks(*clock_point, fork_time) >= 0) {	//Compare clock will return (a>b:1), (a==b:0), (a<b:-1)
            //Is this spot in the bit map open?
			bit_accessable = false;
			//int count = 0;
			//while(1){
				//id = (id + 1) % MAX_PROCESS_COUNT;
				//uint32_t bit = bit_map[id / 8] & (1 << (id % 8));
				//if(bit == 0){
				//	bit_accessable = true;
				//	break;
				//}
			//	else{
				//	bit_accessable = false;
				//}

			//	if(count >= MAX_PROCESS_COUNT - 1){
			//		if (verbose) {
			//			fprintf(stderr, "OSS: bitmap is full\n");
			//			fprintf(fptr, "OSS: bitmap is full\n");
			//			fflush(fptr);
			//		}
			//		
				//	break;
			//	}
			//	count++;
			//}
			////////////////////////////////////////////////////
			int child_i;
			for (child_i = 1; child_i <= MAX_PROCESS_COUNT; child_i++) {
				if (child_pids[child_i] > 0) {
					continue;
				}
				id = child_i;
				bit_accessable = true;
				break;
			}
			if(bit_accessable) {

				if((child_pids[id] = fork()) == 0) {	//Child process
					int j;
					for(j=0;j<NUMBER_OF_RESOURCES;j++){
						resource_table->max_claims[id] = get_max_resource_claims(resource_table->rsc_descs[i].total-1);
					}
					//Execute ./child
					char clock_id_in_char[10];
					char rtbl_id[10];
					char r_msg_id[10];
					char p_id[5];
					
					sprintf(clock_id_in_char, "%d", clock_id);
					sprintf(rtbl_id, "%d", resource_table_id);
					sprintf(r_msg_id, "%d", resource_msg_id);
					sprintf(p_id, "%d", id);

					char *exec_arr[6];
					exec_arr[0] = "./user";
					exec_arr[1] = clock_id_in_char;
					exec_arr[2] = rtbl_id;
					exec_arr[3] = r_msg_id;
					exec_arr[4] = p_id;
					exec_arr[5] = NULL;
					
					execvp(exec_arr[0], exec_arr);
					perror("Child failed to execvp the command!");
					exit(1);
				} 
				else if(child_pids[id] < 0) {	//Failed fork()
					fprintf(stderr, "Fork problem!!!\n\n");
					perror("Child failed to fork!\n");
					cleanup_and_exit();
				}
				else{	//Parent process
					process_counter++;
					//bit_map[id / 8] |= (1 << (id % 8));	//Set bitmap spot to 1
					//Print it to screen and file
					if (verbose) {
						fprintf(stderr, "\nOSS: Generating process with PID %d at time %ld.%ld\n", id, clock_point->sec, clock_point->ns);
						fprintf(fptr, "\nOSS: Generating process with PID %d at time %ld.%ld\n", id, clock_point->sec, clock_point->ns);
						fflush(fptr);
					}
				}
			}
			
            fork_time = get_fork_time_new_proc(*clock_point);
        }

        // Get number of messages
        msgctl(resource_msg_id, IPC_STAT, &msgq_ds);
        num_messages = msgq_ds.msg_qnum;

        // Check for any messages
        if (num_messages > 0) {
            receive_msg(resource_msg_id, &rsc_msg_box, 0);

            // Received msg from user
			char ** recieved_msg_info = split_string(rsc_msg_box.mtext, ",");
			//Separating message into slots
			msg.pid = atoi(recieved_msg_info[0]);
			strcpy(msg.txt, recieved_msg_info[1]);
			msg.resource = atoi(recieved_msg_info[2]);
			msg.numb_of_rsc = atoi(recieved_msg_info[3]);
            resource = msg.resource;
            id = msg.pid;
            if (strcmp(msg.txt, "RQST") == 0) {
                // Process is requesting a resource
                if (verbose) {
					fprintf(stderr, "\nOSS: Process with PID %d requesting Resources: %d Requested amount: %d at time %ld.%ld\n", id, resource+1, msg.numb_of_rsc, clock_point->sec, clock_point->ns);
					fprintf(fptr, "\nOSS: Process with PID %d requesting Resources: %d Requested amount: %d at time %ld.%ld\n", id, resource+1, msg.numb_of_rsc, clock_point->sec, clock_point->ns);
					fflush(fptr);
                }

                num_requests++;

                is_resource_granted = 0;
                is_resource_available = resource_available(resource_table, resource, msg.numb_of_rsc);
                
                if (is_resource_available) {
					// Check with bankers algortihm if its safe to accept this allocation
                    is_resource_granted = bankers_algorithm(resource_table, id, resource, msg.numb_of_rsc);
                    incr_clock(clock_point, random_time_elapsed());
                    num_bankers_ran++;
                }
                
                if (is_resource_granted) {
                    // Resource granted
                    if (verbose) {
						fprintf(stderr, "\nOSS: Giving Resource to PID %d requesting Resources: %d at time %ld.%ld\n", id, resource+1, clock_point->sec, clock_point->ns);
						fprintf(fptr, "\nOSS: Giving Resource to PID %d requesting Resources: %d at time %ld.%ld\n", id, resource+1, clock_point->sec, clock_point->ns);
						fflush(fptr);
                    }

                    // Update program state
					resource_table->rsc_descs[resource].allocated[id]+=msg.numb_of_rsc;
                    num_resources_granted++;
                    
                    if (num_resources_granted % 20 == 0) {
                        // Print table of allocated resources
                        print_allocated_resource_table(resource_table, fptr);
						print_rsc_summary(resource_table, fptr);
                    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Send message back to user program to let it know that it's request was granted
                    send_msg(resource_msg_id, &rsc_msg_box, id+MAX_PROCESS_COUNT);
                }
                else {
                    // Resource was not granted
					fprintf(stderr, "\nOSS: Refusing resource to PID %d requesting Resources: %d Amount RQST: %d and blocking at time %ld.%ld\n", id, resource+1, msg.numb_of_rsc, clock_point->sec, clock_point->ns);
					fprintf(fptr, "\nOSS: Refusing resource to PID %d requesting Resources: %d and blocking at time %ld.%ld\n", id, resource+1, clock_point->sec, clock_point->ns);
					fflush(fptr);

                    // Add process to blocked queue
					resource_table->rsc_descs[resource].rqsted_before_blocked[id] = msg.numb_of_rsc;
                    enqueue(&blocked[resource], id);
                    
                    // Record the time the process was blocked
                    blocked_time[id] = *clock_point;
                }

            }
            else if (strcmp(msg.txt, "RLS") == 0) {
                // Process is releasing a resource
                if (verbose) {
					fprintf(stderr, "\nOSS: PID %d released Resource: %d Amount released: %d at time %ld.%ld\n", id, resource+1, msg.numb_of_rsc, clock_point->sec, clock_point->ns);
					fprintf(fptr, "\nOSS: PID %d released Resource: %d Amount released: %d at time %ld.%ld\n", id, resource+1, msg.numb_of_rsc, clock_point->sec, clock_point->ns);
					fflush(fptr);
                }

                // Update program state
				resource_table->rsc_descs[resource].allocated[id] -= msg.numb_of_rsc;

                // Send message back to user program to let it know that we updated the resource table
                send_msg(resource_msg_id, &rsc_msg_box, id+MAX_PROCESS_COUNT);

                // Check if we can unblock any processes
                //unblock_process_if_possible(resource, rsc_msg_box, blocked_time);
            }
            else {
                // Process terminated
                if (verbose) {
					fprintf(stderr, "\nOSS: PID %d terminated at time %ld.%ld\n", id, clock_point->sec, clock_point->ns);
					fprintf(fptr, "\nOSS: PID %d terminated at time %ld.%ld\n", id, clock_point->sec, clock_point->ns);
					fflush(fptr);
                }
                // Update program state
                child_pids[id] = 0;
                release_resources(resource_table, id); // Updates resource table
                
                for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
                    // Check if we can unblock any processes
                    unblock_process_if_possible(i, rsc_msg_box, blocked_time);   
                }

            }

            // Increment Clock slightly whenever a resource is granted or released
            incr_clock(clock_point, random_time_elapsed());
        }
		else{
			for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
				if(!empty(&blocked[i])){
                    unblock_process_if_possible(i, rsc_msg_box, blocked_time);
				}
            }

		}

        incr_clock(clock_point, random_time_elapsed());

        // Calculate total elapsed real-time sec
		int stat;
		pid_t remove_pid = waitpid(-1, &stat, WNOHANG);	// Non block wait for parent
		// If somebody died then barry them underground
		// and remove them from history
		if(remove_pid > 0){
			int pos;
			for(pos=1; pos<=MAX_PROCESS_COUNT;pos++){
				if(child_pids[pos] == remove_pid){
					child_pids[pos] = 0;
				//	if(!empty(&blocked[i])){
				//		unblock_process_if_possible(i, rsc_msg_box, blocked_time);
				//	}
					break;
					//bit_map[pcb[pos].id / 8] &= ~(1 << (pcb[pos].id % 8));
				}
			}
		}
    }

    // Print information before exiting
	fprintf(stderr, "\nOSS: Finished process at time %ld.%ld\n", clock_point->sec, clock_point->ns);
	fprintf(fptr, "\nOSS: Finished process at time %ld.%ld\n", clock_point->sec, clock_point->ns);
	fflush(fptr);

    //print_allocated_resource_table(resource_table, fptr);
    print_rsc_summary(resource_table, fptr);
    
    print_blocked_queue();

    print_statistics(num_requests);

    cleanup_and_exit();

    return 0;
}

void wait_for_all_children() {
    pid_t pid;
    
    while ((pid = wait(NULL))) {
        if (pid < 0) {
            if (errno == ECHILD) {
                break;
            }
        }
    }
}
/***************************************
	Terminate all existing processes
***************************************/
void terminate_children() {
    printf("OSS: SIGTERM signal sent\n");
    fprintf(fptr, "OSS: SIGTERM signal sent\n");
    int i;
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        if (child_pids[i] == 0) {
            continue;
        }
        if (kill(child_pids[i], SIGTERM) < 0) {
            if (errno != ESRCH) {
                // Child process exists and kill failed
                perror("kill");
            }
        }
    }
}
/**************************************
	Clean up everything and terminate
**************************************/
void cleanup_and_exit() {
	print_allocated_resource_table(resource_table, fptr);
    print_rsc_summary(resource_table, fptr);
    print_blocked_queue();
    print_statistics(num_requests);

    terminate_children();
    printf("OSS: Removing message queues and shared memory\n");
    fprintf(fptr, "OSS: Removing message queues and shared memory\n");
    remove_message_queue(resource_msg_id);
    wait_for_all_children();
    cleanup_shared_memory(clock_id, clock_point);
    cleanup_shared_memory(resource_table_id, resource_table);
    free(blocked);
    free(total_blocked_time);
    fclose(fptr);
    exit(0);
}
/******************************************
	Used to set up time for next process
******************************************/
struct Clock get_fork_time_new_proc(struct Clock system_clock) {
	
    unsigned int time_before_next_process = rand() % FIVE_HUNDRED_MS; 
    incr_clock(&system_clock, time_before_next_process);	//Add random need to elapse time to current time and set to forkable time
    return system_clock;
	
}
/**************************************
	Random elapsed time for simulated
	clock
**************************************/
unsigned int random_time_elapsed() {
    return (rand() % 500000) + 100000; 
}
/****************************************
	Try to unblock process if possible
****************************************/
void unblock_process_if_possible(int resource, struct Message rsc_msg_box, struct Clock* blocked_time) {
    if (empty(&blocked[resource])) {
        // There are no processes blocked on this resource
        return;
    }
	struct Queue temp_queue = blocked[resource];
	int pid;
	struct Queue new_queue;
	bool is_resource_granted = false;
	init_queue(&new_queue);
	while((pid = peek(&temp_queue)) != -1){
	//	char buffer [100];
		is_resource_granted = false;
		// Resource is available so run bankers algorithm to check if we can
		// safely grant this request
		if(resource_available(resource_table, resource, resource_table->rsc_descs[resource].rqsted_before_blocked[pid])){
			is_resource_granted = bankers_algorithm(resource_table, pid, resource, resource_table->rsc_descs[resource].rqsted_before_blocked[pid]);
		}
		else{
			while((peek(&blocked[resource])) != -1){
                    enqueue(&new_queue, peek(&blocked[resource]));
                    dequeue(&blocked[resource]);
            }
			int index;
			while((index = peek(&new_queue)) != -1){
				enqueue(&blocked[resource], index);
				dequeue(&new_queue);
			}
			dequeue(&temp_queue);
			continue;
		}
		incr_clock(clock_point, random_time_elapsed());
		num_bankers_ran++;

		if (is_resource_granted) {
			// Resource granted
			fprintf(stderr, "OSS: Unblocking P%d and granting it R%d Amount RQST: (%d) at time %ld:%'ld\n",
				pid, resource+1, resource_table->rsc_descs[resource].rqsted_before_blocked[pid], clock_point->sec, clock_point->ns);
			//fprintf(stderr, buffer);

			// Update program state
			resource_table->rsc_descs[resource].allocated[pid]+= resource_table->rsc_descs[resource].rqsted_before_blocked[pid];
			num_resources_granted++;
			while((peek(&blocked[resource])) != -1){
				if(peek(&blocked[resource]) != pid){
					enqueue(&new_queue, peek(&blocked[resource]));
					dequeue(&blocked[resource]);
				}
				else{
					dequeue(&blocked[resource]);
				}
			}

			// Add wait time to total time blocked
			struct Clock wait_time = subtract_Clocks(*clock_point, blocked_time[pid]);
			*total_blocked_time = add_clocks(*total_blocked_time, wait_time);
        
			if (num_resources_granted % 20 == 0) {
				// Print table of allocated resources
				print_allocated_resource_table(resource_table, fptr);
			}

			// Send message back to user program to let it know that it's request was granted
			send_msg(resource_msg_id, &rsc_msg_box, pid+MAX_PROCESS_COUNT);
		}
		
		blocked[resource] = new_queue;
		while(dequeue(&new_queue) != -1);
		dequeue(&temp_queue);
	}
}
/*********************************
	Print all blocked processes
	in a queue
*********************************/
void print_blocked_queue() {
    int i;
    bool queue_is_empty = 1;
    char buffer[2000];
    char* queue;
    
    sprintf(buffer, "<<Blocked Processes>>\n");
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (empty(&blocked[i])) {
            // Queue empty
            continue;
        }
        
        // Resource label
        sprintf(buffer + strlen(buffer), "  R%2d:", i+1);
        
        // What processes are blocked on resource i
        queue = get_queue_string(&blocked[i]);
        sprintf(buffer + strlen(buffer), "%s", queue);
        free(queue);
        
        // Queue is not empty
        queue_is_empty = 0;
    }
    
    if (queue_is_empty) {
        sprintf(buffer + strlen(buffer), "  < no blocked processes >\n");
    }
    
    sprintf(buffer + strlen(buffer), "\n");

	fprintf(stderr, buffer);
}
/*****************************************
	Print statistics for program run.
*****************************************/
void print_statistics(unsigned int num_requests) {
    char buffer[2000];

    sprintf(buffer, "<<<Statistics>>>\n");
    sprintf(buffer + strlen(buffer), "  %-22s: %'d\n", "Total Processes", process_counter);
    sprintf(buffer + strlen(buffer), "  %-22s: %'d\n", "Total Granted Requests", num_resources_granted);
    sprintf(buffer + strlen(buffer), "  %-22s: %'d\n", "Total Requests", num_requests);
    sprintf(buffer + strlen(buffer), "  %-22s: %'ld:%'ld\n", "Total Wait Time", total_blocked_time->sec, total_blocked_time->ns);
    sprintf(buffer + strlen(buffer), "  %-22s: %'d\n", "Total Bankers Ran", num_bankers_ran);

    sprintf(buffer + strlen(buffer), "\n");
    
    fprintf(stderr, buffer);
}


/*************** 
* Set up timer *
***************/
static int setuptimer(int time){
	
    struct itimerval value;
    value.it_value.tv_sec = time;
    value.it_value.tv_usec = 0;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    return(setitimer(ITIMER_REAL, &value, NULL));
	
}
 
/*******************
* Set up interrupt *
*******************/
static int setupinterrupt(){
	
    struct sigaction act;
    act.sa_handler = &myhandler;
    act.sa_flags = SA_RESTART;
    return(sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
	
}

/************************
* Set up my own handler *
************************/
static void myhandler(int s){
	
	fprintf(stderr, "\n!!!Termination begin since timer reached its time!!!\n");
	cleanup_and_exit();

}

