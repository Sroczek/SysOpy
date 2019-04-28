compile: 
	gcc -Wall -c monitor.c -o monitor.o -O2
	gcc monitor.o -o monitor

test:
	touch tmp1 tmp2 tmp3
	mkdir -p archiwum
	./monitor lista

clean_all:
	rm tmp*
	rm -r archiwum
	rm *.o
	rm monitor

clean:
	rm *.o
	rm monitor	