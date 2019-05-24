#include <stdio.h>

void passenger(){

}

void cart(){

}


int main(int argc, char** argv){
    
    if(argc < 5){
        printf("Too few argunemts in program call\n");
        exit(-1);
    }

    int number_of_passengers = atoi(argv[1]);
    int number_of_carts = atoi(argv[2]);
    int cart_capacity = atoi(argv[3]);
    int number_of_rides = atoi(argv[4]);


}