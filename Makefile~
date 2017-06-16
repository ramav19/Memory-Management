all: clean main 

clean:
	rm -rf mypthread

main: main.c queue.h my_pthread_t.h my_malloc.h
	gcc -g main.c queue.c my_malloc.c my_pthread_t.c -o mypthread
