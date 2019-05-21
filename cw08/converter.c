#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


void convert(char* path1, char* path2){
    printf("%s %s", path1, path2);
    FILE *file = fopen(path1, "r");
    if(file == NULL){
        printf("Error while opening file under path: %s\n%s", path1, strerror(errno));
        exit(-1);
    }

    int width, height, max_pixel_val;
    fscanf(file, "P5\n%i %i\n%i\n", &width, &height, &max_pixel_val);
    unsigned char* tab = malloc(width*height*sizeof(unsigned char));
    fread(tab, sizeof(unsigned char), width*height, file);
    fclose(file);
    
    FILE *res = fopen(path2, "w");
    if(res == NULL){
        printf("Error while opening file under path: %s\n%s", path2, strerror(errno));
        exit(-1);
    }
    
    fprintf(res, "P2\n%i %i\n%i", width, height, max_pixel_val);
    for(int j=0; j<width*height; j++){
        if(j%width == 0) fprintf(file, "\n");
        unsigned char c = tab[j];   
        // printf("%c ", c);
        fprintf(res, "%i ", c);
    }
    fclose(res);    
}

int main(int argc, char** argv){
    
    if(argc < 3){
        printf("Too few....\n");
        exit(-1);
    }
    
    convert(argv[1], argv[2]);
    
    return 0;
}