#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_RESOURCES 20
#define MAX_PROCESS_COUNT 18
#define MAXCHAR 2500 

struct resource_descriptor {
    unsigned int total;
    unsigned int allocated[MAX_PROCESS_COUNT+1];
	unsigned int rqsted_before_blocked[MAX_PROCESS_COUNT+1];
	bool shareable;
};

struct resource_table {
    struct resource_descriptor rsc_descs[NUMBER_OF_RESOURCES];
    unsigned int max_claims[MAX_PROCESS_COUNT+1];
};

void allocate_resource_table(struct resource_table* resource_table);
struct resource_descriptor get_rsc_desc();
void init_allocated(unsigned int* allocated);
unsigned int get_num_resources();
unsigned int get_max_resource_claims(int MAX);
void release_resources(struct resource_table* resource_table, int pid);
void print_available_resource_table(struct resource_table* resource_table, FILE* fptr);
unsigned int* get_current_alloced_rscs(int pid, struct resource_table* resource_table);
unsigned int get_number_of_allocated_rsc_classes(int pid, struct resource_table* resource_table);
bool has_resource(int pid, struct resource_table* resource_table);
bool resource_available(struct resource_table* resource_table, int rqstd_rsrc, int numb_rqsted);
unsigned int* get_allocated_resources(struct resource_table* resource_table);
unsigned int* get_available_resources(struct resource_table* resource_table);
unsigned int* get_total_resources(struct resource_table* resource_table);
void print_resources(unsigned int* resources, char* title, FILE* fptr);
void print_rsc_summary(struct resource_table* resource_table, FILE* fptr);
void print_allocated_resource_table(struct resource_table* resource_table, FILE* fptr);

#endif
