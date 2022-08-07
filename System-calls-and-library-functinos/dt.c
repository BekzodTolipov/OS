/*	Program Description: This program is designed to traverse given or 
 *	default directory using depth-first principle. This utility is a great
 *	way to practice system calls.
 *	Author: Bekzod Tolipov
 *	Date: 09/01/2019
 *	Class: CS:4760-001
 */
#include <stdio.h> 
#include <dirent.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <stdbool.h>
#include <grp.h>
#include <pwd.h>

// Prototypes for functions
void depthfirst(char *name, int lvl, int spaces, char **options, int optionCounter, int arr[], bool symbolic, bool combine);
int* filePermission(const char *name, char **options, int optionCounter, int arr[], int subSpaces, int lvl, bool combine);

int main(int argc, char **argv) 
{
	char *path = ".";
	int numberOfSpaces;
	int c;
	char spaces[1024] = "4";
	int arr[argc];
	bool symbolic = false;
	bool combineOptions = false;
	int intSpaces = 4;
	char **copyargv = argv;
	int count = 1;
	while (count < argc){
		char *pathCheck = argv[count];
		struct stat st;
		lstat(pathCheck, &st);
                if (S_ISDIR(st.st_mode))
			path = pathCheck;
		count++;
	}
	// reads in all the options given by user thru argv and saves it to
	// an array for other functions use
	int pos = 0;
	while ((c = getopt (argc, argv, "hI:Ldgipstul")) != -1){
		arr[pos] = c;
    		switch (c)
      		{
      			case 'h':
        			printf("\nTo run the program:\n%s [-h] [-I n] [-L -d -g -i -p -s -t -u | -l] [dirname]\n\n\nh - Print a help message and exit\nI n - Change indentation to n spaces for each level\nL - Follow symbolic links\nt - Print information on file type\np - Print permission bits as rwxrwxrwx\ni - Print the number of links to file in inode table\nu - Print the UID associated with the file\ng - Print the GID associated with the file\ns - Print the size of file in bytes\nd - Show the time of last modification\nl - Will print t p i u g s options\n", argv[0]);
        			return 0;
      			case 'L':
				symbolic = true;
				break;
			case 'I': 
				strncpy(spaces, optarg, 255);
				intSpaces = toint(spaces);
        			break;
      			case 'l':
				combineOptions = true;
				break;
      		}
		pos++;
	}
	// Passing all the arguments from the argv
	depthfirst(path, 0, intSpaces, copyargv, argc, arr, symbolic, combineOptions);
	
	return 0; 
}

// Function is designed to convert string to an integer
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
//Remove subSpaces not working
// Function is designed to print information about file by given options by user
int* filePermission(const char *name, char **options, int optionCounter, int arr[], int subSpaces, int lvl, bool combine){
	struct stat fileStat;
	lstat(name, &fileStat);
	int c;
	char mtime[80];
        time_t t = fileStat.st_mtime; /*st_mtime is type time_t */
        struct tm lt;
	struct group *grp;
	struct passwd *pwp;
	int count = 0;
	static int maxLVL = 0;
	int lvlCounter = 6;
	//printf("  %d", lvl);
	//while(lvlCounter < lvl){
		//printf("\t");
	//	lvlCounter-=2;
	//}
         //printf("\t\t");                                    

	if(lvl == 0)
		printf("\t");
	if(subSpaces == 0)
		printf("\t\t\t");
	else if(subSpaces < 0)
		printf("\b\b\b\b\b\b\b\b");
//	else
//		while(count < lvl){printf("\b"); count++;}
	//if(lvl % 3 == 0){printf("\b\b\b\b");}
	count = 0;
	while (count < optionCounter){
                if(!combine){	//Combine option given when user wants to combine 6 options together t p u i g s
			switch (arr[count])
                	{
                        	case 'p':	//Print permission
					if(S_ISDIR(fileStat.st_mode))
    						printf("d");
    					else if(S_ISLNK(fileStat.st_mode))
						printf("l");
					else
						printf("-");
					printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    					printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    					printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    					printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    					printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    					printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    					printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    					printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    					printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    					printf(" ");
					break;
				case 'i':	//Print number of link to given path
					printf(" %5d ",fileStat.st_nlink);
					break;
				case 'u':	//Print user id(should be tolipov)
					pwp = getpwuid(fileStat.st_uid);
					printf(" %5s", pwp->pw_name);
				case 'g':	//Print group id(should be osclass)
                                	grp = getgrgid(fileStat.st_gid);
                    			printf("   %5s", grp->gr_name);
				case 's':	//Print file size
					if(fileStat.st_size > 1000)
                                		printf(" %5dK ",fileStat.st_size/1000);
					else if(fileStat.st_size > 1000000)
						printf(" %5dM ",fileStat.st_size/1000000);
					else if(fileStat.st_size > 1e+9)
                                        	printf(" %5dG ",fileStat.st_size/1e+9);
					else
						printf(" %5dB ",fileStat.st_size);
					break;
				case 'd':	//Print last modified time
    					localtime_r(&t, &lt); //convert to struct tm
    					strftime(mtime, sizeof mtime, "%a, %d %b %Y %T", &lt);
    					printf("%5s", mtime);
					break;
				case 't':	//Print type of the file
					if(S_ISBLK(fileStat.st_mode)){
						printf("Block Special Type");
					}
					else if(S_ISCHR(fileStat.st_mode)){
						printf("Character Special Type");
					}
					else if(S_ISFIFO(fileStat.st_mode)){
     	                                   printf("FIFO Special Type");
        	                        }
					else if(S_ISREG(fileStat.st_mode)){
                        	                printf("Regular File Type");
                                	}
					else if(S_ISDIR(fileStat.st_mode)){
        	                                printf("Directory Type");
                	                }
					else if(S_ISLNK(fileStat.st_mode)){
                                	        printf("Symbolic Link Type");
                                	}
					break;
			}
		}
		else{
			switch (arr[count])
                	{
                        	case 'l':
					if(S_ISDIR(fileStat.st_mode))
                                                printf("d");
                                        else if(S_ISLNK(fileStat.st_mode))
                                                printf("l");
                                        else
                                                printf("-");
	                                printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
        	                        printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
                	                printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
                        	        printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
                                	printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
	                                printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
        	                        printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
                	                printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
                        	        printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
                                	printf(" ");
                                
	                                printf(" %5d ",fileStat.st_nlink);
        	                        
					pwp = getpwuid(fileStat.st_uid);
                        	        printf(" %5s", pwp->pw_name);
					grp = getgrgid(fileStat.st_gid);
	                                printf("   %5s", grp->gr_name);
	
        	                        if(fileStat.st_size > 1000)
                	                        printf(" %5dK ",fileStat.st_size/1000);
                        	        else if(fileStat.st_size > 1000000)
                                	        printf(" %5dM ",fileStat.st_size/1000000);
	                                else if(fileStat.st_size > 1e+9)
        	                                printf(" %5dG ",fileStat.st_size/1e+9);
                	                else
                        	                printf(" %5dB ",fileStat.st_size);
                               
					  if(S_ISBLK(fileStat.st_mode)){
	                                        printf("Block Special Type");
        	                        }
                	                else if(S_ISCHR(fileStat.st_mode)){
                        	                printf("Character Special Type");
                                	}
	                                else if(S_ISFIFO(fileStat.st_mode)){
        	                                printf("FIFO Special Type");
                	                }
                        	        else if(S_ISREG(fileStat.st_mode)){
                                	        printf("Regular File Type");
	                                }
        	                        else if(S_ISDIR(fileStat.st_mode)){
                	                        printf("Directory Type");
                        	        }
                                	else if(S_ISLNK(fileStat.st_mode)){
	                                        printf("Symbolic Link Type");
        	                        }
					break;
                		case 'd':
	                                localtime_r(&t, &lt); /* convert to struct tm */
        	                        strftime(mtime, sizeof mtime, "%a, %d %b %Y %T", &lt);
                	                printf("%5s", mtime);
                        	        break;
			}

		}
			
		count++;
	}
	printf("\n");

return arr;
}
// Function is designed to traverse directories recursivly
void depthfirst(char *name, int lvl, int spaces, char **options, int optionCounter, int arr[], bool symbolic, bool combine){
	struct dirent *de; // Pointer for directory entry
        DIR *dr;
	struct stat st;
	struct stat fileStat;
	char **copyOptions = options;
	int *localArr;
	int nameLength;
	
	// Check if given name(path) is directory
	if(!(dr = opendir(name))){
		perror(name);
		return;
	}
	// Check if it has read permission
	if(!(de = readdir(dr))){
		perror(name);
		return;
	}
	chdir(name);
	char cwd[5000];
	getcwd(cwd, sizeof(cwd));

	do {	
		lstat(de->d_name, &st);
		if (S_ISDIR(st.st_mode)) { //Check if its directory
            		char path[1024];
            		int len = snprintf(path, sizeof(path)-1, "%s/%s", name, de->d_name);
            		path[len] = 0;
            		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                		continue;
			printf("%*s%s\t\t\t\t\t", lvl*spaces,"", de->d_name);
			nameLength = strlen(de->d_name);
			nameLength += lvl*spaces;
			localArr = filePermission(de->d_name, copyOptions, optionCounter, arr,  -1, lvl, combine);
			depthfirst(de->d_name, lvl+1, spaces, copyOptions, optionCounter, localArr, symbolic, combine);
        	}
		else if(S_ISLNK(st.st_mode)){ //Check if it is symbolic link
			if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                                continue;

			printf("%*s%s\t", lvl*spaces,"", de->d_name);

			localArr = filePermission(de->d_name, copyOptions, optionCounter, arr,  0, lvl, combine);
			
			if(symbolic){ // If traversing symbolic link option given enter to symbolic link
				stat(de->d_name, &st);
				depthfirst(de->d_name, lvl+1, spaces, copyOptions, optionCounter, localArr, symbolic, combine);	
				chdir(cwd);
			}

		}
        	else{	//Failed both ifs means it is not symbolic or directory
			char path[1024];
                        int len = snprintf(path, sizeof(path)-1, "%s/%s", name, de->d_name);
                        path[len] = 0;
			
            		printf("%*s%s\t\t\t\t\t", lvl*spaces, "",  de->d_name);
			localArr = filePermission(de->d_name, copyOptions, optionCounter, arr, -1, lvl, combine);
        	}
    	} while (de = readdir(dr));
        chdir("..");
	closedir(dr);
}															
