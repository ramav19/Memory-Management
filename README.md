# Memory-Management

The implementation consists of the following :

1) We have declared a static character array of 8 MB where we perform the necessary allocations. An
array of structures is used to keep track of the threads and the pages allocated to the threads. This array
resides in the 8 MB memory. The rest of the 8MB is divided into pages of size 4 KB each.


2) An array of structures is used for bookkeeping within each page. This structure also keeps track of
which blocks of memory have been allocated and which have been freed. We are using this to handle
fragmentation (blocks of memory which have been freed are reused).
Few of the contents of the structure are:
a)Thread ID
b)Start address of the page
c)End address of the page
d)Count of the number of blocks
e)Structure of blocks to keep track of the allocations and the size of the allocations (used for
allocation and deallocation).
f)Pointer to the next page allocated to the thread.


3) When the thread calls myallocate, all the pages allocated to the thread are parsed to check for free
space. If the free space is available then it is allocated. If not, another page is allocated to the thread
from the main memory.

4) A swap space of size 16 MB is created. An array of structures is used for bookkeeping in this case
too. This array resides in the 16 MB memory.

5) If all the pages in the main memory are filled, then a page which does not belong to the thread is
chosen at random from the main memory. This page is then swapped into an empty location in the swap
space, thereby freeing a page which is allocated to the thread. The bookkeeping for the page is also
swapped out into the swap space.

6) When a thread is switched into context, an initial check is done to make sure all the pages allocated
to it are residing in the main memory. If any page is not present in the main memory, the unwanted
page is swapped out from the main memory to the swap space. The swap space is checked to find the
required page, and the page (with bookkeeping) is swapped into the main memory.
