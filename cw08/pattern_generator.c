#include "pgmlib.h"

int main(int argc, char** argv){
    
    if(argc < 2){
        printf("Too few arguments in pragram call\n");
        exit(-1);
    }

    int size = atoi(argv[1]);

    struct pattern* pat = pattern_init(size);
    
    double sum = 0;
    for(int i=0; i<size; i++){
        for(int j=0; j<size; j++){
            pat->matrix[i][j] = (double)(rand()) / RAND_MAX;
            sum += pat->matrix[i][j];
        }
    }

    for(int i=0; i<size; i++){
        for(int j=0; j<size; j++){
            pat->matrix[i][j] /= sum;
        }
    }

    char* name = calloc(100, sizeof(char));
    sprintf(name, "./patterns/pattern_%ix%i", size, size);
    save_pattern(name, pat);

    return 0;
}