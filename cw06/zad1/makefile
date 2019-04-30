compile:
	gcc -c -Wall communication.c -O2
	gcc -c -Wall client.c -O2
	gcc -c -Wall server.c -O2

	gcc communication.o client.o -o client
	gcc communication.o server.o -o server

clean:
	rm *.o
	rm server
	rm client