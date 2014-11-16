/*
 Names: Cindy Han, Junghyun Seo
 Cosc 301
 Proj 04
 11/17/2014 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ucontext.h>
#include <pthread.h>
#include "threadsalive.h"

/****************************** 
     stage 1 library functions
   ******************************/
// scheduling queue
static thread_node **threads;
static ucontext_t main_t; // store the main thread seperately 
static int current, count, size;

#define STACKSIZE 32768

static void array_resize() {
	// resizes an array of contexts
	thread_node **new = malloc(size*2*sizeof(thread_node *)); //double the size of the array

	// copy over all the existing threads
	for (int i = 0; i < size; i++){
		new[i] = threads[i];
	}
	thread_node **temp = threads;
	threads = new;
	free(temp); // free old array
	size = size*2;
}


//initializes the thread library
void ta_libinit(void) {
	// set up the main thread in the library
	getcontext(&main_t);
	threads = malloc(32*sizeof(thread_node *)); // start with 32 item array
	current = 0;
	count = 0; // current # of threads
	size = 32; // size of the array
   return;
}

//creates a new thread
void ta_create(void (*func)(void *), void *arg) {
	// resize the array if needed
	if (count == size - 1){
		array_resize();
	}
	unsigned char *stack = (unsigned char *)malloc(STACKSIZE);
	assert(stack);
	threads[count] = malloc(sizeof(thread_node *));
	threads[count]->flag = 0;
	threads[count]->block = 0;
	getcontext(&threads[count]->ctx); // initial context for thread
	// set up the thread stack
	threads[count]->ctx.uc_stack.ss_sp = stack;
	threads[count]->ctx.uc_stack.ss_size = STACKSIZE;
	// set up the link to the main thread
	threads[count]->ctx.uc_link = &main_t;
	// set thread entry point
	makecontext(&threads[count]->ctx, (void(*)(void))*func, 1, arg);
	// update count
	count++;
    return;
}

static int find_next(){
	// returns the index of the next runable thread
	int i = (current + 1) % count;
	for (int n = 0; n < count; n++){
		if (threads[i]->flag == 0 && threads[i]->block == 0){ // check if the thread is done
			return i;
		}
		i = (i + 1) % count; 
	}
	return -1; // return -1 if all threads are done
}
 
//yields CPU from current thread to the next runnable thread 
void ta_yield(void) {
	int curr = current;
	current = find_next();
	if (current == -1){
		return;
	}
	// switch to the next ready thread
	swapcontext(&threads[curr]->ctx, &threads[current]->ctx);
  	return;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
int ta_waitall(void) {
	// run all ready threads
	do {
		// switch to next thread
		swapcontext(&main_t, &threads[current]->ctx);
		// set the current node's flag to done
		threads[current]->flag = 1;
		// update current
		current = find_next();
	} while (current != -1);
	// free the queue
	free(threads);
	if (current == -1){
		return 0;
	}
	return -1;
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
	sem = malloc(sizeof(tasem_t)); // allocate space for the semaphore
	sem->value = value; 
	sem->num_blocked = 0;
	sem->current = 0;
	sem->blocked = malloc(sizeof(thread_node)*32);
}

void ta_sem_destroy(tasem_t *sem) {
	free(sem->blocked);
	free(sem); // free the space
	
}

void ta_sem_post(tasem_t *sem) {
	sem->value++;
	// release a blocked thread
	if (sem->num_blocked > 0){
		sem->blocked[current]->block = 0;
		sem->current++;
	}
}

void ta_sem_wait(tasem_t *sem) {
	if (sem->value == 0){ 
		sem->blocked[sem->num_blocked] = threads[current];
		threads[current]->block = 1;
		ta_yield();
	}
	sem->value--;
}

void ta_lock_init(talock_t *mutex) {
	ta_sem_init(mutex->sem, 1);
}

void ta_lock_destroy(talock_t *mutex) {
	ta_sem_destroy(mutex->sem);
}



void ta_lock(talock_t *mutex) {
	ta_sem_wait(mutex->sem);
}

void ta_unlock(talock_t *mutex) {
	ta_sem_post(mutex->sem);
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

