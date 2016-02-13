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

void printStats(int wallClockTime, struct rusage usageInfo, struct rusage prevInfo, int prev) {
	if (prev) {
	/* Print statistics using prev */
	// Wall Clock Time
	printf("Wall Clock Time: %d \n", wallClockTime);
	// CPU Time
	printf("User CPU Time used: %ld \n", (long int) usageInfo.ru_utime.tv_usec - prevInfo.ru_utime.tv_usec);
	printf("System CPU Time used: %ld \n", (long int) usageInfo.ru_stime.tv_usec - prevInfo.ru_stime.tv_usec);
	// Involuntary
	printf("Involuntary context switches: %ld \n", usageInfo.ru_nivcsw - prevInfo.ru_nivcsw);
	// Voluntary	
	printf("Voluntary context switches: %ld \n", usageInfo.ru_nvcsw - prevInfo.ru_nvcsw);
	// Page Faults
	printf("Page faults: %ld \n", usageInfo.ru_majflt - prevInfo.ru_majflt);
	printf("Page reclaims: %ld \n", usageInfo.ru_minflt - prevInfo.ru_minflt);
	}
	else {
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
}

int main(void){
	int status, wallClockTime;
	int runOnce = 0;

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
		
		//clear args for next loop
		for (j=0; j < ARG_SIZE; j++) {
			args[j] = NULL;
		}

		printf("=> ");
		
		fgets(lineIn, BUFFER_SIZE, stdin);

		token = strtok(lineIn, delims);
		
		//Calls prompt if detected null token
		if (token == NULL) continue;
		
		//Propagates tokens
		while (token != NULL) {
			args[i] = token;
			token = strtok(NULL, delims);
			i++;
		}

		//Handle Exit
		if (strcmp(args[0],"exit") == 0) {
			exit(0);
		}
		//Handle CD
		else if (strcmp(args[0],"cd") == 0) {
			if (chdir(args[1]) == -1) {
			printf("%s doesn't exist, exit with code %i", args[1],errno);
			}
		}
		
		// Begin forking
		else if(fork() != 0) {
			// Polls for time before fork begins	
			gettimeofday(&timeBefore,NULL);
			/* Parent Code */
			/*Wait for death of child process*/
			waitpid(-1, &status, 0);
			if (status != 0) printf("Error from child with status %i\n", status);
			
			/*Gets Resource Usage Information */
			getrusage(RUSAGE_CHILDREN, &usageInfo);

			/*Gets time after death of child process*/
			gettimeofday(&timeAfter,NULL);

			/*Substracts the usec values of the timeval structs
			  giving us microseconds */
			wallClockTime = timeAfter.tv_usec-timeBefore.tv_usec;
			
			printStats(wallClockTime, usageInfo, prevInfo, runOnce);
		}
	
		else {
			/* Children Code */
			/* Executes new process */
			execvp(args[0], args);
			/* Exits when done */
			exit(errno);
		}
		runOnce = 1;
		prevInfo = usageInfo;
	}
	return 0;
}


