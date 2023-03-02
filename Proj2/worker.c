//
//  worker.c
//  Proj2
//
//  Created by Erosh Boralugodage on 2/15/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>

#define SHM_KEY 0666


int main(int argc, char* argv[]){
    
    printf("Now at worker\n");
    
    if (argc != 3){
        printf("Usage: %s seconds nanoseconds \n ", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // parsing command line arguments
    long seconds = atol(argv[1]);
    long nanoseconds = atol(argv[2]);
    
    // attach to shared memmory
    int share_memid = shmget(SHM_KEY, sizeof(struct timespec), 0666|IPC_CREAT);
    if(share_memid == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    struct timespec *sys_clock = shmat(share_memid, NULL,0);
    if(sys_clock == (struct timespec *) -1){
        perror("shmat");
        exit(EXIT_FAILURE);
        
    }
    
    //calculate target termination time
    //This program use clock_gettime() function to get system time
    //then calculates the target termination time based on the command-line arguments.
    // It then enters a loop where it repeatedly checks the current system time and compares it to the target termination time.
    //If the current time is greater than or equal to the target termination time, the program outputs a message and exits.
    
    
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    
    //calulate the target termination time
    struct timespec term_time;
    
    term_time.tv_sec = (start_time.tv_sec/1000000000 )+ seconds;
    term_time.tv_nsec = (start_time.tv_sec%1000000000) + nanoseconds;
   
    
    if (term_time.tv_nsec >= 1000000000) {
        term_time.tv_sec++;
        term_time.tv_nsec -= 1000000000;
    }
    
    //print initial information
    printf("WORKER PID:%d PPID:%d SysClockS: %ld SysClockNano: %ld TermTimeS: %ld TermTimeNano: %ld\n",
               getpid(), getppid(), seconds, nanoseconds, term_time.tv_sec, term_time.tv_nsec);
    printf("--Just Starting\n");
        
    
    
    // Enter loop, checking system clock periodically
    while (true) {
        // Get current system clock time
        struct timespec cur_time;
        clock_gettime(CLOCK_REALTIME, &cur_time);
        
        cur_time.tv_sec = seconds + cur_time.tv_sec/1000000000;
        cur_time.tv_nsec = nanoseconds + cur_time.tv_nsec%1000000000;

        // Check if it's time to terminate
        if (cur_time.tv_sec > term_time.tv_sec ||
            (cur_time.tv_sec == term_time.tv_sec && cur_time.tv_nsec >= term_time.tv_nsec)) {
            printf("WORKER PID:%d PPID:%d SysClockS: %ld SysClockNano: %ld TermTimeS: %ld TermTimeNano: %ld\n",
                    getpid(), getppid(), cur_time.tv_sec, cur_time.tv_nsec, term_time.tv_sec, term_time.tv_nsec);
            exit(EXIT_SUCCESS);
                
            }
        
        // Check if a second has passed
        if (cur_time.tv_sec > start_time.tv_sec) {
            printf("WORKER PID:%d PPID:%d SysClockS:%ld SysclockNano:%ld TermTimeS:%ld TermTimeNano:%ld\n",
                   getpid(), getppid(), cur_time.tv_sec, cur_time.tv_nsec, term_time.tv_sec, term_time.tv_nsec);
            printf("--%ld seconds have passed since starting code in C\n", cur_time.tv_sec - start_time.tv_sec);
            start_time = cur_time;
        }
        // Sleep for a short period to avoid using too much CPU
        usleep(10000); // Sleep for 10 milliseconds
    }
    return 0;
}
