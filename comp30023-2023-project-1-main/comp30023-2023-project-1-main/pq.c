#include "pq.h"

int *heap, size, count;

void heap_init(struct heap* h)
{
	h->count = 0;
	h->size = initial_size;
	h->heaparr = (Process_t **) malloc(sizeof(Process_t*) * initial_size);
	for(int i = 0; i < initial_size; i++)
	    h->heaparr[i]=NULL;
	
	if(!h->heaparr) {
		printf("Error allocatinga memory...\n");
		exit(-1);
	}

}

void max_heapify(Process_t** data, int loc, int count) {
	int left, right, largest;
	Process_t* temp;
	left = 2*(loc) + 1;
	right = left + 1;
	largest = loc;
	

	if (left <= count && SJFcompare(data[left],data[largest])) {

		largest = left;
	} 
	if (right <= count && SJFcompare(data[left],data[largest])) {

		largest = right;
	} 
	
	if(largest != loc) {
		temp = data[loc];
		data[loc] = data[largest];
		data[largest] = temp;
		max_heapify(data, largest, count);
	}

}

void heap_push(struct heap* h, Process_t* value)
{
	int index, parent;
 
	// Resize the heap if it is too small to hold all the data
	if (h->count == h->size)
	{
		h->size += 1;
		h->heaparr = realloc(h->heaparr, sizeof(node_t) * h->size);
		if (!h->heaparr) exit(-1); // Exit if the memory allocation fails
	}
 	
 	index = h->count++; // First insert at last of array

 	// Find out where to put the element and put it
	for(;index; index = parent)
	{
		parent = (index - 1) / 2;
        if(SJFcompare(h->heaparr[parent], value))break;
		h->heaparr[index] = h->heaparr[parent];
	}
	h->heaparr[index] = value;
}

int SJFcompare(Process_t *process1, Process_t *process2){
    if(process1->serviceTime < process2->serviceTime){
        return 1;
    }

    if(process1->serviceTime == process2->serviceTime && process1->arrivalTime < process2->arrivalTime){
        return 1;
    }

    if(process1->arrivalTime == process2->arrivalTime && process1->name < process2->name){
        return 1;
    }
    return 0;
}

void heap_display(struct heap* h) {
	int i;
	for(i=0; i < h->count; ++i) {
	    Process_t* n = h->heaparr[i];
        printf("%d ", n->arrivalTime);
        printf("%s ", n->name);
        printf("%d ", n->serviceTime);
        printf("%d ", n->memory);
        printf("\n");
	}
}

Process_t* heap_delete(struct heap* h)
{
	Process_t* removed;
	Process_t* temp = h->heaparr[--h->count];
 	
	
	if ((h->count <= (h->size + 2)) && (h->size > initial_size))
	{
		h->size -= 1;
		h->heaparr = realloc(h->heaparr, sizeof(Process_t) * h->size);
		if (!h->heaparr) exit(-1); // Exit if the memory allocation fails
	}
 	removed = h->heaparr[0];
 	h->heaparr[0] = temp;
	if(temp == removed) h->heaparr[0] = NULL;
 	max_heapify(h->heaparr, 0, h->count);
 	return removed;
}
