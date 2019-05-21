#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


struct image{
    int** matrix;
    int M;          //height of image - number of rows
    int N;          //width of image - number of columns
    int max;
};

struct pattern{
    double** matrix;
    int C;
};


void load_image(char* path, struct image* buffer);
// void load_image_from_P5(char* path, struct image* buffer);
// void P5_to_P2(char* path_P5, char* path_P2);
void save_image(char* path, struct image* image);
void load_pattern(char* path, struct pattern* buffer);
void save_pattern(char* path, struct pattern* pattern);
int image_2_pattern(struct image* image, struct pattern* pattern);
void print_image(struct image *image);
struct image* image_init(int N, int M);
struct pattern* pattern_init(int C);