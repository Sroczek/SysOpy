compile: 
	gcc -Wall -c monitor.c -o monitor.o -O2
	gcc monitor.o -o monitor

	gcc -Wall -c tester.c -o tester.o -O2
	gcc tester.o -o tester

test:
	touch tmp1 tmp2 tmp3
	mkdir -p archiwum
	./monitor lista 60 mem_cp 60 4000 &
	./tester tmp1 5 5 20
	./tester tmp1 5 5 20
	./tester tmp1 5 5 20
	./tester tmp1 5 5 20
	
	./tester tmp2 5 5 20
	./tester tmp2 5 5 20

	./tester tmp3 5 5 20

clean:
	rm *.o
	rm tester
	rm monitor
	rm tmp*
	rm -r archiwum