#ifndef __PQ__
#define __PQ__

/**
 * NIR: Adapted from https://gist.github.com/aatishnn/8265656#file-binarymaxheap-c
 */

#include <stdio.h>
#include <stdlib.h>
#include "proc.h"
#include "list.h"


/**
 * size is the allocated size, count is the number of elements in the queue
 */

struct heap {
	int size;
	int count;
	Process_t** heaparr;
};

#define initial_size  4

void heap_init(struct heap* h);

void max_heapify(Process_t** data, int loc, int count);

void heap_push(struct heap* h, Process_t* value);

void heap_display(struct heap *h);

Process_t* heap_delete(struct heap* h);

int SJFcompare(Process_t *process1, Process_t *prcoess2);

#endif