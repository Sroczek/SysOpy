compile:
	gcc -c -Wall transmitions.c
	gcc -c -Wall sender.c 
	gcc -c -Wall catcher.c


	gcc transmitions.o sender.o -o sender
	gcc transmitions.o catcher.o -o catcher

clean:
	rm *.o
	rm sender
	rm catcher
