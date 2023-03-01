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
    struct timespec term_time;
    term_time.tv_sec = sys_clock->tv_sec + seconds;
    term_time.tv_nsec = sys_clock->tv_nsec + nanoseconds;
    if (term_time.tv_nsec >= 1000000000) {
        term_time.tv_sec++;
        term_time.tv_nsec -= 1000000000;
    }
    
    //print initial information
    printf("WORKER PID:%d PPID:%d SysClockS: %ld SysClockNano: %ld TermTimeS: %ld TermTimeNano: %ld\n",
               getpid(), getppid(), sys_clock->tv_sec, sys_clock->tv_nsec, term_time.tv_sec, term_time.tv_nsec);
    printf("--Just Starting\n");
        
    // Enter loop, checking system clock periodically
    while (true) {
        // Get current system clock time
        struct timespec cur_time;
        clock_gettime(CLOCK_REALTIME, &cur_time);

        // Check if it's time to terminate
        if (cur_time.tv_sec > term_time.tv_sec ||
            (cur_time.tv_sec == term_time.tv_sec && cur_time.tv_nsec >= term_time.tv_nsec)) {
            printf("WORKER PID:%d PPID:%d SysClockS: %ld SysClockNano: %ld TermTimeS: %ld TermTimeNano: %ld\n",
                    getpid(), getppid(), cur_time.tv_sec, cur_time.tv_nsec, term_time.tv_sec, term_time.tv_nsec);
                printf("--Terminating\n");
                break;
            }
    }
    return 0;
}
