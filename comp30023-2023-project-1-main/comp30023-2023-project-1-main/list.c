#include "list.h"

list_t *make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;
	return list;
}

int is_empty_list(list_t *list) {
	assert(list!=NULL);
	return list->head==NULL;
}

void free_list(list_t *list) {
	node_t *curr, *prev;
	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
        free(prev);
	}
	free(list);
}

list_t *insert_at_head(list_t *list, Process_t process) {
	node_t *new;
	new = (node_t*)malloc(sizeof(node_t));
	assert(list!=NULL && new!=NULL);
	new->process = process;
	new->next = list->head;
	list->head = new;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->foot = new;
	}
	return list;
}

list_t *insert_at_foot(list_t *list, Process_t process) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	new->process = process;
	new->next = NULL;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		list->foot->next = new;
		list->foot = new;
	}
	return list;
}

Process_t get_head(list_t *list) {
	assert(list!=NULL && list->head!=NULL);
	return list->head->process;
}

list_t *get_tail(list_t *list) {
	node_t *oldhead;
	assert(list!=NULL && list->head!=NULL);
	oldhead = list->head;
	list->head = list->head->next;
	if (list->head==NULL) {
		/* the only list node just got deleted */
		list->foot = NULL;
	}
	free(oldhead);
	return list;
}

node_t *dequeue(list_t *list){
    node_t *res = list->head;
    
    list->head = list->head->next;
	/* Clean up memory we allocated. */
	// free(oldQ);
	
    return res;
}

node_t *q_delete(list_t* list, Process_t value){
    if (list == NULL){
        printf("list is null\n");
        exit(1);
    }
    if (list->head == NULL){
        printf("head is null\n");
        exit(1);
    }

    node_t *temp = list->head;
    node_t *prev = NULL;

    // Traverse the linked list to find the node to remove
    while(temp != NULL && strcmp(temp->process.name,value.name)!=0){
        prev = temp;
        temp = temp->next;
    }
    // If the node was not found, return
    if(temp == NULL){
        printf("node not found\n");
        exit(1);
    }
    // Update the "next" pointer of the previous node to skip the node to be removed
    if(prev != NULL){
        prev->next = temp->next;
    }else{
        // If the node to be removed is the head of the list, update the head pointer
        list->head = temp->next;
    }
    return temp;
}

void printList(list_t* list){
    node_t* node = list->head;
    while (node != NULL) {
        printf("%d ", node->process.arrivalTime);
        printf("%s ", node->process.name);
        printf("%d ", node->process.serviceTime);
        printf("%d ", node->process.memory);
        printf("\n");
        node = node->next;
    }
    printf("\n");
}

void printNode(node_t *node){
    if(node == NULL){
        printf("node is NULL");
        exit(1);
    }
    printf("%d ", node->process.arrivalTime);
    printf("%s ", node->process.name);
    printf("%d ", node->process.serviceTime);
    printf("%d ", node->process.memory);
    printf("\n");
}