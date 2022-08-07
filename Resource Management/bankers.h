#ifndef BANKERS_H
#define BANKERS_H

#include "resource.h"
#include <stdbool.h>

bool bankers_algorithm(struct resource_table* resource_table, int pid, int rqstd_rsrc, int numb_rqsted);
unsigned int* get_work_arr(unsigned int* available_resources);
bool* get_can_finish();
bool check_for_safe_sequence(bool* can_finish);
unsigned int** get_needs_matrix(struct resource_table* resource_table);

#endif
