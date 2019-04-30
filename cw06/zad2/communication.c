#include "communication.h"

unsigned commandPiority(enum COMMAND cmd){
    switch(cmd){
        case STOP:
            return 4;
        case INIT:
            return 3;
        case FRIENDS:
        case ADD_FRIENDS:
        case DEL_FRIENDS:
        case LIST:
            return 2;
        default:
            return 1;
    }
}

char* getClientQueueName() {
    char *name=malloc(32*sizeof(char));
    sprintf(name, "/cli%i%i", getpid(), rand()%1000);
    return name;
}

