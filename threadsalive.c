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
static thread *threads;
static ucontext_t main_t; // store the main thread seperately 
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
	free(temp); // free old array
	current = 0;
	size = size*2;
}

/*
//set the flag to 1 when the thread starts running, sets it back to 0 when done
//this is in order to track if the thread is running or not
void tflag_bool(void(*func)(void)){
	threads[insert_idx].flag = 1;
	func(void);
	threads[insert_idx].flag = 0;
}
*/

//initializes the thread library
void ta_libinit(void) {
	// set up the main thread in the library
	getcontext(&main);
	threads = malloc(32*sizeof(ucontext_t)); // start withh 32 item array
	current = 0;
	count = 0; // current # of threads
	size = 32; // size of the array
   return;
}

//creates a new thread
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
	// set up the link to the main thread
	threads[insert_idx].uc_link = &main;
	// set thread entry point
	makecontext(&threads[insert_idx], (void(*)(void))*func, 1, arg);
	// update count
	count++;
    return;
}

//yields CPU from current thread to the next runnable thread 
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

//shorter(?) ta_waitall(void) -- lemme know what you think
int ta_waitall(void){
	while(count != 0){
		ta_yield(void);
	}
	if (count == 0){
		return 0;
	}
	return -1;
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
	sem = malloc(sizeof(tasem_t));
	sem->counter = value; 
	pthread_mutex_init(&sem->mutex);
}

void ta_sem_destroy(tasem_t *sem) {
	free(sem);
}

void ta_sem_post(tasem_t *sem) {
	pthread_mutex_lock(&sem->mutex);
	sem->counter++;
	pthread_mutex_unlock(&sem->mutex);
}

void ta_sem_wait(tasem_t *sem) {
	pthread_mutex_lock(&sem->mutex);
	if(sem->counter > 0){
		sem->counter--;
	}
	else if(sem->counter==0){
		ta_yield(void);
	}
	pthread_mutex_unlock(&sem->mutex);
}

void ta_lock_init(talock_t *mutex) {
	mutex = malloc(sizeof(talock_t));
	mutex->flag = 0;
	mutex->guard = 0;
	queue_init(&mutex->queue);
}

void ta_lock_destroy(talock_t *mutex) {
	free(mutex);
}

void ta_lock(talock_t *mutex) {
	//pthread_mutex_lock(&mutex);  ????????????
	while(TestAndSet(&mutex->guard,1) == 1)
	{/*spin*/}
	
	if(mutex->flag == 0){
		mutex->flag = 1;
		mutex->guard = 0;
	}else{
			queue_add(mutex->queue, threadself());
			setpark();
			mutex_guard = 0;
			//gives up processor
			park();
	}
}

void ta_unlock(talock_t *mutex) {
	//pthread_mutex_unlock(mutex); ??
	while(TestAndSet(&mutex->guard,1) == 1)
	{/*spin*/}
	
	if(queue_empty(mutex->queue)){
		mutex->flag = 0;
	}else{//give flag to next thread on queue
	unpark(queue_remove(mutex->queue));
	}
	mutex->guard = 0;
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

