#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resource.h"
#include "helpers.h"

void print_rsc_summary(struct resource_table* resource_table, FILE* fptr) {
    unsigned int* total_resources = get_total_resources(resource_table);
    unsigned int* allocated_resources = get_allocated_resources(resource_table);
    unsigned int* available = get_available_resources(resource_table);

    char buffer[100];

    sprintf(buffer, "%48s", "<<<<Resource Summary>>>>\n");
    fprintf(stderr, buffer);

    print_resources(total_resources, "<<<<Total Resources>>>>\n", fptr);
    print_resources(allocated_resources, "<<<<Allocated Resources>>>>\n", fptr);
    print_resources(available, "<<<<Available Resources>>>>\n", fptr);

    free(total_resources);
    free(allocated_resources);
    free(available);
}

void print_resources(unsigned int* resources, char* title, FILE* fptr) {
    int i;
    char buffer[MAXCHAR];
    
    // Print title
    sprintf(buffer, "\n");
    sprintf(buffer + strlen(buffer), "%s", title);
    
    // Print column titles
    sprintf(buffer + strlen(buffer), "  ");
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        sprintf(buffer + strlen(buffer),"R%-4d", i+1);
    }
    sprintf(buffer + strlen(buffer),"\n");
    
    // Print data
    sprintf(buffer + strlen(buffer), "  ");
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        sprintf(buffer + strlen(buffer),"%-5d", resources[i]);
    }
    sprintf(buffer + strlen(buffer),"\n");
    fprintf(stderr, buffer);
    fprintf(fptr, buffer);
}

void allocate_resource_table(struct resource_table* resource_table) {
    int i, count = 0;
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        resource_table->rsc_descs[i] = get_rsc_desc();
		if(count < 4){
			resource_table->rsc_descs[i].shareable = true;
		}
		else{
			resource_table->rsc_descs[i].shareable = false;
		}
    }
	for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        resource_table->max_claims[i] = 0;
    }
}

struct resource_descriptor get_rsc_desc() {
    struct resource_descriptor rsc_desc = {
        .total = get_num_resources(),
    };
    init_allocated(rsc_desc.allocated);
    return rsc_desc;
}

unsigned int get_num_resources() {
    return (rand() % 10) + 1; // 1 - 10 inclusive
}

void init_allocated(unsigned int* allocated) {
    int i;
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        allocated[i] = 0;
    }
}

unsigned int get_max_resource_claims(int MAX) {
	//fprintf(stderr, "Generating max from given: (%d)", MAX);
    return rand() % (MAX-1) + 1;
}

void release_resources(struct resource_table* resource_table, int pid) {
    unsigned int i;
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        resource_table->rsc_descs[i].allocated[pid] = 0; 
    }
}

unsigned int* get_current_alloced_rscs(int pid, struct resource_table* resource_table) {
    unsigned int num_resources, num_resource_classes = 0;
    unsigned int i, j;

    num_resource_classes = get_number_of_allocated_rsc_classes(pid, resource_table);

    unsigned int* allocated_resources = malloc(sizeof(unsigned int) * num_resource_classes);
    j = 0;
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        num_resources = resource_table->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            allocated_resources[j++] = i;
        }
    }
    return allocated_resources;
}

unsigned int get_number_of_allocated_rsc_classes(int pid, struct resource_table* resource_table) {
    unsigned int num_resources, num_resource_classes = 0;
    unsigned int i;
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        num_resources = resource_table->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            num_resource_classes++;
        }
    }
    return num_resource_classes;
}

bool has_resource(int pid, struct resource_table* resource_table) {
    unsigned int i;
    unsigned int num_resources = 0;
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        num_resources = resource_table->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            return 1;
        }
    }
    return 0;
}

bool resource_available(struct resource_table* resource_table, int rqstd_rsrc, int numb_rqsted) {
	
    unsigned int* allocated_resources = get_allocated_resources(resource_table);
    unsigned int currently_allocated = allocated_resources[rqstd_rsrc];
    unsigned int total = resource_table->rsc_descs[rqstd_rsrc].total;
    free(allocated_resources);
    if (currently_allocated == total) {	//NO resource currently available
        return 0;
    }
	else if((currently_allocated+numb_rqsted) > total){
		return 0;
	}
    return 1;
}

unsigned int* get_available_resources(struct resource_table* resource_table) {
    unsigned int i;
    unsigned int* allocated_resources = get_allocated_resources(resource_table);
    unsigned int* total_resources = get_total_resources(resource_table);
    unsigned int* available_resources = malloc(sizeof(unsigned int) * NUMBER_OF_RESOURCES);
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available_resources[i] = total_resources[i] - allocated_resources[i];
    }
    
    free(allocated_resources);
    free(total_resources);
    
    return available_resources;
}

unsigned int* get_total_resources(struct resource_table* resource_table) {
    unsigned int i;
    unsigned int* total_resources = malloc(sizeof(unsigned int) * NUMBER_OF_RESOURCES);
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        total_resources[i] = resource_table->rsc_descs[i].total;
    }
    return total_resources;
}

unsigned int* get_allocated_resources(struct resource_table* resource_table) {
	
    unsigned int i, j;
    unsigned int* allocated_resources = malloc(sizeof(unsigned int) * NUMBER_OF_RESOURCES);
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        allocated_resources[i] = 0;
    }

    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {	//Find all allocated resources for each processes
        for (j = 1; j <= MAX_PROCESS_COUNT; j++) {
            allocated_resources[i] += resource_table->rsc_descs[i].allocated[j];
        }
    }
    return allocated_resources;
	
}

void print_allocated_resource_table(struct resource_table* resource_table, FILE* fptr) {
    int i, j;
    char buffer[MAXCHAR];
    sprintf(buffer,"\n");
    sprintf(buffer + strlen(buffer),"%61s", "<< Current Allocated Resources >>\n");
    sprintf(buffer + strlen(buffer),"     ");
     // print column titles
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        sprintf(buffer + strlen(buffer),"R%-3d", i+1);
    }
    sprintf(buffer + strlen(buffer),"\n");
    for (i = 1; i <= MAX_PROCESS_COUNT; i++) {
        sprintf(buffer + strlen(buffer),"P%-4d", i);
        // print all resources allocated for process i
        for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
            sprintf(buffer + strlen(buffer),"%-4d", resource_table->rsc_descs[j].allocated[i]);
        }
        sprintf(buffer + strlen(buffer),"\n");
    }
    sprintf(buffer + strlen(buffer),"\n");
    fprintf(stderr, buffer);
    fprintf(fptr, buffer);
}

