#include "proc.h"

// init array of processes
Process_t* make_processes(void) {
    Process_t* process;
    process = calloc(INITIAL, sizeof(Process_t));
    assert(process);
    return process;
}



void printProcess(Process_t process){
        printf("%d ", process.arrivalTime);
        printf("%s ", process.name);
        printf("%d ", process.serviceTime);
        printf("%d ", process.memory);
        printf("\n");
}

void free_process(Process_t* process, int read_size){
    for(int i = 0; i < read_size; i++){
    }
    free(process);
}