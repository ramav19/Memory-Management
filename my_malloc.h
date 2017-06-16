#ifndef _MY_MALLOC_H

#define _MY_MALLOC_H 1
//#include "my_pthread_t.h"

// static buffer of 8 MB
// We won't use the first location to make it easier for maintenance

#define THREADREQ 1
#define LIBRARYREQ 2

//#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
//#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)

// 8MB static buffer can be allocated for a maximum of 8MB/4KB = 2K threads
// Each should hold a start location of static buffer
// If it's NULL, that means this thread isn't allocated any memory
/*
struct addressMap {
	unsigned int begin;
	unsigned int max;
	unsigned int end;
} addressMap_default {0,0,0};
*/

// Again, we won't use first location for simpler maintenance
//unsigned int memoryMapBegin[2048+1] = {0};
//unsigned int memoryMapEnd[2048+1] = {0};

// custom memory allocator
void* myallocate(unsigned int bytes, char* fileName, int lineNo, int flag);

// custom free function
void mydeallocate(char *ptr, char* fileName, int lineNo, int flag);

void initializeThreadPages();


void initializeMemory();

#endif // _MY_MALLOC_H
