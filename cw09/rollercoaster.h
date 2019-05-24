#include<pthread.h>

struct Passenger{
    pthread_t id;
} typedef Passenger;

struct Cart{
    pthread_t id;
    Passenger* passengers;
    int capacity;
    int max_capacity;
} typedef Cart;

// Passenger* start(){

// }

// Cart* start(int max_capacity){
//     Cart* 
// }