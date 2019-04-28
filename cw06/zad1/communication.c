#include "communication.h"

key_t getServerQueueKey() {
    char *homeDir = getenv("HOME");
    if (homeDir == NULL){
        printf("No \"HOME\" variable");
        exit(1);
    }
    key_t key = ftok(homeDir, KEY_LETTER);
    if (key == -1){
        printf("Error while generating key");
    }
    return key;
}

key_t getClientQueueKey() {
    char *homeDir = getenv("HOME");
    if (homeDir == NULL){
        printf("No \"HOME\" variable");
        exit(1);
    }
    key_t key = ftok(homeDir, getpid());
    if (key == -1){
        printf("Error while generating key");
    }
    return key;
}
