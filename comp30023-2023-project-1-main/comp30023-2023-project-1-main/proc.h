#ifndef __PROC__
#define __PROC__

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define INITIAL 1

typedef struct{
    int arrivalTime;
    char name[8];
    int serviceTime;
    int timeRan;
    int memory;
}Process_t;

void printProcess(Process_t process);

void free_process(Process_t *process, int read_size);

Process_t* make_processes(void);

#endif