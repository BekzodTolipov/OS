/* Program Description: This program is designed to practice system call
 * stderr, fork, and getopt. it receives optional 3 options: -h for help
 * -i for input file, -o for output file, and -t fmr time. all options
 * come with default values. It will read first line in file which is 
 * identifier how many lines come after first. Each line starts with number
 * as sum and program finds if any number after sum can sum-up to sum.
 * 
 * Author: Bekzod Tolipov(Bek)
 * Date: 09/15/2019
 * Class: CS:4760-001
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

void child_function(char file_name[], int line_to_read, char out_file[]);
static int is_full_digit(char str[], bool child);
static int setuptimer(int s);
static int setupinterrupt();
static void myhandler(int s);
void selectionSort(int arr[], int n);
void swap(int *xp, int *yp);
bool isSubsetSum(int *set, int size, int sum, char *modify);
void nBits(int n, int size, int set[], int sum, char *modiy);
int sum_up(int size, int set[], int sum, char *modify);

pid_t parent_pid;
pid_t child_id;
int *arraA = 0;

#define MAXCHAR 1024
#define TIME_OUT 2
#define ERR_EXIT 1
#define SUCC_EXIT 0

int main(int argc, char **argv)
{     
	char input_file_name[MAXCHAR] = "input.dat";
	char output_file_name[MAXCHAR] = "output.dat";
	char time_in_string[MAXCHAR];
	int child_ids[MAXCHAR];
	int time_to_elapse = 10;
	char str[MAXCHAR];
	int rounds = 0;
	parent_pid = getpid();
	struct tm *tm;
	FILE *fp;
	FILE *fwrite;
	// Reading options given from argument
        int c;
	printf("\nWelcome to subset summation utility to make your life easier!\n\n");
        while ((c = getopt (argc, argv, "hi:o:t:")) != -1){
                switch (c)
                {                        
                	case 'h':
				printf("\n\nThis utility comes with following options:\n[-h -i input_file_name -o output_file_name -t time(in seconds)]\n\n\n");
				break;
			case 'i':
				strncpy(input_file_name, optarg, MAXCHAR);
                                break;
			case 'o':
				strncpy(output_file_name, optarg, MAXCHAR);
                                break;
			case 't':
				strncpy(time_in_string, optarg, 255);
				if(time_in_string[0] == '-'){
					fprintf(stderr, "ERROR: Negative time not allowed (%s)\n", time_in_string);
					abort();
				}
				time_to_elapse = toint(time_in_string);
                                break;
			default:
				fprintf(stderr, "Unknown option: Print help: %s -h\n\n", argv[0]);
				abort();
		}

	}
	//Validate input and output file names
	if(strcmp(input_file_name, output_file_name) == 0){
		fprintf(stderr, "ERROR: input file name(%s) and output file name(%s) are same not allowed\n", input_file_name, output_file_name);
		abort();	
	}
	//Open input file for reading
	fp = fopen(input_file_name, "r");
	fwrite = fopen(output_file_name, "a");
	if (fp == NULL){
		fprintf(fwrite, "ERROR: Could not open file %s\n",input_file_name);
		fprintf(stderr, "ERROR: Could not open file %s\n",input_file_name);
        	return 1;
    	}
	int number_of_lines = count_lines(input_file_name);
    	fgets(str, MAXCHAR, fp) != NULL;
	if(is_full_digit(str, false) == false){
		fprintf(fwrite, "ERROR: Wrong format, first line should contain only digit\n");
                fprintf(stderr, "Parent : ERROR: Wrong format, first line should contain only digit\n");
                fclose(fwrite);
                fclose(fp);
                exit(ERR_EXIT);	
	}
        rounds = atoi(str);
	//Validate if number of lines same as 1st line
	if((number_of_lines-1) != rounds){
		fprintf(fwrite, "ERROR: first line: %d != number of lines after: %d", rounds, number_of_lines-1);
		fprintf(stderr, "ERROR: first line: %d != number of lines after: %d\n", rounds, number_of_lines-1);
		fclose(fp);
		fclose(fwrite);
		return ERR_EXIT;
	}
	int counter = 0;
	if(setuptimer(time_to_elapse) == -1){
		fprintf(fwrite, "ERROR: Failed set up timer");
		fprintf(stderr, "ERROR: Failed set up timer");
		fclose(fp);
		fclose(fwrite);
		return ERR_EXIT;
	}
	if(setupinterrupt() == -1){
		fprintf(fwrite, "ERROR: Failed to set up handler");
		fprintf(stderr, "ERROR: Failed to set up handler");
		fclose(fwrite);
		fclose(fp);
		return ERR_EXIT;
	}
	int position = 0;
	int count = 0;
	while(counter < rounds){
		counter++;
		child_id = fork ();
  		if (child_id == 0){
			//Call child function
			child_function(input_file_name, counter+1, output_file_name);
			//Will exit if there was no error
			exit(SUCC_EXIT);
		}
		else{
			int status; 
     			child_ids[position] = child_id; 
			waitpid(child_id, &status, 0); 
			position++;
			if(WIFSIGNALED(status)){
				if(WTERMSIG(status) == SIGTERM){
					fprintf(stderr, "%d: ERROR: Ran out of time\n", parent_pid);
                                        fprintf(fwrite, "%d: ERROR: Ran out of time\n", parent_pid);
                                        count = 0;
        				while(count < position){
                				fprintf(fwrite, "Child_id: %d\n", child_ids[count]);
                				count++;
        				}
					fprintf(fwrite, "Parent_id: %d\n", parent_pid);
					fclose(fwrite);
                                        fclose(fp);
					return ERR_EXIT;
				}
			}

    			if ( WIFEXITED(status) ) 
    			{ 
        			int exit_status = WEXITSTATUS(status);
				if(exit_status == 2){
					fprintf(stderr, "%d: ERROR: Ran out of time\n", child_id);
					fprintf(fwrite, "%d: ERROR: Ran out of time\n", child_id);
					count = 0;
				        while(count < position){
                				fprintf(fwrite, "Child_id: %d\n", child_ids[count]);
                				count++;
        				}
					fprintf(fwrite, "Parent_id: %d\n", parent_pid);
					fclose(fwrite);
					fclose(fp);
					return ERR_EXIT;
				}
			}
		}
	}
	int child_status;
	/* Busy wait for the child to send a signal. */
  	//while ((parent_pid = wait(&child_status)) > 0);

	count = 0;
	while(count < position){
		fprintf(fwrite, "Child_id: %d\n", child_ids[count]);
		count++;
	}
	fprintf(fwrite, "Parent_id: %d\n", parent_pid);
	fclose(fp);
        fclose(fwrite);	
	printf("\nDone with program, so happy! :)\n");
	//kill(-parent_pid, SIGQUIT);
	return SUCC_EXIT;
}

/* The child process executes this function. */
void child_function(char file_name[], int line_to_read, char out_file[])
{
	FILE *fp;
	FILE *fwrite;
        char str[MAXCHAR];
        fp = fopen(file_name, "r");
        if (fp == NULL){
                fprintf(stderr, "ERROR: Could not open file %s\n",file_name);
                exit(ERR_EXIT);
        }

	fwrite = fopen(out_file, "a");
        if (fp == NULL){
                fprintf(stderr, "ERROR: Could not open file %s\n",file_name);
                exit(ERR_EXIT);
        }

        int line = 0;
        while(line != line_to_read){
		fgets(str, MAXCHAR, fp);
		line++;
	}
	fclose(fp);
	//Check if string only digits
	if(is_full_digit(str, true) == false){
                fprintf(fwrite, "%d: ERROR: Wrong format, should contain only digit\n", getpid());
                fprintf(stderr, "%d: ERROR: Wrong format, should contain only digit\n", getpid());
                fclose(fwrite);
                exit(ERR_EXIT);        
        }

	int len = 0;
	int sum_pos = 0;
	int sum = 0;
	int digit_to_save = 0;
	int digits_pos = 0;
	char *pstr = str;
	int size = 0;
	//Find out size first
	while(1 == sscanf(pstr, "%d%n", &digit_to_save, &len)){
		if(sum_pos == 0){
                        sum_pos++;
                        pstr += len;
                }
                else{
			size++;
                        pstr += len;
                }
	}
	//Save digits from string to int array
	int digits[size];
	char *newstr = str;
	sum_pos = 0;
	while(1 == sscanf(newstr, "%d%n", &digit_to_save, &len)){
                if(sum_pos == 0){
                        sum_pos++;
                        sum = digit_to_save;
                        newstr += len;
                }
                else{
			digits[digits_pos++] = digit_to_save;
                        newstr += len;
                }
        }
	//int n = sizeof(digits)/sizeof(digits[0]);
	if(setuptimer(1) == -1){
                perror("Failed set up timer\n");
                exit(ERR_EXIT);
        }
        if(setupinterrupt() == -1){
                perror("Failed to set up handler\n");
                exit(ERR_EXIT);
        }
	//Calculate subset sum problem
	char local_modify[MAXCHAR] = "";
	if (isSubsetSum(digits, size, sum, local_modify) == true){ 
		printf("%s\n", local_modify);
		fprintf(fwrite, local_modify);
		fclose(fwrite);
		exit(SUCC_EXIT);
	}
	else{
		fprintf(fwrite, "%d: No subset of numbers summed to %d\n", getpid(), sum);
     		fprintf(stderr, "%d: No subset of numbers summed to %d\n", getpid(), sum); 
		fclose(fwrite);
		exit(SUCC_EXIT);
	}	
  exit (ERR_EXIT);
}

/*Swap function for selection sort*/
void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
}  
/* Selection sort */
void selectionSort(int arr[], int n) 
{ 
	int i, j, min_idx; 
	for (i = 0; i < n-1; i++) 
    	{
		min_idx = i; 
        	for (j = i+1; j < n; j++) 
          		if (arr[j] < arr[min_idx]) 
            		min_idx = j;
		
		swap(&arr[min_idx], &arr[i]);
	}  
}

/* Identify if given string consist of fully digits */
static int is_full_digit(char str[], bool child){
	//If it not child than checking is about if digit only
	//If it is a child then checking is digits than if its not space
	int i;
	if(child == false){
        	for(i=0;str[i]!='\n';i++)
        	{
                	if(isdigit(str[i]) == 0)
                	{
                        	return false;
                	}
        	}
	}
	else{
		for(i=0;str[i]!='\n';i++)
                {
                        if(isdigit(str[i]) == 0){ 
				if(str[i] != ' ')
   				{
                                	return false;
				}
                        }
                }
	}
	return true;
}

/* Set up timer */
static int setuptimer(int time){
	struct itimerval value;
	value.it_value.tv_sec = time;
	value.it_value.tv_usec = 0;

	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	return(setitimer(ITIMER_REAL, &value, NULL));
}

/* Set up interrupt */
static int setupinterrupt(){
	struct sigaction act;
	act.sa_handler = &myhandler;
	act.sa_flags = SA_RESTART;
	return(sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
}
/* Set up my own handler */
static void myhandler(int s){
	if(parent_pid == getpid()){
		if(kill(child_id, 0) == 0){
			kill(child_id, SIGTERM);
		}
	}
	else{
		exit(TIME_OUT);
	}	
}

// Function is designed to convert string to an integer
int toint(char str[])
{
	int len = strlen(str)-1;
        int i, num = 0;
        for (i = 0; i < len; i++)
        {
		printf("this is i: %d", i);
        	num = num + ((str[len - (i + 1)] - '0') * pow(10, i));
        }
	return num;
}
/* Count how many lines in the file */
int count_lines(char file_name[]){

	FILE *fp;
	char str[MAXCHAR];
	fp = fopen(file_name, "r");
        if (fp == NULL){
                fprintf(stderr, "ERROR: Could not open file %s\n",file_name);
                return 1;
        }

        int number_of_lines = 0;
        while(fgets(str, MAXCHAR, fp) != NULL){
                number_of_lines++;
        }
	fclose(fp);
	return number_of_lines;
}   
/* Returns true if there is a subset of set[] with sun equal to given sum */
bool isSubsetSum(int *set, int size, int sum,  char *modify) 
{ 
	// The value of subset[i][j] will be true if there is a  
	// subset of set[0..j-1] with sum equal to i 
	bool subset[size+1][sum+1];
	int i;
	int j;
	// If sum is 0, then answer is true 
	for (i = 0; i <= size; i++) 
      		subset[i][0] = true; 

	// If sum is not 0 and set is empty, then answer is false
	for (i = 1; i <= sum; i++) 
      		subset[0][i] = false;    
	// Fill the subset table in botton up manner 
	for (i = 1; i <= size; i++) 
     	{ 
       		for (j = 1; j <= sum; j++) 
       		{ 
         		if(j<set[i-1]){ 
         			subset[i][j] = subset[i-1][j];
			} 
         		if (j >= set[i-1]) 
  	         		subset[i][j] = subset[i-1][j] || subset[i - 1][j-set[i-1]]; 
       		} 
     	}

	selectionSort(set, size);
	arraA = malloc(sizeof(int) * size);
	if(subset[size][sum])
		nBits(size, size, set, sum, modify);
/*	This algorithm failed but trying to figure out what else i can do
	//Checks if given sum can be summed-up with the following numbers
	if(subset[size][sum]){
		int numbers_to_take = 1;
		bool found = false;
		char str[MAXCHAR] = "";
		int total = 0;
		int g = 0;
		for(i = 0; i < size; i++){
			total = 0;
			numbers_to_take = 1;
			for(j=0; j < size; j++){
				//Do NOT check if number adding to itself = sum
				if(i == j)
					continue;
				total = set[i] + set[j];
				g = j+1; //next position to add and check
				int counter = 0;
				while(counter < numbers_to_take){
					if(g == size)
                                        	g = 0;
				//printf("Total looking: %d\n", total);
					//Do not go over array size to avoid Seg fault
					if(numbers_to_take > size)
						break;
					if(total > sum)
						break;
					if(total == sum){
						int length = 0;
						length += sprintf(str, "%d: %d + %d", getpid(), set[i], set[j]);
						int pos = g-1;
						int count = counter;
						while(count > 0){
							if(pos == j)
								break;
							if(pos < 0)
								pos = size-1;
							length += sprintf(str+length, " + %d", set[pos]);
							pos--;
							count--;
						}
						length += sprintf(str+length, " = %d\n", total);
						strcpy(modify, str);
						break;
					}
					if(g == size)
						g = 0;
					if(g == i)
						break;
					total += set[g++];
					counter++;
					numbers_to_take++;
				}
				if(found)
					break;
			}
			if(found)
				break;
		}
	}
*/
	return subset[size][sum];
}
bool found = false;
/*String with n bits*/
void nBits(int n, int size, int set[], int sum, char *modify) {
		if (n <= 0) {
			if(!found){
				if(sum_up(size, set, sum, modify)){
					found = true;
				}
				else
					found = false;
			}
		} else {
			arraA[n - 1] = 0;
			nBits(n - 1, size, set, sum, modify);
			arraA[n - 1] = 1;
			nBits(n - 1, size, set, sum, modify);
		}
}

int sum_up(int size, int set[], int sum, char *modify){
	int i;
	int total = 0;
	int pos_arr[size];
	int pos = 0;
	char str[MAXCHAR];
	for(i=0; i<size; i++){
		if(arraA[i] == 1){
			total+=set[i];
			pos_arr[pos] = set[i];
			pos++;
		}
		if(total > sum)
			break;
		else if(total == sum){
			int j = 0;
			int length = 0;
                        length += sprintf(str, "%d: %d", getpid(), pos_arr[0]);

			//printf("%d: %d ", getpid(), pos_arr[0]);
			for(j=1; j<pos; j++){
				length += sprintf(str+length, " + %d", pos_arr[j]);
				//printf("+ %d ", pos_arr[j]);
			}
			length += sprintf(str+length, " = %d\n", total);
			//printf("= %d\n", total);
			strcpy(modify, str);
			return true;
		}
		//total = 0;
	}
	return false;
}
