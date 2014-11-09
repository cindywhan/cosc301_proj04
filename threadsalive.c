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
static ucontext_t *threads;
static ucontext_t main; // store the main thread seperately 
static int current, count, size;

#define STACKSIZE 8192

void array_resize() {
	// resizes an array of contexts
	ucontext_t *new = malloc(size*2*sizeof(ucontext_t)); //double the size of the array
	// copy over all the existing threads
	for (int i = 0; i < size; i++){
		new[i] = threads[current];
		current = (current + 1) % size;
	}
	ucontext_t *temp = threads;
	threads = new;
	free(temp); // free old array;
	current = 0;
	size = size*2;
}

void ta_libinit(void) {
	// set up the main thread in the library
	getcontext(&main);
	threads = malloc(32*sizeof(ucontext_t)); // start withh 32 item array
	current = 0;
	count = 0; // current # of threads
	size = 32; // size of the array
    return;
}

void ta_create(void (*func)(void *), void *arg) {
	// resize the array if needed
	if (count == size){
		array_resize();
	}
	unsigned char *stack = (unsigned char *)malloc(STACKSIZE);
	assert(stack);
	int insert_idx = (current + count + 1) % size;
	getcontext(&threads[insert_idx]); // initial context for thread
	// set up the thread stack
	threads[insert_idx].uc_stack.ss_sp = stack;
	threads[insert_idx].uc_stack.ss_size = STACKSIZE;
	// set thread entry point
	makecontext(&threads[insert_idx], (void(*)(void))*func, 1, arg);
	// set up the link to the main thread
	threads[insert_idx].uc_link = &main;
	// update count
	count++;
    return;
}

void ta_yield(void) {
	// put self at the end of the queue
	if (count == size){
		// resize if needed
		array_resize();
	}
	int curr = current;
	current = (current + 1) % size;
	// switch to the next ready thread
	swapcontext(&threads[curr], &threads[current]);
    return;
} 
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
int ta_waitall(void) {
	printf("count: %d, size: %d, nextout: %d. \n", count, size, current);
	// run all ready threads
	for (int i = 0; i < count; i++) {
		// switch to next thread
		swapcontext(&main, &threads[current]);
		current = (current + 1) % size;
	}
	// free the queue
	free(threads);
	if (count == 0){
		return 0;
	}
	return -1;
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

