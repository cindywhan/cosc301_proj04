/*
 Names: Cindy Han, Junghyun Seo
 Cosc 301
 Proj 04
 11/17/2014 
 
 
 Part 1: Cindy wrote most of part 1, Junghyun fixed the segfaults
 Part 2: Junghyun wrote the code for part 2. Cindy reworked some of the lock functions using the semaphore. Cindy and Junghyun worked together to fix the errors
 Part 3: Junghyun wrote the code and Cindy helped with the segfaults. 
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

//waits for all other threads to finish; returns 0 if all threads have completed; returns -1 if there are threads but aren't runnable
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

//creates and initializes a semaphore
void ta_sem_init(tasem_t *sem, int value) {
	sem->count = value; 
	sem->num_blocked = 0;
	sem->current = 0;
	sem->blocked = malloc(16*sizeof(thread_node *));
}

//destroys semaphore and frees
void ta_sem_destroy(tasem_t *sem) {
	free(sem->blocked);
}

//increases semaphore value atomically
void ta_sem_post(tasem_t *sem) {
	sem->count++;
	// release a blocked thread
	if (sem->num_blocked > 0){
		sem->blocked[current]->block = 0;
		sem->count++;
	}
}

void ta_sem_wait(tasem_t *sem) {
	if (sem->count == 0){ 
	//if semaphore value is zero, block and yield processor to next ready thread
		if (current != -1){
			sem->blocked[(sem->num_blocked)] = threads[current];
			threads[current]->block = 1;
		}
		ta_yield();
	}
	sem->count--; //decrements by one when value is greater than zero

}

//initializes a lock
void ta_lock_init(talock_t *mutex) {
	mutex->sem = malloc(sizeof(tasem_t));
	ta_sem_init(mutex->sem, 1);
}

//destroys and frees lock
void ta_lock_destroy(talock_t *mutex) {
	ta_sem_destroy(mutex->sem);
	free(mutex->sem);
}

//lock
void ta_lock(talock_t *mutex) {
	ta_sem_wait(mutex->sem);
}

//unlock
void ta_unlock(talock_t *mutex) {
	ta_sem_post(mutex->sem);
}

/* ***************************** 
     stage 3 library functions
   ***************************** */

//create and initialize condition variable
void ta_cond_init(tacond_t *cond) {
	cond->sem = malloc(sizeof(tasem_t));
	ta_sem_init(cond->sem, 0);
}

//destroy condition variable 
void ta_cond_destroy(tacond_t *cond) {
	ta_sem_destroy(cond->sem);
}

//wait on condition variable until another thread calls ta_signal()
void ta_wait(talock_t *mutex, tacond_t *cond) {
	ta_unlock(mutex);
	ta_sem_wait(cond->sem);
	ta_lock(mutex);
}

//wake one thread
void ta_signal(tacond_t *cond) {
	ta_sem_post(cond->sem);
}

