#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

int is_running = 0;
struct monitored_file* curr;
int RTMIN_counter = 0;
int is_parent = 1;

struct monitored_file **g_monitored_files;
int g_monitored_numb;

struct monitored_file{
    char *name;
    char *path;
    int refresh_frequency;
    time_t last_mtime;
    char *copy;

    pid_t *pid;
    int is_monitored;
    int copy_counter;
};

enum operation {LIST, START_ALL, STOP_ALL, START_PID, STOP_PID, END, NONE};

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
    res->pid = NULL;
    res->is_monitored = 0;   //boolean
    res->copy_counter = 0;

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

void print_monitored_files_info(struct monitored_file **arr, int size){
    for(int i=0; i<size; i++){
        if(arr[i]->pid != NULL){
            printf("PID: %lld   Name: %s   Path: %s   Refresh: %i s  Copy_counter: %i   Is_monitored: %i\n",
                (long long int)(*arr[i]->pid),
                arr[i]->name,
                arr[i]->path,
                arr[i]->refresh_frequency,
                arr[i]->copy_counter,
                arr[i]->is_monitored);
        }
        else{
            printf("PID: null   Name: %s   Path: %s   Refresh: %i s  Copy_counter: %i   Is_monitored: %i\n",
                arr[i]->name,
                arr[i]->path,
                arr[i]->refresh_frequency,
                arr[i]->copy_counter,
                arr[i]->is_monitored);
        }
    }
    printf("\n");
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

void to_upper(char *str){
    for(int i=0; i<strlen(str); i++)  str[i] = (char)(toupper(str[i]));
}

void read_op_from_cmdline(enum operation *op,long int *PID){
    char buff[100];
    if(scanf("%s", buff) != 1) printf("Error while reading from commandd line\n");
    to_upper(buff);

    if(strcmp("LIST", buff) == 0){*op = LIST; return;}

    if(strcmp("LS", buff) == 0){*op = LIST; return;}

    if(strcmp("START", buff) == 0){
        if(scanf("%s", buff) != 1) printf("Error while reading from commandd line\n");
        to_upper(buff);

        if(strcmp("ALL",buff) == 0) *op = START_ALL;
        else{
            *op = START_PID;
            *PID = strtol(buff, NULL, 10);
        }
        return;
    }

    if(strcmp("STOP", buff) ==0){
        if(scanf("%s", buff) != 1) printf("Error while reading from commandd line\n");
        to_upper(buff);

        if(strcmp("ALL",buff) == 0) *op = STOP_ALL;
        else{
            *op = STOP_PID;
            *PID = strtol(buff, NULL, 10);
        }
        return;
    }

    if(strcmp("END", buff) == 0) {*op = END; return;}

    *op = NONE;
}

void execute_operation(struct monitored_file **mf, int monitored_numb, enum operation op, long int pid){
    
    switch (op)
    {
        case LIST:
            for(int i=0; i<monitored_numb; i++){
                kill(*mf[i]->pid, SIGRTMIN);
            }
            while(RTMIN_counter < monitored_numb) sleep(1);
            RTMIN_counter = 0;
            break;

        case START_ALL:
            for(int i=0; i<monitored_numb; i++){
                kill(*mf[i]->pid, SIGUSR1);
            }
            break;

        case START_PID:
            kill(pid, SIGUSR1);
            break;

        case STOP_ALL:
            for(int i=0; i<monitored_numb; i++){
                kill(*mf[i]->pid, SIGUSR2);
            }
            break;

        case STOP_PID:
            kill(pid, SIGUSR2);
            break;

        case END:
            for(int i=0; i<monitored_numb; i++){
                kill(*mf[i]->pid, SIGRTMIN);
            }

            while(RTMIN_counter < monitored_numb) sleep(1);
            RTMIN_counter = 0;

            for(int i=0; i<monitored_numb; i++){
                kill(*mf[i]->pid, SIGTERM);
            }

            exit(-1);
            break;

        case NONE:
            printf("Nieznana operacja\n");
            break;

    }
}

void monitor(struct monitored_file *mf){
    double eps = 10e-7;

    time_t present_time;
    time(&present_time);

    time_t last_check_time;
    time(&last_check_time);

    mf->last_mtime = get_mtime(mf->path);

    copy_mem(mf);

    while(1){
        if(is_running){
            mf->is_monitored = 1;
            if(difftime(present_time, last_check_time) > mf->refresh_frequency){
                //check if file has been modified since last check
                time_t tmp = get_mtime(mf->path);
                if( fabs(difftime(tmp,mf->last_mtime)) > eps){
                    copy_mem(mf);
                    mf->last_mtime = tmp;

                    mf->copy_counter++;
                }
                time(&last_check_time);
            }

            time(&present_time);
        }
        else{
            mf->is_monitored = 0;
        }
    }
}

int run_monitoring(struct monitored_file **monitored_files, int monitored_numb){
    for(int i=0; i<monitored_numb; i++){
        pid_t *child_pid = malloc(sizeof(pid_t));
        *child_pid = fork();
        if(*child_pid == 0){
            is_parent = 0;
            curr = monitored_files[i];
            monitor(monitored_files[i]);
        }
        monitored_files[i]->pid = child_pid;
    }
    
    print_monitored_files_info(monitored_files, monitored_numb);
    
    enum operation op;
    long int pid;
    while(1){
        read_op_from_cmdline(&op, &pid);
        execute_operation(monitored_files, monitored_numb, op, pid);
    }
    
    free_monitored_files_array(monitored_files, monitored_numb);
    return 0;
}

void SIGUSR1_handler(){
    is_running = 1;
}

void SIGUSR2_handler(){
    is_running = 0;
}

void SIGRTMIN_handler(int signal, siginfo_t *info, void* ucontext){
    if(!is_parent){
        printf("PID: %lld   Name: %s   Path: %s   Refresh: %i s  Copy_counter: %i   Is_monitored: %i\n",
                (long long int)(getpid()),
                curr->name,
                curr->path,
                curr->refresh_frequency,
                curr->copy_counter,
                curr->is_monitored);
        kill((long int)(info->si_pid), SIGRTMIN);
    }
    else RTMIN_counter++;
}

void SIGINT_handler(){
    execute_operation(g_monitored_files, g_monitored_numb, END, 0);
}

int main(int argc, char **argv){

    if(argc < 2){
        printf("To few argument in program call\n");
        exit(-1);
    }

    int monitored_numb = 0;
    struct monitored_file **monitored_files = get_monitored_files_array(argv[1], &monitored_numb);
    g_monitored_files = monitored_files;
    g_monitored_numb =monitored_numb;

    //print_monitored_files_info(monitored_files,monitored_numb);

    struct sigaction act1;
    act1.sa_handler = SIGUSR1_handler;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = 0;
    sigaction(SIGUSR1,&act1,NULL);

    struct sigaction act2;
    act2.sa_handler = (void *)SIGUSR2_handler;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = 0;
    sigaction(SIGUSR2,&act2,NULL);

    struct sigaction act3;
    act3.sa_sigaction = (void *) SIGRTMIN_handler;
    sigemptyset(&act3.sa_mask);
    act3.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN,&act3,NULL);

    struct sigaction act4;
    act4.sa_handler = SIGINT_handler;
    sigemptyset(&act4.sa_mask);
    act4.sa_flags = 0;
    sigaction(SIGINT,&act4,NULL);

    run_monitoring(monitored_files,monitored_numb);
}