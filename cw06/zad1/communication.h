#ifndef COMUNICATION_H
#define COMUNICATION_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define KEY_LETTER 'c'
#define MAX_FRIENDS_NUMB 10
#define MAX_CLIENTS_NUMB 25
#define MESSAGE_SIZE 1024
#define MAX_COMMAND_ID 10
#define MAX_COMMAND_SIZE 1024


enum COMMAND{
    STOP = 1,
    INIT,
    LIST,
    FRIENDS,
    ADD_FRIENDS,
    DEL_FRIENDS,
    SEND_2_ALL,
    SEND_2_FRIENDS,
    SEND_2_ONE,
    ECHO
};

struct MESSAGE {
    long command_type;
    // enum COMMAND command_type;
    int senderID;
    char message[MESSAGE_SIZE];
};

#define msg_size sizeof(struct MESSAGE)-sizeof(long)

key_t getServerQueueKey();
key_t getClientQueueKey();


#endif