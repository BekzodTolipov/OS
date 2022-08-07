#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>

void print_usage();
bool parse_cmd_line_args(int argc, char* argv[]);
void set_timer(int duration);
bool event_occured(unsigned int pct_chance);
unsigned int** create_array(int m, int n);
void destroy_array(unsigned int** arr);
char** split_string(char* str, char* delimeter);

#endif
