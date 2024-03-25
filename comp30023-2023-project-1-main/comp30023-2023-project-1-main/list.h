#ifndef __LIST__
#define __LIST__

#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "proc.h"

typedef struct node node_t;


struct node {
	Process_t process;
	node_t *next;
};

typedef struct {
	node_t *head;
	node_t *foot;
} list_t;



list_t *make_empty_list(void);

int is_empty_list(list_t *list);

void free_list(list_t *list);

list_t *insert_at_head(list_t *list, Process_t process);

list_t *insert_at_foot(list_t *list, Process_t process);

Process_t get_head(list_t *list);

list_t *get_tail(list_t *list);

node_t *q_delete(list_t* list, Process_t value);

node_t *dequeue(list_t *list);

void printNode(node_t *node);

void printList(list_t* list);

#endif