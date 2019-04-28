compile:
	gcc -c -Wall communication.c
	# gcc -c -Wall client.c 
	gcc -c -Wall server.c

	# gcc comunication.o client.o -o client
	gcc communication.o server.o -o server

clean:
	rm *.o
	rm server
	# rm client