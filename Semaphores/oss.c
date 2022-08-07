/********************************************************************************************  
 *		Program Description: Simple program demonstration of inter process communication	*
 *      by using following system calls(fork, exec, shared memory, and semaphore)			*
 *      "oss" will generate shared memory for simulated clock which will have seconds		*
 *		and nanoseconds, in addition to that shared for messages.							*
 *		"oss" will start by forking of by given maximum number of process allowed to be		*
 *		spawned and keep itterating loop and increment simulated clock by 10,000.			*
 *		In addition to that it will replace all kids thats been terminated along the		*
 *		way.																				*
 *		After each termination of children oss will receive message and output that			*
 *		message along with it's own message to give or default output file					*
 *																							*
 *      Author: Bekzod Tolipov																*
 *      Date: 10/01/2019																	*
 *      Class: CS:4760-001																	*
 *******************************************************************************************/

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

//Prototypes
int toint(char str[]);
static int setuptimer(int s);
static int setupinterrupt();
static void myhandler(int s);
void fix_time();
void sem_clock_lock();
void sem_clock_release();

/////////////////////

pid_t parent_pid;
pid_t child_id;
pid_t child_arr[20];
static bool quit;
char *shmMsg;

//Shared clock
struct Clock{
	int sec;
	int ns;
};
//Shared memory set up
struct Clock *clock_point;
int *check_permission;
//Semaphore set up
struct sembuf sem_op;
int sem_id;

#define MAXCHAR 1024
int main(int argc, char **argv)
{
	char file_name[MAXCHAR] = "log.dat";
	char dummy[MAXCHAR];
	int numb_child = 5;
	int max_time = 5;
	int c;
	quit = false;
	
	parent_pid = getpid();
	// Read the arguments given in terminal
	while ((c = getopt (argc, argv, "hs:l:t:")) != -1){
		switch (c)
		{
			case 'h':
				printf("To run the program you have following options:\n\n[ -h for help]\n[ -s maximum number of processes spawned ]\n[ -l filename ]\n[ -t time in seconds ]\nTo execute the file follow the code:\n./%s [ -h ] or any other options", argv[0]);
				return 0;
			case 's':
				strncpy(dummy, optarg, 255);
                numb_child = toint(dummy);
                if(numb_child > 20 || numb_child < 1){
					fprintf(stderr, "ERROR: number of children should between 1 and 20");
					abort();
				}
				break;
			case 'l':
				strncpy(file_name, optarg, 255);
				break;
			case 't':
				strncpy(dummy, optarg, 255);
				max_time = toint(dummy);
                break;
			default:
				fprintf(stderr, "ERROR: Wrong Input is Given!");
				abort();

		}
	}

	FILE *fptr;
	fptr = fopen(file_name, "w");
	// Validate if file opened correctly
	if(fptr == NULL){
		fprintf(stderr, "Failed to open the file, terminating program\n");
		return 1;
	}
	setvbuf(fptr, NULL, _IONBF, 0);

	// ftok to generate unique key 
	key_t key = ftok("./oss.c", 21);  
	key_t key_2 = ftok("./oss.c", 22);
	key_t key_3 = ftok("./oss.c", 23);
	key_t key_4 = ftok("./oss.c", 24);

	// shmget returns an identifier 
	int shmid = shmget(key,sizeof(struct Clock),0644 | IPC_CREAT);
	if(shmid < 0){
		fprintf(stderr, "shmget failed on first key");
		return 1;
	}
	int shmid_2 = shmget(key_2, 2048,0666 | IPC_CREAT);
	if(shmid_2 < 0){
		fprintf(stderr, "shmget failed on second key");
        return 1;

	}
	int shmid_3 = shmget(key_4, sizeof(int), 0666 | IPC_CREAT);
	if(shmid_3 < 0){
        fprintf(stderr, "shmget failed on 4th key");
        return 1;
    }

	// shmat to attach to shared memory 
	clock_point = shmat(shmid,NULL,0);
	shmMsg = shmat(shmid_2,NULL,0);
	if(shmMsg == (char*)-1){
		fprintf(stderr, "shmat failed on shared message");
		return 1;
	}
	check_permission = shmat(shmid_3, NULL, 0);
	
	//Set up starter values for clock and message signal
	*check_permission = 0;
	clock_point->sec = 0;
	clock_point->ns = 0;
 	
	// We initialize the semaphore id
	sem_id = semget(key_3, 2, IPC_CREAT | IPC_EXCL | 0666);
	if(sem_id == -1){
		fprintf(stderr, "ERROR: Failed semget");
		return 1;
	}
	// Now we will set up 2 semaphores
	semctl(sem_id, 0, SETVAL, 1);
	semctl(sem_id, 1, SETVAL, 1);
	
	// System Timer set up
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

	int i;
    for(i=0; i<20; i++){	//Initialize array to zero's
			child_arr[i] = 0;
	}

    int child_pid;
    int count = 0;
    int forked_kids_total = 0;
    bool found = false;
	bool max_reached = false;
	int hundred_or_time = 0;	// Identifies which interrupt happened
    while(!quit){
		// Keep track of how many processes alive at each itteration
        if(count < numb_child){
            child_pid = fork();
			forked_kids_total++;
			// Check if total processes reached 100
			if(forked_kids_total >= 100){
				max_reached = true;
				hundred_or_time = 1;
				quit = true;
			}
			// Check if simulated time reached 2 seconds
			if(clock_point->sec > 2){
				max_reached = true;
				hundred_or_time = 1;
				quit = true;
			}
        }

        if (child_pid == 0) {
            execl("./user", "./user", file_name, NULL);	// Execute user executable file
        }
        else if(child_pid == -1){	// If fork failed than output necessary message
            perror("Error: Fork() failed\n");
        }
        else {
            found = false;
            for(i=0;i<20;i++){	// Checks kid is still alive
                if(child_arr[i] == child_pid){
                    found = true;
                }
            }
            if(!found){	// If new kid is spawnable then spawn new kid
                for(i=0; i<20; i++){
                    if(child_arr[i] == 0){
                        child_arr[i] = child_pid;
                        count++;
                        break;
                    }
                }
            }
		}
		int stat;
        pid_t remove_pid = waitpid(-1, &stat, WNOHANG);	// Non block wait for parent
		// If somebody died then barry them underground
		// and remove them from history
        if(remove_pid > 0){
            for(i=0; i<20; i++){
                if(child_arr[i] == remove_pid)
                {
                    child_arr[i] = 0;
	                   break;
                }
            }
            count--;
        }
		// Ask for permission to clock critical resource
        sem_clock_lock();

        //increment seconds
        clock_point->ns += 1000;
        fix_time();
		// Release the critical section
        sem_clock_release();
		// Check if child sent the message
		if(*check_permission == 1){
			fprintf(fptr, "\nMaster: Child pid is terminating at my time %d.%d because %s\n", clock_point->sec, clock_point->ns, shmMsg);
			strcpy(shmMsg, "\0");
			*check_permission = 0;
		}
    }
	// If oss loop got termincated by reaching either 100 kids or 2 seconds simulated time
	// will terminated all existing kids
	if(max_reached){
		if(hundred_or_time == 1){
			fprintf(stderr, "\nWARNING: Hundred processes\n");
		}
		else{
			fprintf(stderr, "\nWARNING: 2 second time reached\n");
		}
		int i;
	    for(i=0; i<20; i++){
			if(child_arr[i] != 0){
				if(kill(child_arr[i], 0) == 0){
					if(kill(child_arr[i], SIGTERM) != 0){
						perror("Child can't be terminated for unkown reason\n");
					}
				}
			}
		}

		// Wait for kids to get terminated
		for(i=0;i<20;i++){
			if(child_arr[i] != 0){
				waitpid(child_arr[i], NULL, 0);
			}
		}
	}
	
	printf("\nFreeing memories\n");
	// Clean up memory
    shmdt(clock_point);
    shmdt(shmMsg);
	shmdt(check_permission);
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmid_2, IPC_RMID, NULL);
	shmctl(shmid_3, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    semctl(sem_id, 1, IPC_RMID);
	// Close the file
	fclose(fptr);
    return 0;
}

/*******************************************************
* Function is designed to convert string to an integer *
*******************************************************/
int toint(char str[])
{
    int len = strlen(str);
    int i, num = 0;

    for (i = 0; i < len; i++)
    {
        num = num + ((str[len - (i + 1)] - '0') * pow(10, i));
    }

   return num;
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
	int i;
	for(i=0; i<20; i++){
		if(child_arr[i] != 0){
			if(kill(child_arr[i], 0) == 0){
				if(kill(child_arr[i], SIGTERM) != 0){
					perror("Child can't be terminated for unkown reason\n");
				}
			}
		}
	}

	for(i=0;i<20;i++){
		if(child_arr[i] != 0){
			waitpid(child_arr[i], NULL, 0);
		}
	}

	quit = true;
}

/***********************************************
* Function to trim down nanoseconds to seconds *
***********************************************/
void fix_time(){
    if((int)(clock_point->ns / 1000000000) == 1){
        clock_point->sec++;
        clock_point->ns -= 1000000000;
    }
}

/***************************
* Lock the clock semaphore *
***************************/
void sem_clock_lock(){
	sem_op.sem_num = 0;
	sem_op.sem_op = -1;
	sem_op.sem_flg = 0;
	semop(sem_id, &sem_op, 1);
}

/**************************
* Release clock semaphore *
**************************/
void sem_clock_release(){
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

