all: static shared

static:
	gcc -Wall -c library.c -o library.o
	ar rcs library.a library.o

shared: 
	gcc -Wall -fPIC -c library.c -o library.o
	gcc -Wall -fPIC -shared -o library.so library.o

clean:
	rm *.o
	rm *.so
	rm *.a
