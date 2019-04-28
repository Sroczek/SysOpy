#include <unistd.h>
#include <stdio.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


void print_command(char ***command){
    int n = sizeof(command)/sizeof(char**);
    n=3;
    for(int i=0; i<n; i++){
        int m = sizeof(command[i])/sizeof(char*);
        m=3;
        for(int j=0; j<m; j++){
            printf("%s|", command[i][j]);
        }
        printf("\n");
    }
}

int read_line(FILE *file, char **line){
    int buffer_size = 100;
    *line = malloc(buffer_size*sizeof(char));
    int size = 0;

    char c = getc(file);
    while(c != '\n' && c != EOF){
        (*line)[size] = c;
        size++;
        if(size >= buffer_size){
            buffer_size += buffer_size/3;
            realloc(*line, buffer_size*sizeof(char));
        }
        c = getc(file);
    }

    realloc(*line, size*sizeof(char));
    if(c == '\n') return 0;
    return -1;
}

int load_command(char *line, char ***command){
    
    char delim[] = " ";
    char *tmp;
    int buff_size = 10;
    *command = malloc(buff_size*sizeof(char*));

    int len = 0;
    tmp = strtok(line, delim);
    while(tmp != NULL){
        if(len >= buff_size){
            buff_size += buff_size/3;
            realloc((*command), buff_size*sizeof(char*));
        }
        (*command)[len]=tmp;
        (len) +=1;
        tmp = strtok(NULL, delim);
    }

    realloc((*command), len*sizeof(char*));
    

    // printf("Command %i\n", *len);
    // for(int i=0; i<*len; i++){
    //     printf("%s\n",(*command)[i]);
    // }
    return len;
}

int get_commands(FILE *file, char ****commnads, int *len){
    char *line;
    int flag = 0;

    int buffer_size = 10;
    *commnads = malloc(buffer_size*sizeof(char***));
    *len = 0;

    while(flag == 0){
        if((*len) >= buffer_size){
            buffer_size += buffer_size/3;
            realloc(*commnads, buffer_size*sizeof(char***));
        }

        flag = read_line(file, &line);
        char **command;
        load_command(line, &command);

        (*commnads)[*len] = command;
        (*len)++;
    }

    realloc(*commnads, (*len)*sizeof(char***));
    return *len;
}

void generate_pipeline(FILE *file){
    
    char ***commands;
    int len;
    get_commands(file,&commands, &len);

    // print_command(commands);

    int acc[2];
    int previous[2];
    for(int i=0; i<len; i++){
        pipe(acc);
        pid_t child_pid = fork();
        if(child_pid == 0){

            printf("PID: %i\n", getpid());

            if(i< len-1){
                dup2(acc[1],STDOUT_FILENO);
                close(acc[0]);
            }
            if(i > 0){
                dup2(previous[0], STDIN_FILENO);
                close(previous[1]);
            }
            
            if( execvp(commands[i][0], commands[i]) == -1) printf("Error with \"exec\"\n");
            exit(-1);
        }
        
        if(i > 0){
            close(previous[0]);
            close(previous[1]);
        }

        previous[0] = acc[0];
        previous[1] = acc[1];
    }
    close(acc[0]);
    close(acc[1]);

    for(int i=0; i<len; i++) {
    int pid = wait(NULL);
    // printf("%i\n", pid);
    }

}

int main(int argc, char** argv){

    if(argc < 2){
        printf("To few arguments in program call\n");
        exit(-1);
    }

    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Cannot open file\n");
        exit(-1);
    } 

    generate_pipeline(file);

    fclose(file);
    return 0;
}