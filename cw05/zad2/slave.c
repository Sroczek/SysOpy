#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int line_lenght = 128;

int main(int argc, char **argv){

    if(argc < 3){
        printf("To few arguments in program call\n");
        exit(-1);
    }

    char buffer1[line_lenght];
    char buffer2[line_lenght];

    long int N = strtol(argv[2], NULL, 10);
    int pipe = open(argv[1], O_WRONLY);
    
    long int pid = (long int)(getpid());
    printf("Slave with PID: %ld start working\n", pid);
    for(int i=0; i<N; i++){
        FILE *date = popen("date", "r");
        fgets(buffer1, line_lenght, date);
        pclose(date);

        sprintf(buffer2, "Slave: %ld - %s", pid, buffer1);
        write(pipe, buffer2, strlen(buffer2));
        printf("%s", buffer2);

        int time = 2 + rand()%4;
        for(int k=0; k<time; k++) sleep(1);
    }
    close(pipe);
    return 0;
}