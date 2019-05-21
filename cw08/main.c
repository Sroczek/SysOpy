#include <ctype.h> //for toupper function
#include <math.h>
#include <pthread.h>
#include "pgmlib.h"
#include <sys/time.h>

double get_current_time() {
  struct timeval curr;
  gettimeofday(&curr, NULL);
  return curr.tv_sec + curr.tv_usec * 1.0 / 1e6;
}

struct arg{
    struct image* input_image;
    struct pattern* pattern;
    int number_of_threads;
    int k;
};

enum mode{
    BLOCK,
    INTERLEAVED
}typedef mode;

struct image output_image;

void to_upper(char *str){
    for(int i=0; i<strlen(str); i++) str[i] = (char)(toupper(str[i]));
}

mode str_to_mode(char* str){
    to_upper(str);   
    if(strcmp(str, "BLOCK") == 0) return BLOCK;
    else if (strcmp(str, "INTERLEAVED") == 0) return INTERLEAVED;
    else{
        printf("Illegal value of \'mode\'\t try one of: BLOCK, INTERLEAVED\n");
        exit(-1);
    }
}

int splot(struct image im, struct pattern pat, int x, int y){
    int C = pat.C;
    int ceil = C/2;
    double splt = 0;
    int x_cor;
    int y_cor;

    for(int i=0; i<C; i++){
        for(int j=0; j<C; j++){
            x_cor = x - ceil + i;
            y_cor = y - ceil + j;
            
            if(x_cor < 0) x_cor=0;
            if(y_cor < 0) y_cor=0;
            if(x_cor >= im.M) x_cor = im.M -1;
            if(y_cor >= im.N) y_cor = im.N -1;
            
            splt += im.matrix[x_cor][y_cor] * pat.matrix[i][j];
        }
    }

    // printf("%lf  ", splt);
    int res = (int)(round(splt));
    return res; 
}

void* splot_block(void* arg){
    double t1 = get_current_time();

    struct arg* a = (struct arg*)(arg);
    int start = a->k*ceil(a->input_image->N/a->number_of_threads);
    int end = (a->k+1)*ceil(a->input_image->N/a->number_of_threads) -1;
    for(int i=start; i<=end; i++){
        for(int j=0; j<a->input_image->M; j++){
            output_image.matrix[j][i] = splot(*(a->input_image), *(a->pattern), j, i);
        }
    }
    
    double* time = malloc(sizeof(double));
    *time = get_current_time() - t1;
    pthread_exit(time);
}

void* splot_interleaved(void* arg){
    double t1 = get_current_time();

    struct arg* a = (struct arg*)(arg);
    // int start = a->k*ceil(a->input_image->N/a->number_of_threads);
    // int end = (a->k+1)*ceil(a->input_image->N/a->number_of_threads) -1;
    for(int i=a->k; i< a->input_image->N; i += a->number_of_threads){
        for(int j=0; j<a->input_image->M; j++){
            output_image.matrix[j][i] = splot(*(a->input_image), *(a->pattern), j, i);
        }
    }
    
    double* time = malloc(sizeof(double));
    *time = get_current_time() - t1;
    pthread_exit(time);
}


int main(int argc, char** argv){

    if(argc < 6){
        printf("Too few arguments in program call\nnumber_of_threads\tmode\tinput_picture_path\tpattern_picture_path\toutput_picture_path\n");
        exit(-1);
    }

    int number_of_threads = atoi(argv[1]);
    if(number_of_threads < 1){
        printf("Number of thread cannot be lower than 0\n");
        exit(-1);
    }
    
    mode m = str_to_mode(argv[2]);

    struct image input_image;
    struct pattern pattern;
    load_image(argv[3], &input_image);
    load_pattern(argv[4], &pattern);

    output_image = *image_init(input_image.N, input_image.M);


    pthread_t* threads = calloc(number_of_threads, sizeof(pthread_t));
    double** threads_times = calloc(number_of_threads, sizeof(double*));
    struct arg* args = calloc(number_of_threads, sizeof(struct arg));

    double before = get_current_time();
    for(int i=0; i<number_of_threads; i++){
        args[i].input_image = &input_image;
        args[i].pattern = &pattern;
        args[i].number_of_threads = number_of_threads;
        args[i].k = i;
        switch(m){
            case BLOCK:
                pthread_create(&threads[i], NULL, splot_block, &args[i]);
                break;
            case INTERLEAVED:
                pthread_create(&threads[i], NULL, splot_interleaved, &args[i]);
                break;
        }
    }


    for(int i=0; i<number_of_threads; i++){
        pthread_join(threads[i], (void**)&threads_times[i]);
    }

    double after = get_current_time();

    printf("----------------------------------------------------\n");
    printf("Size of pattern: %ix%i       Number of treads: %i\n\n", pattern.C, pattern.C, number_of_threads);
    // printf("----------------------------------------------------\n");

    for(int i=0; i<number_of_threads; i++){
        printf("Thread number %i have worked for %lf s\n", i, *threads_times[i]);
    }

    printf("Total time: %lf s\n", after - before);


    FILE* file = fopen("./Times.txt", "a");

    fprintf(file,"\n----------------------------------------------------\n");
    fprintf(file,"Size of pattern %ix%i       Number of treads: %i\n\n", pattern.C, pattern.C, number_of_threads);
    // fprintf(file,"----------------------------------------------------\n");

    for(int i=0; i<number_of_threads; i++){
        fprintf(file,"Thread number %i have worked for %lf s\n", i, *threads_times[i]);
    }

    fprintf(file, "\nTotal time: %lf s\n", after - before);


    output_image.max = input_image.max;             //math :D 
    save_image(argv[5], &output_image);
}