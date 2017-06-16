#ifndef _MY_PTHREAD_H
#define _MY_PTHREAD_H

#include <stdio.h>
#include <unistd.h>
//#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>
#include "queue.h"
#include "my_malloc.h"
/**
 * My pthread library
 *
 */

#define MAXTHREADS 10
// 1MB
#define STACKSIZE (1024*1024)

#define DEBUG_PRINT printf("@%s:%d\n", __func__, __LINE__);
//#define DEBUG_PRINT

typedef enum {
	READY = 0,
	WAITING = 1,
	RUNNING = 2,
	BLOCKED = 3,
	DONE = 4,
	DEFAULT = 5
} T_STATUS;

// pthread_t struct
typedef struct thread {
	unsigned int tid;
	unsigned int timeQuantum;
	ucontext_t context;
	T_STATUS status;
	struct timeval now;
	struct timezone tz;
	unsigned int priority;
	void* stack;
	unsigned int isActive;
} my_pthread_t;

#define pthread_t my_pthread_t

// Main context
ucontext_t mainContext;

// Higher priority queue
queue_t q1;

// Lower priority queue
queue_t q2;

// Wait queue
queue_t w1;

typedef struct
{
	int volatile value;
} my_pthread_mutex_t;

/**
 * Creates a pthread that executes function. Attributes are ignored.
 */
int my_pthread_create(pthread_t* thread, pthread_attr_t* attr, 
		void(*function)(void), void* arg);

/**
 * Explicit call to the my_pthread_t scheduler requesting that the current context 
 * be swapped out and another be scheduled.
 */
void my_pthread_yield();

/**
 * Explicit call to the my_pthread_t library to end the pthread that called it.
 * If the value_ptr isn't NULL, any return value from the thread will be saved.
 */
void pthread_exit(void *value_ptr);

/**
 * Call to the my_pthread_t library ensuring that the calling thread will not execute 
 * until the one it references exits. If value_ptr is not null, the return value of the 
 * exiting thread will be passed back.
 */
int my_pthread_join(pthread_t thread, void **value_ptr);


void my_pthread_init();
void my_timer_init();

/* Mutex functions */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex);

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);


#endif // _MY_PTHREAD_H
