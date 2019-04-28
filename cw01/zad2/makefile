small_dir= "/bin"
medium_dir = "/lib"
large_dir = "/"

basic: library.o main.o
	gcc library.o main.o -o main

library.o: ../zad1/library.c
	gcc -c -Wall ../zad1/library.c

main.o: main.c
	gcc -c -Wall main.c

test:
	./main 10 search_directory $(large_dir) \* tmp.txt search_directory $(medium_dir) \* tmp1.txt search_directory $(small_dir) \* tmp2.txt load_block_from_file tmp.txt load_block_from_file tmp1.txt load_block_from_file tmp2.txt delete_block 0 delete_block 1 delete_block 2 repeat_load_and_remove tmp.txt 5 4 repeat_load_and_remove tmp1.txt 5 4 repeat_load_and_remove tmp2.txt 5 4

clean:
	rm *.o
	rm main
	rm tmp*