#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/resource.h>

enum copy_mode {MEM, EXEC};

struct monitored_file{
    char *name;
    char *path;
    int refresh_frequency;
    time_t last_mtime;
    char *copy;
};

struct monitored_file* get_monitored_file(char* str){
    struct monitored_file *res = malloc(sizeof(struct monitored_file));

    char delim[] = " ";
    char *tmp;
    tmp = strtok(str,delim);
    if(tmp == NULL){
        free(res);
        return NULL;
    }
    res->name = calloc(strlen(tmp),sizeof(char));
    strcpy(res->name, tmp);

    tmp = strtok(NULL,delim);
    if(tmp== NULL){
        free(res->name);
        free(res);
        return NULL;
    }
    res->path = calloc(strlen(tmp),sizeof(char));
    strcpy(res->path, tmp);

    tmp = strtok(NULL,delim);
    if(tmp == NULL){
        free(res->name);
        free(res->path);
        free(res);
        return NULL;
    }

    res->refresh_frequency = atoi(tmp);

    res->copy = NULL;

    return res;
}

struct monitored_file** get_monitored_files_array(char *path, int *monitored_number){
    FILE *file;
    file = fopen(path, "r");

    if(file == NULL){
        printf("Cannot open file %s: \n", path);
        exit(-1);
    }

    //counting maximum number of monitored files 
    int lines_counter = 0;
    char *line = NULL;
    size_t size;
    while(getline(&line,&size,file) != -1){
       lines_counter++;
    }

    fseek(file,0,SEEK_SET);

    //creating returning array
    int monitors_counter = 0;
    struct monitored_file **monitored_files = calloc(lines_counter,sizeof(struct monitored_file*));
    for(int i = 0; i < lines_counter; i++){
        if(getline(&line, &size, file) == -1) {
            printf("There are no more lines to read.\n");
            break;
        }
        struct monitored_file *tmp = get_monitored_file(line);
        if(tmp != NULL){
            monitored_files[monitors_counter] = tmp;
            monitors_counter++;
        }
    }

    if(monitors_counter < lines_counter) if(realloc(monitored_files, monitors_counter * sizeof(struct monitored_file*)) == NULL) printf("Error during reallocation\n");
    
    *monitored_number = monitors_counter;
    return monitored_files;
}

void free_monitored_files_array(struct monitored_file **mf, int monitored_numb){
    for(int i=0; i<monitored_numb; i++){
        free(mf[i]->name);
        free(mf[i]->path);
        free(mf[i]->copy);
    }
}

time_t get_mtime(char *path){
    struct stat *st = malloc(sizeof(struct stat));
    if(lstat(path, st) == -1) {
        printf("File stat structure is not available\n");
        free(st);
        return 0;
    }
    time_t res = st->st_mtime;
    free(st);
    return res;
}

char* get_saving_file_name(struct monitored_file *mf){
    char *res = malloc(strlen("./archiwum/") + strlen(mf->name) + strlen("_RRRR-MM-DD_GG-MM-SS"));
    char buffer[21];
    strftime(buffer,21,"_%Y-%m-%d_%H-%M-%S",localtime(&mf->last_mtime));
    
    res = strcpy(res,"./archiwum/");
    strcat(res, mf->name);
    strcat(res, buffer);

    return res;
}

void copy_mem(struct monitored_file *mf){
    if(mf->copy != NULL){
        char *saving_file_name = get_saving_file_name(mf);
        FILE *file;
        file = fopen(saving_file_name,"w+");
        fprintf(file,"%s",mf->copy);

        fclose(file);

        free(mf->copy);
    }

    FILE *file;
    file = fopen(mf->path,"r");

    if(file == NULL){
        printf("Cannot open file %s\n",mf->path);
        exit(-1);
    }

    char c = getc(file);
    int file_size = 0;
    while(c != EOF){
        file_size++;
        c = getc(file);
    }

    mf->copy = calloc(file_size, sizeof(char));

    fseek(file,0,SEEK_SET);

    for(int i=0; i<file_size; i++) mf->copy[i] = getc(file);
    fclose(file);

}

void copy_exec(struct monitored_file *mf){
    pid_t child_pid;
    child_pid = fork();
    if(child_pid == 0){
        execlp("cp", "cp", mf->path, get_saving_file_name(mf), NULL);
    }
}

int monitor(struct monitored_file *mf, double time_of_running, enum copy_mode mode){
    int copy_counter = 0;
    double eps = 10e-7;
    
    time_t begin_time;
    time(&begin_time);

    time_t present_time;
    time(&present_time);

    time_t last_check_time;
    time(&last_check_time);

    mf->last_mtime = get_mtime(mf->path);

    switch (mode)
    {
        case MEM:
            copy_mem(mf);
            break;

        case EXEC:
            copy_exec(mf);
            copy_counter++;
            break;
    }


    while(difftime(present_time,begin_time) < time_of_running){

        if(difftime(present_time, last_check_time) > mf->refresh_frequency){
            //check if file has been modified since last check
            time_t tmp = get_mtime(mf->path);
            if( fabs(difftime(tmp,mf->last_mtime)) > eps){
                
                switch (mode)
                {
                    case MEM:
                        copy_mem(mf);
                        mf->last_mtime = tmp;
                        break;

                    case EXEC:
                        mf->last_mtime = tmp;
                        copy_exec(mf);
                        break;
                }

                copy_counter++;
            }
            time(&last_check_time);
        }

        time(&present_time);
    }

    return copy_counter;
}

int run_monitoring(struct monitored_file **monitored_files, int monitored_numb, double time_of_running, enum copy_mode mode,struct rlimit mem_limit, struct rlimit cpu_limit){
    for(int i=0; i<monitored_numb; i++){
        pid_t child_pid;
        child_pid = fork();
        if(child_pid == 0){
            setrlimit(RLIMIT_CPU, &cpu_limit);
            setrlimit(RLIMIT_AS, &mem_limit);
            return monitor(monitored_files[i], time_of_running, mode);
        }
    }
    
    for(int i=0; i<monitored_numb; i++){
        int stat;
        struct rusage all_children_rusage1;
        struct rusage all_children_rusage2;
        getrusage(RUSAGE_CHILDREN, &all_children_rusage1);
        pid_t child_pid = wait(&stat);
        getrusage(RUSAGE_CHILDREN, &all_children_rusage2);

        printf("Proces %ld utworzył %i kopii pliku\n", (long int)(child_pid), WEXITSTATUS(stat));
        long long int cpu_sys_time = (long long int)(all_children_rusage2.ru_stime.tv_sec - all_children_rusage1.ru_stime.tv_sec);
        long long int cpu_usr_time = (long long int)(all_children_rusage2.ru_utime.tv_sec - all_children_rusage1.ru_utime.tv_sec);
        printf("CPU_sys_usage: %lld s, CPU_usr_usage: %lld s \n", cpu_sys_time, cpu_usr_time);
    }
    
    free_monitored_files_array(monitored_files, monitored_numb);
    return 0;
}

void print_monitored_files(struct monitored_file **arr, int size){
    for(int i=0; i<size; i++){
        printf("name: %s\npath: %s\nf: %d\n\n", arr[i]->name, arr[i]->path, arr[i]->refresh_frequency);
    }
}

int main(int argc, char **argv){

    if(argc < 6){
        printf("To few argument in program call\n");
        exit(-1);
    }

    int monitored_numb = 0;
    struct monitored_file **monitored_files = get_monitored_files_array(argv[1], &monitored_numb);
    double time_of_running = (double)(atoi(argv[2]));

    enum copy_mode mode;
    if(strcmp(argv[3], "mem_cp") == 0) mode = MEM;
    else if(strcmp(argv[3], "exec_cp") == 0) mode = EXEC;
    else {
        printf("Ilegal value of argument 3\n");
        exit(-4);
    }

    if(strtol(argv[4],NULL,10) < 0){
        printf("Illegal value of argument 4. CPU time limit should be positive\n");
        exit(-4);
    }
    
    if(strtol(argv[5],NULL,10) < 0){
        printf("Illegal value of argument 5. Memory limit should be positive\n");
        exit(-4);
    }

    struct rlimit mem_limit;
    struct rlimit cpu_limit;

    cpu_limit.rlim_cur = mem_limit.rlim_max = (rlim_t)strtoull(argv[4],NULL,10);
    mem_limit.rlim_cur = mem_limit.rlim_max = (rlim_t)(strtoull(argv[5],NULL,10) * 1024 * 1024);

    return run_monitoring(monitored_files,monitored_numb,time_of_running,mode, mem_limit, cpu_limit);
}