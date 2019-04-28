#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


void folow_dir(char *dir_path){

    DIR *dir = opendir(dir_path);

    if(dir == NULL){
        printf("No such directory under given path.\n");
        return; 
    }

    struct dirent *current = readdir(dir);
    struct stat *st = malloc(sizeof(struct stat));
    int stat;

    pid_t child_pid = fork();
    if(child_pid == 0){
        printf("\nPID: %ld   Path: %s\n", (long int)getpid(), dir_path);
        execlp("ls", "ls", "-l", dir_path, NULL);
    }
    else{
        wait(&stat);
    }

    while(current != NULL){
        char * curr_path = calloc(strlen(dir_path) + strlen(current->d_name) + 2 ,sizeof(char));
        strcat(curr_path, dir_path);
        strcat(curr_path, "/");
        strcat(curr_path, current->d_name);

        if(strcmp(current->d_name,".") != 0 && strcmp(current->d_name,"..") != 0){
            
            lstat(curr_path, st);

            if(S_ISDIR(st->st_mode)) folow_dir(curr_path);

            free(curr_path);
        }
        current = readdir(dir);
    }

    closedir(dir);
    free(st);
}


int main(int argc, char **argv){

    if(argc < 2){
        printf("To few arguments in program call");
        return -1;
    }

    folow_dir(argv[1]);
    
    return 0;
}