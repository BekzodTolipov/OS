#pragma once
#ifndef HEADER_FILE
#define HEADER_FILE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>          /* O_CREAT, O_EXEC */
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdint.h>

#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <stdarg.h> 
#include <time.h>

#define thresh_hold_oss 100000
#define thresh_hold_user 70000
#define QUANTUM 50000
#define ALPHA 2.2
#define BETTA 2.8
#define MAX_PROCESS 18
#define MAX_CLAIMS 5
#define NUM_RSCS 20
#define QUEUE_SIZE 18
#define EVERY_FORK_TIME 500000000;


void fix_time();
static int setupinterrupt();
static void myhandler(int s);
int sec_to_millis(int q);
int ns_to_millis(int q);
void sem_clock_lock();
void sem_clock_release();
void sem_print_lock();
void sem_print_release();
static void terminator();

struct Clock {

    unsigned int sec;
    unsigned int ns;

};

struct Queue {

    struct QNode *front;
	struct QNode *rear;
	int count;
	int q[QUEUE_SIZE+2];

};

struct Message {

	long int mtype;
    int process_id;
    int done_flag;
    int id;
    int priority;
    int duration;
	int burst_time;
    int sec;
    int ns;
    char* message;
	int wait_time;

};

struct rescource_descriptor{
	unsigned int total;
	unsigned int allocated[MAX_PROCESS+1];
};

struct resource_table{
	struct resource_descriptor rsc_descrp[NUM_RSCS];
	unsigned int max_claims[MAX_PROCESS+1];
};

#endif
