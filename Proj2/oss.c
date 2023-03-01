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
    long  seconds;
    long nanoseconds;
}Clock;


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
    
    
    
    
    
    // Create shared memory segment for the simulated system clock
    
    int shm_id = shmget(SHM_KEY, sizeof(Clock),0666|IPC_CREAT);
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
    
    
    // random number generator for seconds and nanseconds
    //initialize system clock
    srand(time(0));
   
     clock->nanoseconds = rand()% (1000000000) + 1;
     clock->seconds = rand()%(t-1) + 1;
    

    
    // fork a child process
    pid_t pid = fork();
    
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
        
    }else if(pid == 0){
        //child process
        printf("Child process: seconds = %ld, nanoseconds = %ld\n", clock->seconds, clock->nanoseconds);
        char sec_str[10];
        char nansec_str[10];
        sprintf(sec_str, "%ld", clock->seconds);
        sprintf(nansec_str, "%ld", clock->nanoseconds);
        execl("./worker", "worker",sec_str,nansec_str,NULL);
        exit(EXIT_SUCCESS);
    } else {
        // Parent processand
        wait(NULL);
        printf("Parent process: simulated system clock after child process: seconds = %ld, nanoseconds = %ld\n", clock->seconds, clock->nanoseconds);
        }
    
    
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
    
    
    
    
    
    
    
    return 0;
}
