#include "pgmlib.h"


void load_image(char* path, struct image *buffer){
    FILE *file = fopen(path, "r");
    if(file == NULL){
        printf("Error while opening file under path: %s\n%s", path, strerror(errno));
        exit(-1);
    }

    int width, height, max_pixel_val, tmp;
    tmp = fscanf(file, "P2\n%i %i\n%i\n", &width, &height, &max_pixel_val);
    // unsigned char* tab = malloc(width*height*sizeof(unsigned char));
    
    buffer->N = width;
    buffer->M = height;
    buffer->max = max_pixel_val;
    buffer->matrix = calloc(buffer->M, sizeof(int*));

    int val;
    for(int i=0; i<buffer->M; i++){
        buffer->matrix[i] = calloc(buffer->N, sizeof(int));
        for(int j=0; j<buffer->N; j++){
            tmp = fscanf(file, "%i ", &val);
            buffer->matrix[i][j] = val;
        }
    }
    tmp = tmp;
    fclose(file);
}

// void load_image_from_P5(char* path, struct image* buffer);
// void P5_to_P2(char* path_P5, char* path_P2);

void save_image(char* path, struct image* image){
    FILE* file = fopen(path, "w");
    if(file == NULL){
        printf("Error while opening file under path: %s\n%s", path, strerror(errno));
        exit(-1);
    }

    fprintf(file, "P2\n%i %i\n%i\n", image->N, image->M, image->max);
    for(int i=0; i<image->M; i++){
        for(int j=0; j<image->N; j++)
            fprintf(file, "%i ", image->matrix[i][j]);
        fprintf(file, "\n");
    }

    fclose(file);  
}

void load_pattern(char* path, struct pattern* buffer){
    FILE *file = fopen(path, "r");
    if(file == NULL){
        printf("Error while opening file under path: %s\n%s", path, strerror(errno));
        exit(-1);
    }

    int c, tmp;
    tmp = fscanf(file, "R2\n%i\n", &c);
    // double* tab = malloc(c*c*sizeof(double));
    
    buffer->C = c;
    buffer->matrix = calloc(buffer->C, sizeof(double*));

    double val;
    for(int i=0; i<buffer->C; i++){
        buffer->matrix[i] = calloc(buffer->C, sizeof(double));
        for(int j=0; j<buffer->C; j++){
            tmp = fscanf(file, "%lf ", &val);
            buffer->matrix[i][j] = val;
        }
    }
    tmp = tmp;
    fclose(file);
}

void save_pattern(char* path, struct pattern* pattern){
    FILE* file = fopen(path, "w");
    if(file == NULL){
        printf("Error while opening file under path: %s\n%s", path, strerror(errno));
        exit(-1);
    }

    fprintf(file, "R2\n%i\n", pattern->C);
    for(int i=0; i<pattern->C; i++){
        for(int j=0; j<pattern->C; j++)
            fprintf(file, "%lf ", pattern->matrix[i][j]);
        fprintf(file, "\n");
    }

    fclose(file); 
}

int image_2_pattern(struct image* image, struct pattern* pattern){
    if(image->N != image->M){
        printf("Cannot convert - image shape is not a quadratic\n");
        return -1;
    }

    pattern->C = image->N;
    pattern->matrix = calloc(pattern->C, sizeof(double*));
    long long int sum = 0;
    for(int i=0; i<pattern->C; i++){
        pattern->matrix[i] = calloc(pattern->C, sizeof(double));
        for(int j=0; j<pattern->C; j++){
            pattern->matrix[i][j] = (double)(image->matrix[i][j]);
            sum += image->matrix[i][j];
        }
    }

    for(int i=0; i<pattern->C; i++)
        for(int j=0; j<pattern->C; j++)
            pattern->matrix[i][j] /= sum;       

    return 0;
}


void print_image(struct image *image){
    printf("%i %i\n", image->M, image->N);
    for(int i=0; i<image->M;i++){
        printf("\n");
        for(int j=0; j<image->N; j++)
            printf("%i ", image->matrix[i][j]);
    }
}

struct image* image_init(int N, int M){
    struct image* image= malloc(sizeof(struct image));
    image->M = M;
    image->N = N;
    image->matrix = calloc(image->M, sizeof(int*));
    for(int i=0; i<image->M; i++) 
        image->matrix[i] = calloc(image->N, sizeof(int));
    return image;
}

struct pattern* pattern_init(int C){
    struct pattern* pattern = malloc(sizeof(struct pattern));
    pattern->C = C;
    pattern->matrix = calloc(C, sizeof(double*));
    for(int i=0; i<C; i++){
        pattern->matrix[i] = calloc(C, sizeof(double));
    }
    return pattern;
}