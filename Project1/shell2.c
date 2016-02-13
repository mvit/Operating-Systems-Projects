	/* Written by Christopher Navarro (cjnavarro)
	   and Maurizio Vitale (mvitale) */

	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>
	#include <sys/resource.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <string.h>


	#define TRUE 1
	#define BUFFER_SIZE 128
	#define ARG_SIZE 32

	void printStats(int wallClockTime, struct rusage usageInfo) {
		/* Print statistics */
		// Wall Clock Time
		printf("Wall Clock Time: %d \n", wallClockTime);
		// CPU Time
		printf("User CPU Time used: %d \n", (int) usageInfo.ru_utime.tv_usec);
		printf("System CPU Time used: %d \n", (int) usageInfo.ru_stime.tv_usec);
		// Involuntary
		printf("Involuntary context switches: %ld \n", usageInfo.ru_nivcsw);
		// Voluntary	
		printf("Voluntary context switches: %ld \n", usageInfo.ru_nvcsw);
		// Page Faults
		printf("Page faults: %ld \n", usageInfo.ru_majflt);
		printf("Page reclaims: %ld \n", usageInfo.ru_minflt);
	}

	int main(void){
		int status = 0, wallClockTime;
		int bgflag = 0, bgprocess = 0;

		char delims[] = " \n";
		char lineIn[BUFFER_SIZE];
		char *args[ARG_SIZE];
		
		struct timeval timeBefore;
		struct timeval timeAfter;
		struct rusage usageInfo;
		struct rusage prevInfo;
		
		
		while(TRUE){
			char *token;
			int i = 0, j=0;
			
				while (bgprocess) {
					if (wait3(&status, WNOHANG, &usageInfo)) {
						bgprocess = 0;
						printf("Process %i completed!\n", getpid());
						gettimeofday(&timeAfter,NULL);

					/*Substracts the usec values of the timeval structs
					  giving us microseconds */
						wallClockTime = timeAfter.tv_usec-timeBefore.tv_usec;
				
						printStats(wallClockTime, usageInfo);
					}
					else {
						break;
					}
				}
			// reset args buffer
			for (j=0; j < ARG_SIZE; j++) {
				args[j] = NULL;
			}
			//give prompt
			printf("=> ");
			//get line
			fgets(lineIn, BUFFER_SIZE, stdin);
			//buffer check
			if(strlen(lineIn) > 128) {
				printf("Exceeded maximum buffer limit of 128\n");
				continue;
			}
			// fetch first token
			token = strtok(lineIn, delims);
			//null check
			if (token == NULL) continue;
			//Propagates tokens
			while (token != NULL) {
					args[i] = token;
					token = strtok(NULL, delims);
					i++;
			}
			if (i > 32) {
				printf("Exceeded maximum number of arguments (32)\n");
				continue;
			}
			// ampersand compare
			if (strcmp(args[i-1],"&")==0) {
				bgflag = 1;
				args[i-1]="NULL";
			}
			//Handle Exit
			if (strcmp(args[0],"exit") == 0) {
				if (!bgprocess){
					exit(0);
				}
				else {
					printf("can't exit shell due to running background process\n");
					continue;
				}
			}
			//Handle CD
			else if (strcmp(args[0],"cd") == 0) {
				if (chdir(args[1]) == -1) {
				printf("%s doesn't exist, exit with code %i", args[1],errno);
				}
			}
			
			// Begin forking
			else if(fork() != 0) {			
				/* Parent Code */
				
				// Polls for time before fork begins	
				gettimeofday(&timeBefore,NULL);
				/*Wait for death of child process*/
				// BG has been signaled
				 if (bgflag == 1) {
				 	 bgprocess = 1;
				 	 bgflag = 0;
				 }
				//Normal child case
				else {
					wait3(&status,0,&usageInfo);
					/*Gets time after death of child process*/
					gettimeofday(&timeAfter,NULL);

					/*Substracts the usec values of the timeval structs
					  giving us microseconds */
					wallClockTime = timeAfter.tv_usec-timeBefore.tv_usec;
					//Prints the stats
					printStats(wallClockTime, usageInfo);
				}
				//In case of error
				if (status != 0) printf("Error from child with status %i\n", status);
				

			}
		
			else {
				/* Children Code */
				/* Executes new process */
				execvp(args[0], args);
				/* Exits when done */
				exit(errno);
			}
		}
		return 0;
	}


