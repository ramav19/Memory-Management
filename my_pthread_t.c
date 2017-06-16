/**
 * My pthread library
 */
#include "my_pthread_t.h"
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

// Timer interval of 50 ms
#define TIMER_INTERVAL 50

unsigned int numThreads = 0;
static pthread_t* currRunningThread = NULL;
unsigned int threadId = 0;
unsigned int currentRunningThreadId = 0;
unsigned int currQueue = 1;
unsigned int numWaitingThreads = 0;
unsigned int dummyTid = 0;

// Flag indicating if we are in a thread or main process
unsigned int inThread = 0;

void my_pthread_init()
{
	initQueue(&q1);
	initQueue(&q2);
	initQueue(&w1);
	initializeMemory();
}

void schedule_threads(int signum)
{
	DEBUG_PRINT
	my_pthread_yield();
}

void my_timer_init()
{
	struct itimerval it_val;

	if(signal(SIGALRM, (void (*)(int)) schedule_threads) == SIG_ERR)
	{
		perror("Unable to catch SIGALARM");
		exit(1);
	}

	it_val.it_value.tv_sec = TIMER_INTERVAL/1000;
	it_val.it_value.tv_usec = (TIMER_INTERVAL*1000) % 1000000;	
	it_val.it_interval = it_val.it_value;
	
	if(setitimer(ITIMER_REAL, &it_val, NULL) == -1)
	{
		perror("error calling setitimer()");
		exit(1);
	}

	while (1)
	{ 
		pause();
	}
}

void debug()
{
	DEBUG_PRINT
	int i = 0;
	for(i=0; i<10; i++)
		printf("Val: %d\n", i);
	return;
}

void wrapper_function(void(*function)(void))
{
	DEBUG_PRINT
	currRunningThread->status = RUNNING; 
	function();
	currRunningThread->status = DONE; 
	// Yield after execution is done
	my_pthread_yield();
	//swapcontext(&(((pthread_t*)(q1.head)->thread)->context), &mainContext);
	DEBUG_PRINT
}

/**
 * Creates a pthread that executes function. Attributes are ignored.
 */
int my_pthread_create(pthread_t* thread, pthread_attr_t* attr, 
		void(*function)(void), void* arg)
{
	if(numThreads == MAXTHREADS)
		return ERROR;

	queueNode_t* n = createNode();
	thread->status = READY;
	n->thread = thread;
	printf("Thread added with pointer: node:%p thread:%p\n", n, n->thread);

	// Enqueue into default high priority queue
	enque(&q1, n);
	printf("Current Q1 size: %d\n", q1.size);

	if(getcontext(&(((pthread_t*)(q1.tail)->thread)->context)) == -1)
	{
		printf("%s:%d Error: Getting context failed\n", __func__, __LINE__);
		return ERROR;
	}

	// Allocate stack etc.
	(((pthread_t*)(q1.tail)->thread)->context).uc_link = 0;
	dummyTid++;
	((pthread_t*)(q1.tail)->thread)->stack = malloc(STACKSIZE);

	//((pthread_t*)(q1.tail)->thread)->stack = myallocate(STACKSIZE, __FILE__, __LINE__, LIBRARYREQ);

	(((pthread_t*)(q1.tail)->thread)->context).uc_stack.ss_sp = ((pthread_t*)(q1.tail)->thread)->stack;
	(((pthread_t*)(q1.tail)->thread)->context).uc_stack.ss_size = STACKSIZE;
	(((pthread_t*)(q1.tail)->thread)->context).uc_stack.ss_flags = 0;

	/*
	ucontext_t temp;
	printf("GetCtx: %d @%s:%d\n",getcontext(&temp), __func__, __LINE__);
	temp.uc_link = 0;
	temp.uc_stack.ss_sp = malloc(STACKSIZE);
	temp.uc_stack.ss_size = STACKSIZE;
	temp.uc_stack.ss_flags = 0;

	makecontext(&temp, &debug, 0);
	*/

	if(!((pthread_t*)(q1.tail)->thread)->stack)
	{
		printf("%s:%d Error: Allocating thread stack failed\n", __func__, __LINE__);
		return ERROR;
	}

	((pthread_t*)(q1.tail)->thread)->status = READY;
	++threadId;
	((pthread_t*)(q1.tail)->thread)->tid = threadId;
	((pthread_t*)(q1.tail)->thread)->priority = 1;
	// Save created time
	gettimeofday(&(((pthread_t*)(q1.tail)->thread)->now), &(((pthread_t*)(q1.tail)->thread)->tz));
	// Time quantum default - 200ms
	//((pthread_t*)(q1.tail)->thread)->timeQuantum = 200;

	printf("%s:%d Creating thread with tid: %d\n", __func__, __LINE__, ((pthread_t*)(q1.tail)->thread)->tid);
	++numThreads;
	makecontext(&(((pthread_t*)(q1.tail)->thread)->context), (void (*)(void))wrapper_function, 1, function);
	return OK;
}

/**
 * Explicit call to the my_pthread_t scheduler requesting that the current context 
 * be swapped out and another be scheduled.
 */
void my_pthread_yield()
{
	DEBUG_PRINT
	// If number of threads is 0
	// Clear important pointers and return
	if(numThreads == 0)
	{
		printf("All threads are done executing, so clearing it up and returning\n");
		currRunningThread = NULL;
		exit(0);
	}

	currRunningThread = (pthread_t*)(q1.head)->thread;
	
	// If we are inside fiber, switch to main context
	if(inThread)
	{
		printf("Thread with tid:%d is yielding to main context\n", currRunningThread->tid);
		currentRunningThreadId = currRunningThread->tid;
		swapcontext(&(currRunningThread->context), &mainContext);
	}
	else
	{
		if(numThreads == 0)
			return;

		// If we are in the main context, switch to one of the fibers
		printf("Switching to fiber: %d\n", currRunningThread->tid);
		//DEBUG_PRINT
		inThread = 1;
		//DEBUG_PRINT
		currentRunningThreadId = currRunningThread->tid;
		//DEBUG_PRINT
		initializeThreadPages();
		swapcontext(&mainContext, &(currRunningThread->context));
		//DEBUG_PRINT
		inThread = 0;

		// If thread finished executing, free it's resources
		if(currRunningThread->status == DONE)
		{
			printf("Current thread: %d is done executing\n", currRunningThread->tid);
			queueNode_t* n = deque(&q1);
			if(!n)
			{
				// Should never happen
				DEBUG_PRINT
				printf("Queue is empty, returning\n");
				return;
			}
			// Free stack allocation
			//free(currRunningThread->context.uc_stack.ss_sp);
			// Free queue node
			//free(n);

			// Update number of threads
			numThreads--;

			// return so that next 
			return;
		}
		// If thread is still running, execute next thread in the queue
		else if(currRunningThread->status == RUNNING)
		{
			printf("Current thread is still running, scheduling next thread\n");
			queueNode_t* n = deque(&q1);
			if(!n)
			{
				// Should never happen
				DEBUG_PRINT
				printf("Queue is empty, returning\n");
				return;
			}
			// Enque it, so that it will go to the end
			enque(&q1, n);

			currRunningThread = (pthread_t*)(q1.head)->thread; 
			currentRunningThreadId = currRunningThread->tid;
			return;
		}
	}
	//swapcontext(&mainContext, &(((pthread_t*)(q1.head)->thread)->context));
	
	/*
	pthread_t* prevThread = (q1.head)->thread;
	queueNode_t* n = deque(&q1);
	if(!n)
	{
		printf("Queue is empty\n");
		return;
	}
	else
	{
		printf("Thread removed with pointer:%p\n", n);
	}

	printf("Yield: Curr Q size:%d\n", q1.size);
	enque(&q1, n);
	printf("Yield: Curr Q size:%d\n", q1.size);
	//n = deque(&q1);
	if(!n)
	{
		printf("Queue is empty\n");
		return;
	}
	else
	{
		printf("Thread removed with pointer:%p\n", n);
	}

	currRunningThread = (pthread_t*)(q1.head)->thread;
	
	swapcontext(&mainContext, &(((pthread_t*)(q1.head)->thread)->context));
	*/
}

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


int my_pthread_mutex_init(my_pthread_mutex_t *mutex)
{
	printf("Initalizing mutex\n");
	//mutex = malloc(sizeof(my_pthread_mutex_t));
	//static allocation
	if(!mutex)
		return -1;

	mutex->value = 0 ;
	return 0;
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
	if(!mutex)
		return -1;

	while (1) {
		// If mutex is aquired, keep waiting and yielding
		if(mutex->value == 1)
		{
			printf("waiting for resource to be unlocked\n");
			numWaitingThreads++;
			my_pthread_yield();
		}
		else
		{
			// Acquire
			printf("Acquiring mutex\n");
			mutex->value = 1;
			return 0;
		}
	}
	return 0;
}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
	if(!mutex)
		return -1;

	if(mutex->value = 1)
	{
		printf("Releasing mutex\n");
		mutex->value = 0;
	}

	return 0;
}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	// static allocation
	//mutex = NULL;
	return 0;
}

