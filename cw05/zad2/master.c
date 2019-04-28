#include <stdlib.h> 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> 

int line_lenght = 128;

int main(int argc, char **argv){

    if(argc < 2){
        printf("To few arguments in program call\n");
        exit(-1);
    }
    
    if(mkfifo(argv[1], 0700) == -1 && errno != EEXIST){ 
        printf("Error while creating FIFO\n");
        exit(-1);
    }

    char buffer[line_lenght];
    FILE *pipe = fopen(argv[1], "r");

    while(fgets(buffer, line_lenght, pipe) != NULL){
        printf("%s", buffer);
    }

    fclose(pipe);
    return 0;
}