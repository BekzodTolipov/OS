#include "bankers.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

#define NUMBER_OF_RESOURCES 20
#define MAX_PROCESS_COUNT 18

bool bankers_algorithm(struct resource_table* resource_table, int pid, int rqstd_rsrc, int numb_rqsted) {
    unsigned int i, j;
    
    // We grant the request and then check if there is a safe state
    //resource_table->rsc_descs[rqstd_rsrc].allocated[pid]+=numb_rqsted;

    unsigned int* available_resources = get_available_resources(resource_table); 
    unsigned int** needs = get_needs_matrix(resource_table);
    unsigned int* work = get_work_arr(available_resources);		//Copy available to work
    bool* can_finish = get_can_finish();

    /*
     *  Determine if there is a safe sequence
     */
    unsigned int num_that_could_finish = 0;

    do {
        num_that_could_finish = 0;

        // Check if each process can finish executing
        // If it can then add its allocated resources to the work vector
        for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
            if (can_finish[i]) {
                // We've already determined that this process can finish
                continue;
            }
            // For process i
            for(j = 0; j < NUMBER_OF_RESOURCES; j++) {
				//if(!resource_table->rsc_descs[j].shareable){
					// Check if needs is greater than available
					if (needs[i][j] > available_resources[j]) {
						// If it is then we cannot finish executing
						can_finish[i] = 0;
						break;
					}
			//	}
            }
            if (can_finish[i]) {
                // Can finish so add process i's allocated resources to the work vector
                for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
                    work[j] += resource_table->rsc_descs[j].allocated[i];
                }
                num_that_could_finish++;
            }
        }

    } while (num_that_could_finish > 0);

    bool safe_sequence_exists = check_for_safe_sequence(can_finish);
    
    // Restore resource table state
    //resource_table->rsc_descs[rqstd_rsrc].allocated[pid]-=numb_rqsted;
    
    free(available_resources);
    free(work);
    free(can_finish);
    free(needs);

    return safe_sequence_exists;
}

unsigned int* get_work_arr(unsigned int* available_resources) {
    unsigned int i;
    unsigned int* work = malloc(sizeof(unsigned int) * NUMBER_OF_RESOURCES);
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        work[i] = available_resources[i];
    }
    return work;
}

bool* get_can_finish() {
    unsigned int i;
    bool* can_finish = malloc(sizeof(bool) * MAX_PROCESS_COUNT+1);
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        can_finish[i] = 1;	//Set every process to false
    }
    return can_finish;
}

unsigned int** get_needs_matrix(struct resource_table* resource_table) {
    unsigned int i, j;
    unsigned int** needs = create_array(MAX_PROCESS_COUNT+1, NUMBER_OF_RESOURCES);
    unsigned int max_processes, allocated_processes;
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
            max_processes = resource_table->max_claims[i];                   // Max number of resources for process i
            allocated_processes = resource_table->rsc_descs[j].allocated[i]; // Number of allocated resources for process i
            needs[i][j] = max_processes - allocated_processes;
			//Debug
			//fprintf(stderr, "Process(%d) need:(%d)\n", i, needs[i][j]);
        }
    }
	//sleep(2);
    return needs;
}

bool check_for_safe_sequence(bool* can_finish) {
    unsigned int i;
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        if (!can_finish[i]) {
			//Debug
			//fprintf(stderr, "RESOURCE: process will be blocked if give resouce: (%u)", i);
            return 0;
        }
    }
    return 1;
}
