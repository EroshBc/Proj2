//
//  oss.c
//  Proj2
//
//  Created by Erosh Boralugodage on 2/15/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>
#include <getopt.h>

#define SHM_KEY 18181

typedef  struct {
    int seconds;
    int nanoseconds;
}Clock


int main(int argc, char* argv[]){
  
    int n=0, s = 0, t=7;

    int option=0;

    /* get opt() use to pass the command line options
     loops runs untill getopt retutns -1
     switch statement value of options to determine options passed
     if unknown option passed exit form the program*/

    while((option = getopt(argc, argv, "hn:s:t:")) != -1){
        switch(option){
            case 'h':
                printf("oss [-h] [-n proc] [-s simul] [-t timelimit]\n");
                printf("number of total children to launch,\n");
                printf("bound of the time that a child process will be launched\n");
                printf("the simul parameter indicates how many children to allow to run simultaneously.\n");
                exit(0);
                break;

            case 'n':
                n = atoi(optarg);
                
                break;
            case 's':
                s = atoi(optarg);
                
                break;
            case 't':
                t = atoi(optarg);
                
                break;
            case '?':
                printf("Invalid option entered\n");
                printf("Exit from the program");
                exit(0);
                break;

        }
    }
    
    
    
    
    
    // Create shared memory segment for the simaulaed system clock
    
    int shm_id = shmget(SHM_KEY, sizeof(Clock),0666);
    if (shm_id == -1) {
            perror("shmget");
            exit(EXIT_FAILURE);
        }
    
    //Attach shared memory segment to process address
    Clock *clock = (Clock *) shmat(shm_id, NULL, 0);
        if (clock == (Clock *) -1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }
    
    // initialize the simulated system clock
    clock->seconds = 0;
    clock->nanoseconds =0;
    
    
    
    
    //Detach shared memory segment from process address space
    if (shmdt(clock) == -1) {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }

    // Destroy shared memory segment
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(EXIT_FAILURE);
        }
    
    
    
    
    // random number generator for seconds and nanseconds
    srand(time(0));
   
    int nanoseconds = rand()% (1000000000) + 1;
    int seconds = rand()%(t-1) + 1;
    
    printf("System clock is now Seconds: %d and nanoseconds : %d", seconds, nanoseconds);
    
    
    return 0;
}
