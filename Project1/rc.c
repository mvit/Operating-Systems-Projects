/* Written by Christopher Navarro (cjnavarro)
   and Maurizio Vitale (mvitale) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[]){
	// Child process status
	int status;
	// Time Calculation Variables/Structs
	int wallClockTime;

	struct timeval timeBefore;
	struct timeval timeAfter;

	// Usage Calculation Structs
	struct rusage usageInfo;
	
	
	// Correctness Checker
	if (argc < 2) {
		printf("Not enough parameters to perform process\n");
		printf("Usage is ./rc COMMAND PARAMETERS\n");
		exit(1);
	}
	// Polls for time before fork begins	
	gettimeofday(&timeBefore,NULL);
	// Begin forking
	if(fork() != 0) {
		/* Parent Code */
		/*Wait for death of child process*/

		waitpid(-1, &status, 0);

		/*Gets Resource Usage Information */
		getrusage(RUSAGE_CHILDREN, &usageInfo);

		/*Gets time after death of child process*/
		gettimeofday(&timeAfter,NULL);

		/*Substracts the usec values of the timeval structs
		  giving us microseconds */
		wallClockTime = timeAfter.tv_usec-timeBefore.tv_usec;
		
		if (status != 0) printf("%s exited with error code %d\n", argv[1], status);
	}
	
	else {
		/* Children Code */
		/* Executes new process */
		execvp(argv[1], argv+1);
		/* Exits when done */
		exit(errno);
	}
	
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

	return 0;
}

