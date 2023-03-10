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

typedef struct{
    int seconds;
    int nanoseconds;
}Clock;


//increment the system clock
void incrementClock(Clock *clock, int increment) {

clock->nanoseconds += increment;
    if (clock->nanoseconds >= 1000000000) {
        clock->seconds++;
        clock->nanoseconds -= 1000000000;
    }
}

typedef struct {
    int occupied; // either true or false
    int pid; // process id of this child
    int startSeconds; // time when it was forked
    int startNano; // time when it was forked
}PCB;

PCB process_Table[20];

int num_processes_launched = 0;
int num_processes_terminated = 0;




// Function to add a new process to the process table
void addToProcessTable(pid_t pid, int startSeconds, int startNano) {
    for (int i = 0; i < 20; i++) {
        if (!process_Table[i].occupied) {
            process_Table[i].occupied = 1;
            process_Table[i].pid = pid;
            process_Table[i].startSeconds = startSeconds;
            process_Table[i].startNano = startNano;
            break;
        }
    }
}

// Function to update the process table when a process terminates
void updateProcessTable(pid_t pid) {
    for (int i = 0; i < 20; i++) {
        if (process_Table[i].occupied && process_Table[i].pid == pid) {
            process_Table[i].occupied = 0;
            process_Table[i].pid = 0;
            process_Table[i].startSeconds = 0;
            process_Table[i].startNano = 0;
            break;
        }
    }
}


// Function to print the process table
void printProcessTable() {
    printf("Process Table:\n");
    printf("Entry\tOccupied\tPID\tStartS\tStartN\n");
    for (int i = 0; i < 20; i++) {
        printf("%d\t%d\t\t%d\t%d\t%d\n", i, process_Table[i].occupied,
               process_Table[i].pid, process_Table[i].startSeconds, process_Table[i].startNano);
    }
    printf("\n");
}

void checkIfChildHasTerminated() {
    int pid, status;
    pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
        num_processes_terminated++;
        for (int i = 0; i < 20; i++) {
            if (process_Table[i].pid == pid) {
                process_Table[i].occupied = 0;
                break;
            }
        }
    }
}


void updatePCBOfTerminatedChild(int pid) {
    // Update process table entry for terminated child
    int i;
    for (i = 0; i < 20; i++) {
        if (process_Table[i].occupied && process_Table[i].pid == pid) {
            process_Table[i].occupied = 0;
            process_Table[i].pid = 0;
            process_Table[i].startSeconds = 0;
            process_Table[i].startNano = 0;
            break;
        }
    }
    
}

int main(int argc, char* argv[]){
  
    int n=1, s = 0, t=7;
    int children = 0;
    int option =0;
    int status;
    int stillChildrenToLaunch = 1;
    int childHasTerminated = 0;
    
    int increment = 1000000; // increment clock by 1 millisecond
    

    // Initialize process table
    for (int i = 0; i < 20; i++) {
        process_Table[i].occupied = 0;
        process_Table[i].pid = 0;
        process_Table[i].startSeconds = 0;
        process_Table[i].startNano = 0;
    }

   

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
    
    // Initialize process table
        for (int i = 0; i < 20; i++) {
            process_Table[i].occupied = 0;
        }
    
    while(stillChildrenToLaunch){
        incrementClock(clock, increment);
        static int counter = 0;
                if (counter == 5) {
                    counter = 0;
                    printf("OSS PID:%d SysClockS: %ld SysclockNano: %ld\n", getpid(), time(NULL), 0L);
                    printProcessTable();
                } else {
                    counter++;
                }
        checkIfChildHasTerminated();
        
        if(childHasTerminated){
            // Update PCB of terminated child
            updatePCBOfTerminatedChild( pid);
                
    
   
    while (children < n) {
        if (children >= s) {
            wait(NULL);
        }
        for(int i=0; i<20; i++){
            if (!process_Table[i].occupied) {
                
            
    // fork a child process
    pid_t pid = fork();
    
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
        
    }else if(pid == 0){
        //child process
        printf("Child process: seconds = %d, nanoseconds = %d\n", clock->seconds, clock->nanoseconds);
        
        char sec_str[10];
        char nansec_str[10];
        sprintf(sec_str, "%d", clock->seconds);
        sprintf(nansec_str, "%d", clock->nanoseconds);
        
        execl("./worker", "worker",sec_str,nansec_str,NULL);
        
        exit(EXIT_SUCCESS);
    } else {
        // Parent processand
        wait(NULL);
        printf("Parent process: simulated system clock after child process: seconds = %d, nanoseconds = %d\n", clock->seconds, clock->nanoseconds);
        
        
        addToProcessTable(pid, clock->seconds, clock->nanoseconds);
        process_Table[i].occupied = 1;
        process_Table[i].pid = pid;
        process_Table[i].startSeconds = time(NULL);
        process_Table[i].startNano = 0;
        num_processes_launched++;
        break;
        }
    
            }else{
                stillChildrenToLaunch = 0;
            }
    
   
    
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
        }
        }
    }
    return 0;
}
