#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ftw.h>

enum time_relation {EARLIER, SAME_TIME, LATER};

enum time_relation tr;
time_t rel_time;

void print_info(const char *path, const struct stat *st){
    if(S_ISREG(st->st_mode))  printf("file      ");
    if(S_ISDIR(st->st_mode))  printf("dir       ");
    if(S_ISCHR(st->st_mode))  printf("char_dev  ");
    if(S_ISBLK(st->st_mode))  printf("block_dev ");
    if(S_ISFIFO(st->st_mode)) printf("fifo      ");
    if(S_ISLNK(st->st_mode))  printf("slink     ");
    if(S_ISSOCK(st->st_mode)) printf("sock      ");

    printf("%10lld  ", (long long int)(st->st_size));

    char buffer[20];
    strftime(buffer, 20, "%Y-%m-%d_%H:%M:%S", localtime(&st->st_atime));
    printf("%s  ", buffer);

    strftime(buffer, 20, "%Y-%m-%d_%H:%M:%S", localtime(&st->st_mtime));
    printf("%s  ", buffer);

    printf("%s\n", path);
}

static int nftw_print_info(const char *path, const struct stat *st, int type, struct FTW *ftwbuf){
     switch (tr)
            {
                case EARLIER:
                    if(difftime(rel_time, st->st_mtime) > 0) print_info(path,st);
                    break;

                case LATER:
                    if(difftime(rel_time, st->st_mtime) < 0) print_info(path,st);
                    break;

                case SAME_TIME:
                    if(difftime(rel_time, st->st_mtime) == 0) print_info(path,st);
                    break;

            }
    return 0;
}

void folow_dir(char *dir_path){
    DIR *dir = opendir(dir_path);

    if(dir == NULL){
        printf("No such directory under given path.\n");
        return; 
    }

    struct dirent *current = readdir(dir);
    struct stat *st = malloc(sizeof(struct stat));     

    while(current != NULL){
        char * curr_path = calloc(strlen(dir_path) + strlen(current->d_name) + 2 ,sizeof(char));
        strcat(curr_path, dir_path);
        strcat(curr_path, "/");
        strcat(curr_path, current->d_name);

        if(strcmp(current->d_name,".") != 0 && strcmp(current->d_name,"..") != 0){
            
            lstat(curr_path, st);

            switch (tr)
            {
                case EARLIER:
                    if(difftime(rel_time, st->st_mtime) > 0) print_info(curr_path,st);
                    break;

                case LATER:
                    if(difftime(rel_time, st->st_mtime) < 0) print_info(curr_path,st);
                    break;

                case SAME_TIME:
                    if(difftime(rel_time, st->st_mtime) == 0) print_info(curr_path,st);
                    break;

            }

            //jeśli katalog to wejdź
            if(S_ISDIR(st->st_mode)) folow_dir(curr_path);

            free(curr_path);
        }
        current = readdir(dir);
    }
    closedir(dir);
    free(st);
}


int main(int argc, char **argv){

    if(argc < 4){
        printf("To few arguments in program call");
        return -1;
    }

    //operant of relation between time
    if(strcmp(argv[2], "<") == 0) tr = EARLIER;
    else if(strcmp(argv[2], "=") == 0) tr = SAME_TIME;
    else if(strcmp(argv[2], ">") == 0) tr = LATER;
    else {
        printf("Illegal value of second argument.");
        return -2;
    }


    //getting time
    struct tm *given_time = malloc(sizeof(struct tm));

    char format[17] = "%Y-%m-%d_%H:%M:%S";
    strptime(argv[3], format, given_time);
    rel_time = mktime(given_time);

    //open given diricetory

    char *path = realpath(argv[1], NULL);


    printf("Custom way:\n -----------------------------------------------\n");
    folow_dir(path);

    printf("\n\nNFTW:\n -----------------------------------------------\n");
    nftw(path, &nftw_print_info, 100, FTW_PHYS);
    
    free(given_time);

    return 0;
}