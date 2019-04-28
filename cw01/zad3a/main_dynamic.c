
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

#include <dlfcn.h>

void* handle;

typedef struct{
    int size;
    char **array;
    char *current_dir;
    char *searching_file;
} find_arr;

int DEBUG1 = 0;

int execute(char** args, int* index, int argc, find_arr* find_array){   // if execute return 1 else 0

    void (*set_searching_parameters)(find_arr*,char*,char*) = dlsym(handle, "set_searching_parameters");
    void (*search)(find_arr*,char*) = dlsym(handle, "search");
    int (*load_searching_result)(find_arr*,char*) = dlsym(handle,"load_searching_result");
    int (*delete)(find_arr*,int) = dlsym(handle,"delete");
    int (*free_slots)(find_arr*) = dlsym(handle, "free_slots");
    void (*print)(find_arr*) = dlsym(handle,"print");

    int i = *index;
    if(strcmp(args[i],"search_directory") == 0){
            if(i+3<argc){
                set_searching_parameters(find_array, args[i+1], args[i+2]);
                search(find_array, args[i+3]);
                i+=4;
                *index = i;
                return 1;
            }
            else{
                printf("There to few aguments in operation \'search_directry\'\n");
            }

            i++;
            if(DEBUG1) print(find_array);
        }
        else{ 
            if(strcmp(args[i],"load_block_from_file") == 0){
                if(i+1<argc){
                    load_searching_result(find_array, args[i+1]);
                    i+=2;
                    *index = i;
                    return 1;
                }
                else{
                    printf("There to few aguments in operation \'load_block_from_file\'\n");
                }
                i++;
                if(DEBUG1) print(find_array);
            }
            else{
            if(strcmp(args[i],"delete_block") == 0){
                if(i+1<argc){
                    delete(find_array, (int)(strtol(args[i+1],'\0', 10)));
                    i+=2;
                    *index = i;
                    return 1;
                }
                else{
                    printf("There to few aguments in operation \'delete_block\'\n");
                }
            
                i++;
                if(DEBUG1) print(find_array);
            }
            else{
                if(strcmp(args[i],"repeat_load_and_remove")==0){
                    if(i+3<argc){
                        int numb_of_blocks = (int)(strtol(args[i+2],'\0', 10));
                        int* indexes = calloc(numb_of_blocks,sizeof(int));
                        int repeat = (int)(strtol(args[i+3],'\0', 10));

                        if(numb_of_blocks > free_slots(find_array)){
                            printf("Declared of blocks to load must be lower than actual number of free slots in block_array\n");
                            *index = i+1; 
                            return 0;
                        }

                        for(int j=0; j<repeat;j++){
                            for(int k=0; k<numb_of_blocks; k++) indexes[k] = load_searching_result(find_array, args[i+1]);
                            for(int k=0; k<numb_of_blocks; k++) delete(find_array,indexes[k]);
                        }
                        free(indexes);
                        i+=4;
                        *index = i;
                        return 1;
                    }    
                    else{
                        printf("There to few aguments in operation \'repeat_load_and_remove\'\n");
                    }

                    if(DEBUG1) print(find_array);
                    }
            }
            }
        }
    *index = i+1;
    return 0;
}

char* build_command(char** argv, int first, int last){
    int length = 0;
    for(int i = first; i <= last;i++) length += strlen(argv[i]) + 1;
    length--;

    char* res = calloc(length,sizeof(char));

    for(int i = first; i < last;i++){
        strcat(res,argv[i]);
        strcat(res," ");
    }
        strcat(res,argv[last]);

    return res;
}

void print_times(char* command, clock_t real_start, struct tms tms_start, clock_t real_end, struct tms tms_end){
    printf("\nCommand: %s \n", command);
    printf("Real: %.3lf  ", (double)(real_end - real_start)/sysconf(_SC_CLK_TCK));
    printf("User: %.3lf ", (double)(tms_end.tms_utime - tms_start.tms_utime)/sysconf(_SC_CLK_TCK));
    printf("System: %.3lf\n", (double)(tms_end.tms_stime - tms_start.tms_stime)/sysconf(_SC_CLK_TCK));
}

void print_times_to_file(char* command, clock_t real_start, struct tms tms_start, clock_t real_end, struct tms tms_end){
    FILE* file;
    file = fopen("raport2.txt","r+");
    if(file == NULL) file = fopen("raport2.txt","w");
    fseek(file,0,SEEK_END);
    fprintf(file, "\nCommand: %s \n", command);
    fprintf(file,"Real: %.3lf  ", (double)(real_end - real_start)/sysconf(_SC_CLK_TCK));
    fprintf(file,"User: %.3lf ", (double)(tms_end.tms_utime - tms_start.tms_utime)/sysconf(_SC_CLK_TCK));
    fprintf(file,"System: %.3lf\n", (double)(tms_end.tms_stime - tms_start.tms_stime)/sysconf(_SC_CLK_TCK));
    fclose(file);
}

int main(int argc, char** argv){

    if(argc < 2){
        printf("There is to few argument in program call");
        return 0;
    }

    handle = dlopen("./library.so", RTLD_LAZY);

    find_arr* (*create)(int) = dlsym(handle,"create");

    find_arr *find_array = create((int)(strtol(argv[1],'\0', 10)));

    struct tms* tms_start = malloc(sizeof(struct tms));
    struct tms* tms_end = malloc(sizeof(struct tms));

    for(int i=2; i<argc;){
        int i_copy = i;

        clock_t real_start = times(tms_start);
        int executed = execute(argv, &i, argc, find_array);
        clock_t real_end = times(tms_end);

        char* command;
        if(executed) command = build_command(argv,i_copy,i-1);
        else {
            command = malloc(sizeof(char)*strlen("Illegal command. It haven't been executed properly \n"));
            strcat(command, "Illegal command. It haven't been executed properly\n");
        }
       
        print_times(command, real_start, *tms_start, real_end, *tms_end);
        print_times_to_file(command, real_start, *tms_start, real_end, *tms_end);

        free(command);
    }

    dlclose(handle);
    free(tms_end);
    free(tms_start);
}