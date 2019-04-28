#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/times.h>

enum operation {generate, sort, copy, sort_l, copy_l};
 
unsigned char* generate_record(int record_size){
    unsigned char* res = calloc(record_size,sizeof(unsigned char));
    struct timespec *ts = malloc(sizeof(struct timespec));
    
    clock_gettime(0, ts);
    
    srand((ts->tv_nsec));

    for(int i=0;i<record_size;i++) res[i] = (unsigned char)(rand()% 256);
    //res[record_size-1]='\n

    free(ts);
    return res;
}

void generate_file(char* file_name, int records_number, int record_size){
    FILE* file;
    file = fopen(file_name, "w+");


    if(file == NULL){
        printf("Error while opening file\n");
        exit(-1);
    }

    for(int i=0; i<records_number; i++) {
        unsigned char *tmp = generate_record(record_size);
        if(fwrite(tmp, sizeof(unsigned char), record_size, file) != record_size) printf("Error while writing to file\n");
        free(tmp);
    }

    fclose(file);
}

void sort_sys(char* file_name, int records_number, int record_size){
    int f;
    char* record1 = calloc(record_size, sizeof(char));
    char* record2 = calloc(record_size, sizeof(char));

    char curr;
    char prev;
    int min;

    f = open(file_name, O_RDWR);

    for(int i=0; i<records_number-1; i++){
        min = i;
        if(read(f,record1,record_size)!= record_size) printf("Error while reading file\n");
        prev = record1[0];
        for(int j=i+1; j<records_number; j++){
            if(read(f,&curr,1)!= record_size) break;
            if((int)curr < (int)prev){
                min = j;
                prev = curr;
            }
            lseek(f,(j+1)*record_size, 0);
        }

        if(min != i){
            lseek(f,min*record_size,0);
            if(read(f, record2, record_size)!= record_size) printf("Error while reading file\n");

            lseek(f,i*record_size,0);
            if(write(f,record2,record_size)!= record_size) printf("Error while writing to file\n");

            lseek(f,min*record_size,0);
            if(write(f,record1,record_size)!= record_size) printf("Error while writing to file\n");
        }

        
        lseek(f,(i+1)*record_size,0);
    }
    close(f);
}

void sort_lib(char* file_name, int records_number, int record_size){
    FILE* file;
    file = fopen(file_name, "r+");

    char* record1 = calloc(record_size, sizeof(char));
    char* record2 = calloc(record_size, sizeof(char));

    char curr;
    char prev;
    int min;


    for(int i=0; i<records_number-1; i++){
        min = i;
        if(fread(record1,1,record_size,file) != record_size) printf("Error while reading file\n");
        prev = record1[0];
        for(int j=i+1; j<records_number; j++){
            if(fread(&curr,1,1,file) != record_size) break;
            if((int)curr < (int)prev){
                min = j;
                prev = curr;
            }
            fseek(file,(j+1)*record_size, 0);
        }

        if(min != i){
            fseek(file,min*record_size,0);
            if(fread(record2,1,record_size,file) != record_size) printf("Error while reading file\n");


            fseek(file,i*record_size,0);
            if(fwrite(record2,1,record_size,file)!= record_size) printf("Error while writing to file\n");

            fseek(file,min*record_size,0);
            if(fwrite(record1,1,record_size,file)!= record_size) printf("Error while writing to file\n");
        }

        fseek(file,(i+1)*record_size,0);
        
    }
    free(record1);
    free(record2);
    fclose(file);
}

void copy_sys(char* file_name, char* copy_name, int records_number, int record_size){
    int file, copy; 
    char* buffer = calloc(record_size,sizeof(char));

    file = open(file_name,O_RDONLY);
    copy = open(copy_name, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

    for(int i=0; i<records_number; i++){
        if(read(file,buffer,record_size)!= record_size) printf("Error while reading file\n");
        if(write(copy,buffer,record_size)!= record_size) printf("Error while writing to file\n");
    }

    close(file);
    close(copy);
}

void copy_lib(char* file_name, char* copy_name, int records_number, int record_size){
    FILE *file, *copy; 
    char* buffer = calloc(record_size,sizeof(char));

    file = fopen(file_name, "r");
    copy = fopen(copy_name,"w");

    for(int i=0; i<records_number; i++){
        if(fread(buffer,1,record_size,file) != record_size) printf("Error while reading file\n");
        if(fwrite(buffer,1,record_size,copy) != record_size) printf("Error while writing to file\n");;
    }

    free(buffer);
    fclose(file);
    fclose(copy);
}

inline void print_available_operations(){
    printf("generate <file name> <number of records> <length of single record>\n");
    printf("sort <file name> <number of records> <length of single record> [sys|lib]\n");
    printf("generate <file name> <copy name> <number of records> <size of buffer> [sys|lib]\n");
}

enum operation get_operation_from_string(char *str){
    if(strcmp(str,"generate") == 0) return 0;
    if(strcmp(str,"sort") == 0) return 1;
    if(strcmp(str,"copy") == 0) return 2;

    printf("\nNot supported operation.\n\nTry again with one of the folowing:\n");
    print_available_operations();
    exit(-1);
}

inline void is_int_greater_than_zero(char* str, int arg_numb){
    if(atoi(str) == 0){
        printf("Illegal value of argument %i\n", arg_numb);
        exit(-1);
    }
}
inline void is_lib_or_sys(char* str, int arg_numb){
    if(!(strcmp(str,"lib") == 0) && !(strcmp(str,"sys") == 0)){
        printf("Illegal value of argument %i\n", arg_numb);
        exit(-1);
    }
}

enum operation check_argumnets(int argc, char **argv){

    if(argc < 2){
        printf("To few argument in program call");
        exit(-1);
    }

    enum operation op = get_operation_from_string(argv[1]);

    switch (op)
    {
        case generate:
            if(argc < 5){
                printf("To few argument in program call\n");
                printf("Try again using following syntax:\n");
                print_available_operations();
                exit(-1);
            }
            is_int_greater_than_zero(argv[3],3);
            is_int_greater_than_zero(argv[4],4);
            break;

        case sort:
            if(argc < 6){
                printf("To few argument in program call\n");
                printf("Try again using following syntax:\n");
                print_available_operations();
                exit(-1);
            }
            is_int_greater_than_zero(argv[3],3);
            is_int_greater_than_zero(argv[4],4);
            is_lib_or_sys(argv[5],5);
            if(strcmp(argv[5],"lib") == 0) op = sort_l;
            break;
        case copy:
            if(argc < 7){
                printf("To few argument in program call\n");
                printf("Try again using following syntax:\n");
                print_available_operations();
                exit(-1);
            }
            is_int_greater_than_zero(argv[4],4);
            is_int_greater_than_zero(argv[5],5);
            is_lib_or_sys(argv[6],6);
            if(strcmp(argv[6],"lib") == 0) op = copy_l;
            break;
        default:
            break;
    }

    return op;
}

void execute(enum operation op, char **argv){
    switch (op)
    {
        case generate:
            generate_file(argv[2],atoi(argv[3]),atoi(argv[4]));
            break;
        case sort:
            sort_sys(argv[2],atoi(argv[3]),atoi(argv[4]));
            break;
        case sort_l:
            sort_lib(argv[2],atoi(argv[3]),atoi(argv[4]));
            break;
        case copy:
            copy_sys(argv[2],argv[3],atoi(argv[4]),atoi(argv[5]));
            break;
        case copy_l:
            copy_lib(argv[2],argv[3],atoi(argv[4]),atoi(argv[5]));
            break;
        default:
            break;
    }
}

void print_times_to_file(char* command, clock_t real_start, struct tms tms_start, clock_t real_end, struct tms tms_end){
    FILE* file;
    file = fopen("wyniki.txt","r+");
    if(file == NULL) file = fopen("wyniki.txt","w");
    fseek(file,0,SEEK_END);
    char *line = calloc(60,sizeof(char));
    strcat(line,command);
    for(int i=0; i<54-strlen(command);i++)
    strcat(line," ");
    fprintf(file,"%s", line);
    fprintf(file,"%.3lf    ", (double)(tms_end.tms_utime - tms_start.tms_utime)/sysconf(_SC_CLK_TCK));
    fprintf(file,"%.3lf\n", (double)(tms_end.tms_stime - tms_start.tms_stime)/sysconf(_SC_CLK_TCK));
    fclose(file);
    free(line);
}

char* build_command(enum operation op, char **argv){
    int n;
    if(op == 0) n=4;
    if(op == 1 || op == 3) n=5;
    if(op == 2 || op == 4) n=6;

    int size = 0;
    for(int i=1; i<=n; i++) size += strlen(argv[i])+1;

    char* res = calloc(size - 1 ,sizeof(char));
    for(int i=1; i<n; i++) {
        strcat(res,argv[i]);
        strcat(res," ");
    }
    strcat(res,argv[n]);

    return res;
}

int main(int argc, char **argv){
    
    enum operation op = check_argumnets(argc,argv);
    execute(op,argv);

    struct tms* tms_start = malloc(sizeof(struct tms));
    struct tms* tms_end = malloc(sizeof(struct tms));


    clock_t real_start = times(tms_start);
    execute(op,argv);
    clock_t real_end = times(tms_end);

    char* command = build_command(op, argv);

    printf("%s\n",command);
    
    if(op == sort || op==sort_l) print_times_to_file(command, real_start, *tms_start, real_end, *tms_end);
        
    free(command);

    free(tms_end);
    free(tms_start);
    return 0;
}