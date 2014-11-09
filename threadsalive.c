/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ucontext.h>

#include "threadsalive.h"

/****************************** 
     stage 1 library functions
   ******************************/
// scheduling queue
static thread_queue *queue;
#define STACKSIZE 8192
void ta_libinit(void) {
	// initialize the queue
	queue = malloc(sizeof(queue));
	node *n = malloc(sizeof(node));
	n->next = NULL;
	// set up the main thread in the library
	swapcontext(&n->thread, &n->thread);
	// set up the queue
	queue->head = n;
	queue->tail = n;
	queue->size = 1;
    return;
}

void ta_create(void (*func)(void *), void *arg) {
	// make a new node
	unsigned char *stack = (unsigned char *)malloc(STACKSIZE);
	assert(stack);
	node *n = malloc(sizeof(node));
	getcontext(&n->thread); // initial context for thread
	// set up the thread stack
	n->thread.uc_stack.ss_sp = stack;
	n->thread.uc_stack.ss_size = STACKSIZE;
	// set thread entry point
	makecontext(&n->thread, (void(*)(void))*func, 1, arg);
	// add the first thread to the queue
	if (queue->size == 1){
		// set up the link to the main thread
		n->thread.uc_link = &queue->head->thread;
		n->next = queue->head;
		// update head and tail
		queue->head = n;
		queue->tail = n;
		// update size
		queue->size++;
	}
	else{
		// place the thread at the end of the ready queue
		n->thread.uc_link = &queue->tail->next->thread;
		n->next = queue->tail->next;
		queue->tail->thread.uc_link = &n->thread;
		queue->tail->next = n;
		// update tail and size
		queue->tail = n;
		queue->size++;
	}
    return;
}

void ta_yield(void) {
	// put current thread in a new node
	node *n = malloc(sizeof(node));
	// place the thread at the end of the ready queue
	n->thread.uc_link = &queue->tail->next->thread;
	n->next = queue->tail->next;
	queue->tail->thread.uc_link = &n->thread;
	queue->tail->next = n;
	// update tail and size
	queue->tail = n;
	queue->size++;
	// switch to the next ready thread
	swapcontext(&n->thread, &queue->head->thread);
	// update the head and size
	node *temp = queue->head;
	queue->head = queue->head->next;
	queue->size--;
	free(temp); // free the node
    return;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
int ta_waitall(void) {
	// run all ready threads
	while (queue->size >= 1) {
		// switch to the next ready thread
		swapcontext(&queue->tail->next->thread, &queue->head->thread);
		node *temp = queue->head;
		queue->head = queue->head->next;
		queue->size--;
		free(temp); // free the node
	}
	// free the last node
	free(queue->head);
	// free the queue
	free(queue);
	return 0;
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
}

void ta_sem_destroy(tasem_t *sem) {
}

void ta_sem_post(tasem_t *sem) {
}

void ta_sem_wait(tasem_t *sem) {
}

void ta_lock_init(talock_t *mutex) {
}

void ta_lock_destroy(talock_t *mutex) {
}

void ta_lock(talock_t *mutex) {
}

void ta_unlock(talock_t *mutex) {
}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
}

void ta_cond_destroy(tacond_t *cond) {
}

void ta_wait(talock_t *mutex, tacond_t *cond) {
}

void ta_signal(tacond_t *cond) {
}

