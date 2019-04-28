basic: 
	gcc -Wall -c main.c -o main.o -O2
	gcc main.o -o main

test:
	echo "       Command                           user     sys\n" >> wyniki.txt
	make unit_test records=7500 bytes=1
	make unit_test records=7500 bytes=4
	make unit_test records=7500 bytes=512
	make unit_test records=7500 bytes=1024
	make unit_test records=7500 bytes=4096
	make unit_test records=7500 bytes=8192

	make unit_test records=15000 bytes=1
	make unit_test records=15000 bytes=4
	make unit_test records=15000 bytes=512
	make unit_test records=15000 bytes=1024
	make unit_test records=15000 bytes=4096
	make unit_test records=15000 bytes=8192

prepare:
	./main generate tmp.txt $(records) $(bytes)
	./main copy tmp.txt copy1.txt $(records) $(bytes) lib
	./main copy tmp.txt copy2.txt $(records) $(bytes) sys

unit_test: prepare
	./main sort copy1.txt $(records) $(bytes) lib
	./main sort copy2.txt $(records) $(bytes) sys
	echo " " >> wyniki.txt

clean:
	rm main
	rm *.o
	rm *.txt
