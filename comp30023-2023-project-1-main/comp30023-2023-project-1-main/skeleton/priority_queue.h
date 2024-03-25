// Adapted from COMP20003 2021 Assignment 3

#ifndef __PQ__
#define __PQ__

/**
 * NIR: Adapted from https://gist.github.com/aatishnn/8265656#file-binarymaxheap-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include "node.h"
#include "../../include/sokoban.h"


/**
 * size is the allocated size, count is the number of elements in the queue
 */
struct node_s{
    int priority;
    int depth;
    int num_childs;
    move_t move;
    state_t state;
    struct node_s* parent;
};

typedef struct node_s node_t;

struct heap {
	int size;
	int count;
	node_t** heaparr;
};

#define initial_size  4

void heap_init(struct heap* h);

void max_heapify(node_t** data, int loc, int count);

void heap_push(struct heap* h, node_t* value);

void heap_display(struct heap *h, sokoban_t *init_data);

node_t* heap_delete(struct heap* h);

// added "sokoban_t* init_data" argument
void emptyPQ(struct heap* pq, sokoban_t* init_data);

#endif