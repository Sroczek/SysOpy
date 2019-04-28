#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


void add_to_file(char* path, int time_before_adding_line, int bytes){
    time_t present_time;
    time(&present_time);

    char date[20];
    strftime(date,20,"%Y-%m-%d_%H-%M-%S",localtime(&present_time));

    char *random_bytes = calloc(bytes, sizeof(char));
    for(int i=0;i<bytes;i++) random_bytes[i] = (unsigned char)(100 + rand()% 20);
    
    FILE *file;
    file = fopen(path, "a+");

    fprintf(file,"%ld %i %s %s\n", (long int)getpid(), time_before_adding_line, date, random_bytes);

    fclose(file);
}

int main(int argc, char **argv){

    time_t begin_time;
    time(&begin_time);

    if(argc < 5){
        printf("To few arguments in program call\n");
        exit(-1);
    }

    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);

    struct timespec ts;
    clock_gettime(0, &ts);
    srand((ts.tv_nsec));

    int time_before_adding_line = rand() % (pmax - pmin + 1) + pmin;

    time_t end_time;
    time(&end_time);
    while(difftime(end_time,begin_time) < (double)time_before_adding_line) time(&end_time);

    add_to_file(argv[1],time_before_adding_line, bytes);

    return 0;
}