#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG = 0;

struct find_arr* create(int size){
    if(size <= 0){
        printf("Error: array size should be greater than 0");
        return NULL;
    }
    
    struct find_arr* res = malloc(sizeof(struct find_arr));

    res -> size = size;
    res -> array = calloc(size, sizeof(char*));
    res -> current_dir = "";
    res -> searching_file = "";

    if(DEBUG) printf("Size of created array: %i\n",size);
    return res;
}

void set_searching_parameters(struct find_arr *find_arr, char* directiory, char* file){
    if(find_arr == NULL) return;

    find_arr -> current_dir = directiory;
    find_arr -> searching_file = file;
}

void search(struct find_arr *find_arr, char* file_name){
    if(find_arr == NULL){
        printf("Error: ...");
        return;
    }

    int size = strlen(find_arr -> current_dir) + strlen(find_arr -> searching_file) + strlen("find  -name  >  2>  ") + 2*strlen(file_name);
    char* command = calloc(size,sizeof(char));
    strcat(command, "find ");
    strcat(command, find_arr -> current_dir);
    strcat(command, " -name '");
    strcat(command, find_arr -> searching_file);
    strcat(command, "' > ");
    strcat(command, file_name);
    strcat(command, " 2> ");
    strcat(command, file_name);

    if(DEBUG) printf("%s\n\n", command);

    if(system(command) == -1) printf("Command returned -1 (error)");

    free(command);
}

int load_searching_result(struct find_arr* find_arr, char* file_name){
    FILE *file = NULL;
    file = fopen(file_name,"r");
    if(file == NULL){
        printf("Error: błąd otwarcia pliku\n");
        exit(-10);
    }

    if(DEBUG) printf("File is working\n\n");

    fseek(file,0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* block = malloc(size*sizeof(char));

    char c = fgetc(file);
    int i = 0;
    while(c != EOF){
        block[i] = c;
        i++;
        c = fgetc(file);
    }
    
    fclose(file);

    if(DEBUG) printf("%s\n", block);

    i=0;
    while(i<find_arr->size && find_arr->array[i] != NULL) i++;
    if(find_arr->array[i] != NULL){
        printf("There ale no more slots in array\n");
        return -1;      //I'm not shure that it is a good idea to return -1. Perhabs it should end by exit.
    }

    find_arr->array[i] = block;

    return i;
}

int delete(struct find_arr* find_arr, int index){
    if(index >= find_arr->size || index < 0){
        printf("Index out of bound\n");
        return 0;
    }

    free(find_arr -> array[index]);
    find_arr -> array[index] = NULL;

    return 1;
}

void print(struct find_arr* find_array){
    if(find_array == NULL){
        printf("Error... array is null");
        return;
    }

    printf("Acctual state of size of blocs in find_array:\n");
    for(int i = 0;i< find_array -> size;i++){
        if(find_array->array[i] != NULL){
            printf("block %i: %i\n", i, (int)strlen(find_array->array[i]));
        }
        else{
            printf("block %i: NULL\n", i);
        }
    }
}

int free_slots(struct find_arr* find_array){
    int res=0;
    for(int i=0; i<find_array->size;i++){
        if(find_array->array[i] == NULL) res++;
    }
    return res;
}