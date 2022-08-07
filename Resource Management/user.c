/*********************************************************************************************
	Program Description: This program will be executed by child processes created in oss and
	they will ask for resource at random times. It will start by generating random time to 
	ask oss for resource or release the resource or terminate. Request/release will be tran-
	sfered by message queue. Programm will continue working untill terminate chance reached.
	The request claims cannot go above max_claims in resource table which is in shared mem-
	ory. It will only ask if request<max_claims. Requests are generated by random number
	which is not allowed to exceed from max_claims. At random time [0, 250] ms program will
	first check if it will terminate or not. It can also ask if its allowed to release the
	resources in it poseccion.

*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/time.h>
#include <locale.h>
#include <signal.h>

#include "global_constants.h"
#include "helpers.h"
#include "message_queue.h"
#include "shared_memory.h"
#include "clock.h"
#include "resource.h"

bool will_terminate();
unsigned int get_random_pct();
void add_signal_handlers();
void handle_sigterm(int sig);
struct Clock get_time_to_request_release_rsc(struct Clock sysclock);
unsigned int get_nss_to_request_release();
void create_msg_that_contains_rsc(char* mtext, int pid, struct resource_table* resource_table);
unsigned int get_random_resource();
bool will_release_resource();
unsigned int get_resource_to_release(int pid, struct resource_table* resource_table);
void request_a_resource(int rsc_msg_box_id, int pid, struct resource_table* resource_table);
void release_a_resource(int rsc_msg_box_id, int pid, struct resource_table* resource_table);
void send_termination_notification(int rsc_msg_box_id, int pid);

const unsigned int CHANCE_TERMINATE = 25;
const unsigned int CHANCE_RELEASE = 70;
unsigned int MAX_CLAIMS_PER_RSC[NUMBER_OF_RESOURCES];

int main (int argc, char *argv[]) {
    add_signal_handlers();
    srand(time(NULL) ^ getpid());

    // Get shared memory IDs
    int sysclock_id = atoi(argv[1]);
    int resource_table_id = atoi(argv[2]);
    int rsc_msg_box_id = atoi(argv[3]);
    int pid = atoi(argv[4]);


    // Attach to shared memory
    struct Clock* sysclock = attach_shared_memory(sysclock_id, 1);
    struct resource_table* resource_table = attach_shared_memory(resource_table_id, 0);
    struct Clock time_to_request_release  = get_time_to_request_release_rsc(*sysclock);
    while(1) {
        if (compare_clocks(*sysclock, time_to_request_release) < 0) {
            continue;
        }
        // Time to REQUEST/release a resource 
        if (!has_resource(pid, resource_table)) {
            request_a_resource(rsc_msg_box_id, pid, resource_table);
            if (will_terminate()) {
                send_termination_notification(rsc_msg_box_id, pid);
                break;
            }
        }
        else {
            // Determine if we are going to REQUEST or release a resource
            if (will_release_resource()) {
                release_a_resource(rsc_msg_box_id, pid, resource_table);
            }
            else {
                request_a_resource(rsc_msg_box_id, pid, resource_table);
                if (will_terminate()) {
                    send_termination_notification(rsc_msg_box_id, pid);
                    break;
                }
            }
        }
        // Get new time to REQUESTuest/release a resouce
        time_to_request_release = get_time_to_request_release_rsc(*sysclock);
    } 
	//fprintf(stderr, "USER: Did not go to while loop\n");
    return 0;  
}

void send_termination_notification(int rsc_msg_box_id, int pid) {
    struct Message rsc_msg_box;
    sprintf(rsc_msg_box.mtext, "%d,TERM,0,0", pid);
    send_msg(rsc_msg_box_id, &rsc_msg_box, pid); 
}

void release_a_resource(int rsc_msg_box_id, int pid, struct resource_table* resource_table) {
    struct Message rsc_msg_box;
    unsigned int resource_to_release = get_resource_to_release(pid, resource_table);
    sprintf(rsc_msg_box.mtext, "%d,RLS,%d,%d", pid, resource_to_release, resource_table->rsc_descs[resource_to_release].allocated[pid]);
    send_msg(rsc_msg_box_id, &rsc_msg_box, pid);
    // Blocking receive: wait until OSS updates the program state 
    // so that we do not release the same resource many times
    receive_msg(rsc_msg_box_id, &rsc_msg_box, pid+MAX_PROCESS_COUNT);
    return;
}

void request_a_resource(int rsc_msg_box_id, int pid, struct resource_table* resource_table) {
    struct Message rsc_msg_box;
    create_msg_that_contains_rsc(rsc_msg_box.mtext, pid, resource_table);
    send_msg(rsc_msg_box_id, &rsc_msg_box, pid);
    // Blocking receive - wait until granted a resource
    receive_msg(rsc_msg_box_id, &rsc_msg_box, pid+MAX_PROCESS_COUNT);
    // Granted a resource
    return;
}

unsigned int get_resource_to_release(int pid, struct resource_table* resource_table) {
    unsigned int* allocated_resources = get_current_alloced_rscs(pid, resource_table);
    unsigned int size = get_number_of_allocated_rsc_classes(pid, resource_table);
    unsigned int random_idx = rand() % size;
    unsigned int resource_to_release = allocated_resources[random_idx];
    free(allocated_resources);
    return resource_to_release;
}

bool will_release_resource() {
    return event_occured(CHANCE_RELEASE);
}

bool will_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

void create_msg_that_contains_rsc(char* mtext, int pid, struct resource_table* resource_table) {
    int resource_to_request, num_currently_allocated, max_claims, resource_to_claim;
    do {
        resource_to_request = get_random_resource();
		num_currently_allocated = resource_table->rsc_descs[resource_to_request].allocated[pid];
        max_claims = resource_table->max_claims[pid];
    } while (num_currently_allocated == max_claims);
    // We currently do not have more of this resource allocated then our max claims limits
	resource_to_claim = rand() % (max_claims-num_currently_allocated) + num_currently_allocated + 1;
	while(resource_to_claim > (max_claims - num_currently_allocated) || resource_table->rsc_descs[resource_to_request].total < resource_to_claim){
		resource_to_claim = rand() % (max_claims-num_currently_allocated) + num_currently_allocated + 1;
	}
	//Debug
	//fprintf(stderr, "\nUSER: Random amount is: %d, currently allocated: %d, Max allowed: %d\n\n", resource_to_claim, num_currently_allocated, max_claims);
    sprintf(mtext, "%d,RQST,%d,%d", pid, resource_to_request, resource_to_claim);
}

unsigned int get_random_resource() {
    return rand() % NUMBER_OF_RESOURCES;
}

unsigned int get_random_pct() {
    return (rand() % 99) + 1;
}

void add_signal_handlers() {
    struct sigaction act;
    act.sa_handler = handle_sigterm; // Signal handler
    sigemptyset(&act.sa_mask);      // No other signals should be blocked
    act.sa_flags = 0;               // 0 so do not modify behavior
    if (sigaction(SIGTERM, &act, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void handle_sigterm(int sig) {
    _exit(0);
}

struct Clock get_time_to_request_release_rsc(struct Clock sysclock) {
    unsigned int ns = get_nss_to_request_release();
    struct Clock time_to_request_release = sysclock;
    incr_clock(&time_to_request_release, ns);
    return time_to_request_release;
}

unsigned int get_nss_to_request_release() {
    unsigned int lower_bound = 100000;
    return (rand() % (800000 - lower_bound)) + lower_bound;
}