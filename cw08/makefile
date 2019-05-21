compile:
	# gcc -Wall converter.c -o converter -O2
	gcc -c -Wall pattern_generator.c -O2

	gcc -c -Wall pgmlib.c -O2 -lm -pthread
	gcc -c -Wall main.c -O2 -lm -pthread

	gcc pgmlib.o pattern_generator.c -o gen
	gcc pgmlib.o main.o -o main -lm -pthread
	rm *.o

test:
	make generate
	mkdir splots
	make unit_test n=3
	make unit_test n=9
	make unit_test n=15
	make unit_test n=21
	make unit_test n=27
	make unit_test n=33
	make unit_test n=39
	make unit_test n=45
	make unit_test n=51
	make unit_test n=57
	make unit_test n=63


unit_test: 
	./main 1 BLOCK ./picture.pgm ./patterns/pattern_$nx$n ./splots/splot_b_1_$n.pgm
	./main 2 BLOCK ./picture.pgm ./patterns/pattern_$nx$n ./splots/splot_b_2_$n.pgm
	./main 4 BLOCK ./picture.pgm ./patterns/pattern_$nx$n ./splots/splot_b_4_$n.pgm
	./main 8 BLOCK ./picture.pgm ./patterns/pattern_$nx$n ./splots/splot_b_8_$n.pgm
	

generate: 
	mkdir patterns
	./gen 3
	./gen 9
	./gen 15
	./gen 21
	./gen 27
	./gen 33
	./gen 39
	./gen 45
	./gen 51
	./gen 57
	./gen 63


clean:
	rm main
	rm gen


clean_all:
	make clean
	rm -r splots
	rm -r patterns
	# rm converter