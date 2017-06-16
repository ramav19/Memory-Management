#include "my_malloc.h"
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <time.h>

// Get the currently running thread
extern int currentRunningThreadId;
extern int dummyTid;
#define MAX_PAGES 1000
#define MAX_PAGES_SWAP 1000

#define MAX_THREADS 1000

#define PAGE_NOT_AVAILABLE -1
#define PAGE_SIZE 4*1024

char *swapPageOffset;


int findSwapPage();
int initiateSwapMainToSwap(int mainPage);
void resetMainPage(int pageNumber);
/*
// custom memory allocator
void* myallocate(unsigned int bytes, char* fileName, int lineNo, int flag)
{
	printf("My malloc requested from %s:%d\n", fileName, lineNo);

	if(flag == LIBRARYREQ)
	{
		// Normal malloc
	}
	else if(flag == THREADREQ)
	{
		// if current memory request is the first one assign it
	    // a page sized memory
		if(memoryMapBegin[currentRunningThreadId] == 0)
		{
			memoryMapBegin[currentRunningThreadId] = 1+(currentRunningThreadId-1)*4096;
			// Max end address is [4096*(currentRunningThreadId-1)-1
			if(bytes > 4096)
				return NULL;
			
			memoryMapEnd[currentRunningThreadId] = memoryMapBegin[currentRunningThreadId]+(bytes-1); 
			return memoryMapBegin[currentRunningThreadId];
		}
		else
		{
			// Memory was already allocated to this thread earlier
			// Check if new memory is within limits
			if(((memoryMapEnd[currentRunningThreadId]+bytes) - 
						memoryMapBegin[currentRunningThreadId]) >= 4096)
			{
				printf("Requesting for more than a page's worth of memory\n");
				return NULL;
			}
			else
			{
				unsigned int currentEnd = memoryMapEnd[currentRunningThreadId];
				// update new request
				memoryMapEnd[currentRunningThreadId] += bytes;
				return currentEnd;	
			}
		}
	}
}

// custom free function
void mydeallocate(unsigned int bytes, char* fileName, int lineNo, int flag)
{
	printf("My malloc requested from %s:%d\n", fileName, lineNo);

	if(flag == LIBRARYREQ)
	{
	}
	else if(flag == THREADREQ)
	{
		if(bytes <= 0)
		{
			// Nothing to do or invalid
			return;
		}
		else
		{
			// Memory wasn't allocated before, but trying to free
			if(memoryMapBegin[currentRunningThreadId] == 0)
			{
				return;
			}
			else
			{
				// validate
				memoryMapEnd[currentRunningThreadId] - bytes >= 
			}
		}
	}
}
*/

//Structure for book keeping
struct memoryManage
{
	int threadID;
	void *start;
	void *end;

	int count;

	struct internalBlock
	{
		int isOccupied;
		int size;
		void *blockStart;
		void *blockEnd;
	}block[100];

	int nextPage;

};

struct threadManage
{
	int threadID;
	int startPage;
	int numOfPages;

}thread[MAX_THREADS];



char mallocBuffer[1+(8*1024*1024)] = {0};

char swapSpace[1+(16*1024*1024)] = {0};

struct memoryManage *page[MAX_PAGES];

struct memoryManage *swap[MAX_PAGES_SWAP];


void resetSwapPage(int pageNumber)
{
	printf("resetSwapPage start\n");


	//Resetting the bookkeeping for the page
	swap[pageNumber]->threadID = -1;
//	page[pageNumber]->end = page[pageNumber]->start;
//	page[pageNumber]->count = 0;

//	int j=0;
//	for(j=0; j<100; j++)
//	{
//		page[pageNumber]->block[j].isOccupied = 0;
//	}

//	page[pageNumber]->nextPage = -1;

	//Clearing memory in the page

//	memcpy(page[pageNumber]->start, 0, PAGE_SIZE);

	printf("resetSwapPage end\n");
}

int initialteSwap_SwapToMain(int pageNo, int ThreadId)
{

	printf("initialteSwap_SwapToMain pageNo : %d  ", pageNo);
	int i=0;

	char *startAddress = page[pageNo]->start;

	for(i=0; i<MAX_PAGES_SWAP; i++)
	{
//		printf("swap thread ID : %d \n", swap[i]->threadID);
		if(ThreadId == swap[i]->threadID && page[pageNo]->start == swap[i]->start)
		{
			printf("Swap page = %d\n", i);
			char * swapPageLocation = swapPageOffset + i * PAGE_SIZE;

			printf("swapPageLocation : %d, swap[swapPage]->start : %d \n",swapPageLocation, swap[i]->start);

			//Swapping the page from swap space to main memory
			memcpy(page[pageNo]->start, swapPageLocation, PAGE_SIZE);

			//Swapping bookkeeping for the page from swap space to main memory
			memcpy(page[pageNo], swap[i], sizeof(struct memoryManage));

			//resetting the main page
			resetSwapPage(i);

			int j=0;
			for(j=0; j<MAX_PAGES; j++)
			{
				printf("page [%d], page = %d\n", j, page[j]->threadID);
			}
			for(j=0; j<MAX_PAGES_SWAP; j++)
			{
				printf("swap [%d], threadID = %d\n", j, swap[j]->threadID);
			}



			return 1;
		}
	}


	return -1;
}


void initializeThreadPages()
{
	int pageNo = -1;

	int i;
	int found = 0;

	for(i=0; i<MAX_THREADS; i++)
	{
		if(thread[i].threadID == currentRunningThreadId)
		{
			pageNo = thread[i].startPage;
			found = 1;
			break;
		}
	}

	if(found == 0)
	{
		printf("No pages allocated to thread yet");
		return;
	}

	while(pageNo != -1)
	{
		printf("Page number of Thread: %d\n", pageNo);
		//If the required thread page is not present in the main memory
		if(page[pageNo]->threadID != currentRunningThreadId)
		{
			//swapping out the page from main memory into swap space
			int result = initiateSwapMainToSwap(pageNo);
			if(result == -1)
			{
				printf("Swap from Main Memory to Swap file Failed\n");
			}
			else
			{
				printf("Swap from Main Memory to Swap file Successful\n");
			}

			//Swapping the correct thread page back into the main memory
			result = initialteSwap_SwapToMain(pageNo, currentRunningThreadId);
			if(result == -1)
			{
				printf("Swap from Swap File to Main memory Failed\n");
			}
			else
			{
				printf("Swap from Swap File to Main memory Successful\n");
			}
		}

		pageNo = page[pageNo]->nextPage;
	}
}



void initializeThreadStructure()
{
	int i=0;

	for(i=0; i<MAX_THREADS; i++)
	{
		thread[i].threadID = -1;
		thread[i].startPage = -1;
		thread[i].numOfPages = 0;
	}
}

void initializeSwapSpace()
{
	char *startAddress;
	int allocationSize;

	startAddress = swapSpace;
	allocationSize = sizeof(struct memoryManage);

	int i;
	for(i=0; i<MAX_PAGES_SWAP; i++)
	{
		swap[i] = startAddress;
		startAddress += allocationSize;
	}

	swapPageOffset = startAddress;

	printf("swapSpace : %p, swap[0]: %d , swap[1]: %d \n", swapSpace, swap[0] , swap[1]);

	allocationSize = 4 * 1024;

	for(i=0; i<MAX_PAGES_SWAP; i++)
	{
		swap[i]->threadID = -1;
		swap[i]->start = swap[i]->end = startAddress;
		startAddress += allocationSize;

		swap[i]->count = 0;

		int j= 0;

		for(j=0; j<100; j++)
		{
			swap[i]->block[j].isOccupied = 0;
		}

		swap[i]->nextPage = -1;
	}

	printf("threadID : %d, count: %d  \n", swap[1]->threadID, swap[1]->count);
}



void initializeMemory()
{
	char *startAddress;
	int allocationSize;

	startAddress = mallocBuffer;
	allocationSize = sizeof(struct memoryManage);

	int i;
	for(i=0; i<MAX_PAGES; i++)
	{
		page[i] = startAddress;
		startAddress += allocationSize;
	}


	printf("mallocBuffer : %p, page[0]: %d , page[1]: %d \n", mallocBuffer, page[0] , page[1]);

	allocationSize = 4 * 1024;

	for(i=0; i<MAX_PAGES; i++)
	{
		page[i]->threadID = -1;
		page[i]->start = page[i]->end = startAddress;
		startAddress += allocationSize;

		page[i]->count = 0;

		int j= 0;

		for(j=0; j<100; j++)
		{
			page[i]->block[j].isOccupied = 0;
		}

		page[i]->nextPage = -1;
	}

	printf("Main Memory threadID : %d, count: %d  \n", page[1]->threadID, page[1]->count);

	initializeThreadStructure();

	initializeSwapSpace();

}

//Finds a page in main memory to swap out.
//The page that is swapped out is selected at random
int findMainPageToSwap(int currentRunningThreadId)
{
	int count = 0;
	srand(time(NULL));
	int pageNo = 1;
	pageNo = rand() % MAX_PAGES;

	while(1)
	{
		if(page[pageNo]->threadID != currentRunningThreadId)
		{
			return pageNo;
		}

		pageNo++;

		if(pageNo == MAX_PAGES)
		{
			pageNo = 0;
		}

		count++;
		if(count == 100)
		{
			break;
		}
	}

	return -1;

}

//Allocates a new page for a current Thread
int allocateNewPageToThread(int currentRunningThreadId)
{
	printf("allocateNewPageToThread start\n");
	int i=0;
	for(i=0; i<MAX_PAGES; i++)
	{
		if(page[i]->threadID == -1)
		{
			page[i]->threadID = currentRunningThreadId;
			page[i]->count = 0;

			return i;
		}
	}
	printf("Pages not available in main memory\n");

	//Free pages not available in main memory. Will swap out a page in main memory and return this new page.

	int mainPage;

	mainPage = findMainPageToSwap(currentRunningThreadId);

	printf("mainPage = %d\n", mainPage);

	if(mainPage == -1)
	{
		printf("Current thread has too many pages\n");
		return -1;
	}

	int result = initiateSwapMainToSwap(mainPage);

	if(result == -1)
	{
		printf("Swap Failed\n");
	}
	return mainPage;
}


//Scans the page specified to see if there is memory which can be allocated
void *allocateMemoryInPage(int bytes, int pageNo)
{
	int spaceRemaining = page[pageNo]->start + 4*1024 - page[pageNo]->end;
	if(spaceRemaining >= bytes)
	{
		printf("spaceRemaining >= bytes\n");
		int nextBlock = page[pageNo]->count;
		page[pageNo]->count++;

		page[pageNo]->block[nextBlock].blockStart = page[pageNo]->end;
		page[pageNo]->block[nextBlock].blockEnd = page[pageNo]->end + bytes;
		page[pageNo]->end = page[pageNo]->end + bytes;


		page[pageNo]->block[nextBlock].isOccupied = 1;
		page[pageNo]->block[nextBlock].size = bytes;

		return (page[pageNo]->block[nextBlock].blockStart);
	}
	else
	{
		printf("remaining space is smaller than bytes\n");
		int j;

		for(j=0; j<100; j++)
		{
			//The actual size of the block available is greater than the size occupied
			int totalBlockSize = page[pageNo]->block[j].blockEnd - page[pageNo]->block[j].blockStart;
			if(page[pageNo]->block[j].isOccupied == 0 && totalBlockSize >=bytes)
			{

				int nextBlock = j;

				page[pageNo]->block[nextBlock].isOccupied = 1;
				page[pageNo]->block[nextBlock].size = bytes;

				return (page[pageNo]->block[nextBlock].blockStart);
			}
		}

		printf("available blocks are smaller than bytes\n");
		//If there is no space in the current page

		//Check if space is available in the next page
		int nextPage = page[pageNo]->nextPage;
		if(nextPage != -1)
		{
			return allocateMemoryInPage(bytes, nextPage);
		}
		//If space is not available in the next page then allocate a new page

		else
		{
			printf("Next page not available\n");
			int newPage;
			newPage = allocateNewPageToThread(page[pageNo]->threadID);
			printf("New pageNumber = %d\n", newPage);

			if(newPage == -1)
			{
				printf("No space available \n");
				return NULL;
			}

			page[newPage]->threadID = page[pageNo]->threadID;
			page[pageNo]->nextPage = newPage;

			void *allocatedAddress = allocateMemoryInPage(bytes, newPage);

			if(allocatedAddress == NULL)
			{
				printf("Could not allocate memory");
			}

			return allocatedAddress;
		}
	}
}

int findSwapPage()
{

	int i =0;

	for(i=0; i<MAX_PAGES_SWAP; i++)
	{
		if(swap[i]->threadID == -1)
		{
			return i;
		}
	}

	return -1;
}

int initiateSwapMainToSwap(int mainPage)
{

	int swapPage = findSwapPage();

	printf("initiateSwapMainToSwap mainPage = %d, swapPage = %d\n", mainPage, swapPage);


	if(swapPage == -1)
	{
		printf("Swap memory full\n");
		return -1;
	}

	char * swapPageLocation = swapPageOffset + swapPage * PAGE_SIZE;

	printf("swapPageLocation : %d, swap[swapPage]->start : %d \n",swapPageLocation, swap[swapPage]->start);


	//Swapping the page from main memory to swap space
	memcpy(swapPageLocation, page[mainPage]->start, PAGE_SIZE);

	//Swapping bookkeeping for the page from main memory to swap space
	memcpy(swap[swapPage], page[mainPage], sizeof(struct memoryManage));

	printf("swap[swapPage] : %d,  page[mainPage] : %d\n", swap[swapPage]->threadID,  page[mainPage]->threadID);


	//resetting the main page
	resetMainPage(mainPage);

	int j=0;


	for(j=0; j<MAX_PAGES; j++)
	{
		printf("page [%d], page = %d\n", j, page[j]->threadID);
	}
	for(j=0; j<MAX_PAGES_SWAP; j++)
	{
		printf("swap [%d], threadID = %d\n", j, swap[j]->threadID);
	}

	return 0;
}

#ifdef SWAP_OUT_PAGE


int swapOutPage(int previousPage)
{
	int newPage;

	if(previousPage == -1)
	{
		newPage = 1;
	}

	int swapPage;
	swapPage = findSwapPage();



	if(swapPage == -1)
	{
		printf("Swap memory full\n");
		return -1;
	}
	else
	{
		initiateSwapMainToSwap(newPage, swapPage);
		return newPage;
	}


}
#endif


void* myallocate(unsigned int bytes, char* fileName, int lineNo, int flag)
{
	printf("myallocate start\n");
	printf("byes = %d, filename = %s, flag = %d \n", bytes, fileName, flag);

	printf("size of page : %ld\n", sizeof(struct memoryManage));

	int threadID;

	if(flag == LIBRARYREQ)
	{
		printf("myallocate flag == LIBRARYREQ\n");
		threadID = dummyTid;
		//void* ret =  __libc_malloc(bytes);
		//printf("Ret: %p\n", ret);
		//return ret;
	}
	else
	{
		threadID = currentRunningThreadId;
	}


	if(bytes > 4*1024)
	{
		printf("Space not available\n");
		return NULL;
	}

	int i=0;

	//If book keeping for thread is already available
	for(i=0; i<MAX_THREADS; i++)
	{
		if(thread[i].threadID == threadID)
		{
			printf("pageNumber = %d\n", thread[i].startPage);

			void *allocatedAddress = allocateMemoryInPage(bytes, thread[i].startPage);

			if(allocatedAddress == NULL)
			{
				printf("Could not allocate memory");
			}

			return allocatedAddress;

		}
	}

	printf("Thread not found\n");

	//If book keeping not available for thread (first time allocation)

	for(i=0; i<MAX_THREADS; i++)
	{
		if(thread[i].threadID == -1)
		{

			thread[i].threadID = threadID;

			int newPage = allocateNewPageToThread(threadID);
			printf("pageNumber = %d\n", newPage);

			if(newPage == -1)
			{
				printf("Could not allocate memory\n");
//				newPage = swapOutPage(-1);
			}

			thread[i].startPage = newPage;
			thread[i].numOfPages = 1;

			void *allocatedAddress = allocateMemoryInPage(bytes, thread[i].startPage);

			if(allocatedAddress == NULL)
			{
				printf("Could not allocate memory");
			}

			return allocatedAddress;
		}
	}

	printf("Max threads reached");


	printf("myallocate end\n");

	return NULL;
}

//If no memory is occupied by a thread, then the page and book keeping is freed
void resetMainPage(int pageNumber)
{
	printf("resetMainPage start\n");


	//Resetting the bookkeeping for the page
	page[pageNumber]->threadID = -1;
	page[pageNumber]->end = page[pageNumber]->start;
	page[pageNumber]->count = 0;

	int j=0;
	for(j=0; j<100; j++)
	{
		page[pageNumber]->block[j].isOccupied = 0;
	}

	page[pageNumber]->nextPage = -1;

	//Clearing memory in the page

//	memcpy(page[pageNumber]->start, 0, PAGE_SIZE);

	printf("resetMainPage end\n");
}

#ifdef checkThreadSpaceRequired
//Checks if the thread has any memory allocated to it.
void checkThreadSpaceRequired(int pageNumber)
{
	printf("checkThreadSpaceRequired start\n");

	int i=0;
	int found =0;

	for(i=0; i<100; i++)
	{
		if(page[pageNumber]->block[i].isOccupied == 1)
		{
			found = 1;
		}
	}

	if(found == 0)
	{
		resetThreadPage(pageNumber);
	}

	printf("checkThreadSpaceRequired end\n");
}
#endif



//Recursively checks all pages assigned to the thread and deallocates
void recursiveDeallocateInPage(int pageNumber, char *ptr)
{

	if(pageNumber == -1)
		return;

	int j=0;

	int found =0;

	for(j=0; j<100; j++)
	{
		if(page[pageNumber]->block[j].blockStart == ptr)
		{
			printf("Successfully deallocated\n");

			found = 1;

			page[pageNumber]->block[j].isOccupied = 0;
			page[pageNumber]->block[j].size = 0;
			page[pageNumber]->count--;

//			if(page[pageNumber]->count == 0)
//			{
//				resetThreadPage(pageNumber);
//			}


		}
	}

	if(found == 0)
	{
		if(page[pageNumber]->nextPage == -1)
		{
			printf("Could not deallocate\n");
		}
		else
		{
			recursiveDeallocateInPage(page[pageNumber]->nextPage, ptr);
		}
	}


}

void mydeallocate(char *ptr, char* fileName, int lineNo, int flag)
{
	printf("mydeallocate start\n");
	printf("ptr = %d, filename = %s, flag = %d \n", ptr, fileName, flag);

	int threadID;
	if(flag == LIBRARYREQ)
	{
		threadID = dummyTid;
		//__libc_free(ptr);
	}
	else
	{
		threadID = currentRunningThreadId;
	}

	int i=0;
	for(i=0; i<MAX_THREADS; i++)
	{
		if(thread[i].threadID == threadID)
		{
			recursiveDeallocateInPage(thread[i].startPage, ptr);
		}

	}



	printf("mydeallocate end\n\n");
}


