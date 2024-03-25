#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "pq.h"
#include "list.h"
#include "proc.h"



int main(int argc, char* argv[]) {
   FILE *fp;
   Process_t process;
   Process_t* processes;
   int ArrSize = INITIAL;
   int num = 1;
   int i = 0;

   list_t *inputq;
   list_t *roundrobinq;
   struct heap* readyq;

   inputq = make_empty_list();
   roundrobinq = make_empty_list();
   readyq = malloc(sizeof(struct heap));
   heap_init(readyq);
    
   processes = make_processes();

   char *filename = NULL;
   char *schedAlgo = NULL;
   char *memoryAlgo = NULL;
   int quantum = 0;    
   int opt;
   
   // While loop below adapted from https://www.geeksforgeeks.org/getopt-function-in-c-to-parse-command-line-arguments/
   while((opt = getopt(argc, argv, "f:s:m:q:")) != -1) {

        switch(opt) 
        { 
            case 'f': 
                filename = optarg;
                break; 
            case 's': 
                schedAlgo = optarg;
                break; 
            case 'm': 
                memoryAlgo = optarg;
                break; 
            case 'q': 
                quantum = atoi(optarg);
                break; 
        } 
    } 
      
   if ((fp = fopen(filename,"r")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       exit(1);
    }

    // read in file
    while((fscanf(fp, "%d %s %d %d", &process.arrivalTime, process.name,&process.serviceTime, &process.memory) != EOF)){
        if(num  == ArrSize){
            ArrSize *= 2;
            processes = realloc(processes, sizeof(Process_t) * ArrSize);
            assert(processes);
        }

        processes[i].arrivalTime = process.arrivalTime;
        strcpy(processes[i].name, process.name);
        processes[i].serviceTime = process.serviceTime;
        processes[i].memory = process.memory;

        //initialise timeRan for each process
        processes[i].timeRan = 0;

        num++;
        i++;
    }
   fclose(fp); 


    /* --------------------------main program execution-----------------*/
    Process_t *currentRunningProcess = NULL;
    int cycle = 0;
    int simulationTime = cycle * quantum;

    int arraycounter = 0;
    int inputq_counter = 0;
    int readyq_counter = 0;
    int curr_counter = 0;
    double turnaroundTime;
    double turnaroundTime_total = 0;
    double overheads[i];
    double overheadTime;
    double overheadTime_total = 0;
    int index = 0;
    // infinite memory meethod
    if(strcmp(schedAlgo,"SJF") == 0){
        while(arraycounter < (i) || !is_empty_list(inputq) || readyq->count != 0 || currentRunningProcess != NULL){

                // check current process is active
            if(currentRunningProcess != NULL){
                if(currentRunningProcess->timeRan >= currentRunningProcess->serviceTime){
                    //print FINISHED
                    printf("%d, FINISHED, process_name = %s, proc_remaining = %d\n",simulationTime, currentRunningProcess->name, inputq_counter+readyq_counter);
                    turnaroundTime = simulationTime - currentRunningProcess->arrivalTime;
                    turnaroundTime_total += turnaroundTime;

                    overheadTime = turnaroundTime / currentRunningProcess->serviceTime;
                    overheadTime_total += overheadTime;
                    overheads[index] = overheadTime;
                    index++;
                    currentRunningProcess = NULL;
                }

            }

            // add process to inputq based on arrival time from array
            while(arraycounter < i && processes[arraycounter].arrivalTime <= simulationTime){
                inputq = insert_at_head(inputq, processes[arraycounter]);

                arraycounter++;
                inputq_counter++;
            }
            // add process from inputq to ready, check for memory
            while(!is_empty_list(inputq)){
                node_t *ready = dequeue(inputq);
                inputq_counter--;

                //print READY
                if(strcmp(memoryAlgo,"best-fit")==0){
                    printf("%d, READY, process_name = %s\n",simulationTime, ready->process.name);
                }                    
                heap_push(readyq,&(ready->process));
                readyq_counter++;
            }

            //TASK 1 SJF
            // readyq not empty and there's no currentRunningProcess

            if(currentRunningProcess == NULL && readyq->count != 0){
                    currentRunningProcess = heap_delete(readyq);
                readyq_counter--;
                curr_counter++;
                printf("%d, RUNNING, process_name = %s, remaining_time = %d\n",simulationTime, currentRunningProcess->name, currentRunningProcess->serviceTime-currentRunningProcess->timeRan);


            }

            if(currentRunningProcess != NULL){
                currentRunningProcess->timeRan += quantum;
            }




            simulationTime += quantum;
            cycle++;
        }
    }

    if(strcmp(schedAlgo,"RR") == 0){
        while(arraycounter < (i) || !is_empty_list(inputq) || !is_empty_list(roundrobinq) || currentRunningProcess != NULL){
            // check current process is active
            if(currentRunningProcess != NULL){

                if(currentRunningProcess->timeRan >= currentRunningProcess->serviceTime){
                    //print FINISHED
                    printf("%d, FINISHED, process_name = %s, proc_remaining = %d\n",simulationTime, currentRunningProcess->name, inputq_counter+readyq_counter);
                    turnaroundTime = simulationTime - currentRunningProcess->arrivalTime;
                    turnaroundTime_total += turnaroundTime;
                
                    overheadTime = turnaroundTime / currentRunningProcess->serviceTime;
                    overheadTime_total += overheadTime;
                    overheads[index] = overheadTime;
                    index++;
                    currentRunningProcess = NULL;
                }
                // not finished running and queue not empty, add to back of queue
            }
            // if( !is_empty_list(roundrobinq) && currentRunningProcess->timeRan < currentRunningProcess->serviceTime){
            //     insert_at_head(roundrobinq,*currentRunningProcess);
            //     currentRunningProcess = NULL;
            // }
                // // not finished and queue empty
                // if(currentRunningProcess->timeRan < currentRunningProcess->serviceTime && is_empty_list(roundrobinq)){
                //     continue;
                // }
                
            // }

            // add process to inputq based on arrival time from array
            while(arraycounter < i && processes[arraycounter].arrivalTime <= simulationTime){
                inputq = insert_at_head(inputq, processes[arraycounter]);
            
                arraycounter++;
                inputq_counter++;
            }
            // add process from inputq to ready, check for memory
            while(!is_empty_list(inputq)){
                node_t *ready = dequeue(inputq);
                inputq_counter--;
                //print READY
                if(strcmp(memoryAlgo,"best-fit")==0){
                printf("%d, READY, process_name = %s\n",simulationTime, ready->process.name);
                }
                roundrobinq = insert_at_head(roundrobinq, ready->process);
                readyq_counter++;
            }

            //TASK 2 RR
            // readyq not empty and there's no currentRunningProcess
        
            if(currentRunningProcess == NULL && !is_empty_list(roundrobinq)){
                currentRunningProcess = &(dequeue(roundrobinq)->process);
                readyq_counter--;
                curr_counter++;
                printf("%d, RUNNING, process_name = %s, remaining_time = %d\n",simulationTime, currentRunningProcess->name, currentRunningProcess->serviceTime-currentRunningProcess->timeRan);
            
            }

            if(currentRunningProcess != NULL){
                currentRunningProcess->timeRan += quantum;
            }
            // not finished running and queue not empty, add to back of queue
            if( !is_empty_list(roundrobinq) && currentRunningProcess->timeRan <= currentRunningProcess->serviceTime){
                insert_at_head(roundrobinq,*currentRunningProcess);
                readyq_counter++;
                currentRunningProcess = NULL;
                // currentRunningProcess = &(dequeue(roundrobinq)->process);
            }
                // not finished and queue empty
            // if(currentRunningProcess->timeRan < currentRunningProcess->serviceTime && is_empty_list(roundrobinq)){
            //     continue;
            // }

            simulationTime += quantum;
            cycle++;
        }
    }
    
    //Task 5.2
    double max_overhead = 0;
    for(int j = 0; j < index; j++){
        if(overheads[j]> max_overhead){
            max_overhead = overheads[j];
        }
    }
    int ave_turnaroundTime = (int) round(turnaroundTime_total/(arraycounter));
    double ave_overheadTime = overheadTime_total/arraycounter;
    printf("Turnaround time %d\n", ave_turnaroundTime);
    printf("Time overhead %.2f %.2f\n",max_overhead, ave_overheadTime);
    printf("Makespan %d\n", simulationTime-quantum);

    free_process(processes, ArrSize);
    processes = NULL;
    free_list(inputq);
    inputq = NULL;
    free(readyq->heaparr);
    free(readyq);
    readyq = NULL;
    return 0;
}