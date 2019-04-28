small_dir= "/bin"
medium_dir = "/lib"
large_dir = "/"

built: static shared dynamic

static: 
	gcc -c -Wall ../zad1/library.c -o library.o -O$(o)
	ar rcs library.a library.o
	gcc -Wall ../zad2/main.c -I. library.a -o main_static -O$(o)

shared: 
	gcc -c -Wall -fPIC ../zad1/library.c -O$(o) 
	gcc -Wall -fPIC -shared -o library.so library.o -O$(o)
	
	gcc -Wall -L. library.so library.o -o main_shared ../zad2/main.c -O$(o)

dynamic: 
	gcc -c -Wall -fPIC ../zad1/library.c  -O$(o)
	gcc -Wall -fPIC -shared -o library.so library.o -O$(o)
	
	gcc -Wall -o main_dynamic ./main_dynamic.c -ldl -O$(o)

static_test: 
	./main_static 10 search_directory $(large_dir) \* tmp.txt search_directory $(medium_dir) \* tmp1.txt search_directory $(small_dir) \* tmp2.txt load_block_from_file tmp.txt load_block_from_file tmp1.txt load_block_from_file tmp2.txt delete_block 0 delete_block 1 delete_block 2 repeat_load_and_remove tmp.txt 5 4 repeat_load_and_remove tmp1.txt 5 4 repeat_load_and_remove tmp2.txt 5 4

shared_test:
	./main_shared 10 search_directory $(large_dir) \* tmp.txt search_directory $(medium_dir) \* tmp1.txt search_directory $(small_dir) \* tmp2.txt load_block_from_file tmp.txt load_block_from_file tmp1.txt load_block_from_file tmp2.txt delete_block 0 delete_block 1 delete_block 2 repeat_load_and_remove tmp.txt 5 4 repeat_load_and_remove tmp1.txt 5 4 repeat_load_and_remove tmp2.txt 5 4

dynamic_test:
	./main_dynamic 10 search_directory $(large_dir) \* tmp.txt search_directory $(medium_dir) \* tmp1.txt search_directory $(small_dir) \* tmp2.txt load_block_from_file tmp.txt load_block_from_file tmp1.txt load_block_from_file tmp2.txt delete_block 0 delete_block 1 delete_block 2 repeat_load_and_remove tmp.txt 5 4 repeat_load_and_remove tmp1.txt 5 4 repeat_load_and_remove tmp2.txt 5 4

all_tests:
	echo "\n\n\nTest with option "-O0":\n+++++++++++++++++++++++++++++++++" >> raport2.txt
	make test o=0 
	echo "\n\n\nTest with option "-O1":\n+++++++++++++++++++++++++++++++++" >> raport2.txt
	make test o=1
	echo "\n\n\nTest with option "-Os":\n+++++++++++++++++++++++++++++++++" >> raport2.txt
	make test o=s

test: built
	echo "Static:\n------------------------------------------------" >> raport2.txt
	make static_test 
	echo "\n\nShared:\n------------------------------------------------" >> raport2.txt
	make shared_test
	echo "\n\nDynamic:\n------------------------------------------------" >> raport2.txt
	make dynamic_test
	make clean
	
clean:
	rm lib*
	rm main_shared
	rm main_dynamic
	rm main_static
	rm tmp*