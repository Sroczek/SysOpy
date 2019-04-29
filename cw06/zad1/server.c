#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include "communication.h"


struct client{
    int clientID;
    int queue_key;
    pid_t pid;
    struct client* friends[MAX_FRIENDS_NUMB];
    int friends_numb;
};

struct client clients[MAX_CLIENTS_NUMB];
int server_queueID = -1;
int is_running = 1;

int exist_client(int clientID){
    return clientID >= 0 && clientID < MAX_CLIENTS_NUMB && clients[clientID].queue_key != -1;
}

char* get_date(){
    time_t curr_time;
    time(&curr_time);
    char* buffer = calloc(21,sizeof(char));
    strftime(buffer,21,"%Y-%m-%d_%H-%M-%S ",localtime(&curr_time));
    return buffer;
}

int send_from_server(int clientID, enum COMMAND command_type, char message[MESSAGE_SIZE]){
    struct MESSAGE msg;
    msg.command_type = command_type;
    strcpy(msg.message, message);
    msg.senderID = -1;                      //-1 stands for server
    if (!exist_client(clientID)) {
        printf("WRONG CLIENT ID\n");
        return 1;
    }

    if (msgsnd(clients[clientID].queue_key, &msg, msg_size, IPC_NOWAIT)) {
        printf("Error while sending to client\n");
        return 1;
    }
    return 0;
}

int send_from_client(int clientID, int senderID, enum COMMAND command_type, char message[MESSAGE_SIZE]){
    struct MESSAGE msg;
    msg.command_type = command_type;
    strcpy(msg.message, message);
    msg.senderID = senderID; 
    if (!exist_client(clientID)) {
        printf("WRONG CLIENT ID\n");
        return 1;
    }

    if (msgsnd(clients[clientID].queue_key, &msg, msg_size, IPC_NOWAIT)) {
        printf("Error while sending\n");
        return 1;
    }
    return 0;
}

void init(int clientID, char message_text[MESSAGE_SIZE]){
    int i=0;
    while(i<MAX_CLIENTS_NUMB && clients[i].pid != -1) i++;
    
    if(clients[i].pid != -1){
        printf("Cannot add more clients\n");
        return;
    }
    printf("NANA %i\n",i);
    
    int client_queue_key;
    sscanf(message_text, "%i", &client_queue_key);


    clients[i].clientID = i;
    clients[i].pid = clientID;
    clients[i].queue_key = client_queue_key;
    clients[i].friends_numb = 0;   //in case of reuse this field in list

    char receive[MESSAGE_SIZE];
    sprintf(receive, "%i", i);
    send_from_server(i, INIT, receive);
}

void echo(int clientID, char message_text[MESSAGE_SIZE]){
    char *date = get_date();
    char response[MESSAGE_SIZE];
    sprintf(response, "%s %s", date, message_text);
    send_from_server(clientID, ECHO, response);
}

void list(int clientID){
    char response[MESSAGE_SIZE];
    char buffer[MESSAGE_SIZE];
    for(int i=0; i<MAX_CLIENTS_NUMB; i++){
        if(clients[i].pid != -1){
            sprintf(buffer, "ID: %ld Key: %i\n", (long int)clients[i].pid, clients[i].queue_key);
            strcat(response, buffer);
        } 
    }
    send_from_server(clientID, LIST, response);
}

void add_friend(int clientID, struct client* friend){
    int first_free_place = -1;
    for(int i=0; i<MAX_FRIENDS_NUMB; i++){
        if(clients[clientID].friends[i] == NULL && first_free_place == -1){
            if(clients[clientID].friends[i] == friend) return;
            first_free_place = i;
        }
    }
    if(first_free_place == -1){
        printf("Cannot add more friends\n");
        return;
    }
    clients[clientID].friends[first_free_place] = friend;
    clients[clientID].friends_numb++;
}

void delete_friend(int clientID, struct client* friend){
    for(int i=0; i<MAX_FRIENDS_NUMB; i++){
        if(clients[clientID].friends[i] == friend){
            clients[clientID].friends[i] = NULL;
            clients[clientID].friends_numb--;
            return;
        }
    }
    printf("Cannot delete friend which is not friend of given client\n");
}

void add_friends(int clientID, char message_text[MESSAGE_SIZE]){
    char delim[] = " ";
    char *tmp;
    tmp = strtok(message_text,delim);
    while(tmp != NULL){
        clientID = strtol(tmp, NULL, 10);
        if(!exist_client(clientID)){
            printf("Wrong client ID\n");
        }
        else{
            add_friend(clientID, &clients[clientID]);
        }
        tmp = strtok(NULL, delim);
    }
}

void delete_friends(int clientID, char message_text[MESSAGE_SIZE]){
    char delim[] = " ";
    char *tmp;
    tmp = strtok(message_text,delim);
    while(tmp != NULL){
        clientID = strtol(tmp, NULL, 10);
        if(!exist_client(clientID)){
            printf("Wrong client ID\n");
        }
        else{
            delete_friend(clientID, &clients[clientID]);
        }
        tmp = strtok(NULL, delim);
    }
}

void clean_friends(int clientID){
    for(int i=0;i<MAX_FRIENDS_NUMB; i++) clients[clientID].friends[i] = NULL;
}

void friends(int clientID, char message_text[MESSAGE_SIZE]){
    clean_friends(clientID);
    char delim[] = " ";
    char *tmp;
    tmp = strtok(message_text,delim);
    while(tmp != NULL){
        clientID = strtol(tmp, NULL, 10);
        if(!exist_client(clientID)){
            printf("Wrong client ID\n");
        }
        else{
            add_friend(clientID, &clients[clientID]);
        }
        tmp = strtok(NULL, delim);
    }
}

void send_2_one(int clientID, char message_text[MESSAGE_SIZE]){
    char buffer[16];
    int i = 0;
    for(; i<16; i++){
        if(message_text[i] == ' ') break;
        buffer[i] = message_text[i];
    }

    int receiverID = strtol(buffer, NULL, 10);

    char *date = get_date();
    char dated_message[MESSAGE_SIZE];
    sprintf(dated_message, "%s %s", date, message_text);

    send_from_client(receiverID, clientID, SEND_2_ONE, dated_message);
}

void send_2_friends(int clientID, char message_text[MESSAGE_SIZE]){
    for(int i=0; i<MAX_FRIENDS_NUMB; i++){
            if(exist_client(clients[clientID].friends[i]->clientID)){
            char *date = get_date();
            char dated_message[MESSAGE_SIZE];
            sprintf(dated_message, "%s %s", date, message_text);

            send_from_client(clients[clientID].friends[i]->clientID, clientID, SEND_2_FRIENDS, dated_message);
        }
    }
}

void send_2_all(int clientID, char message_text[MESSAGE_SIZE]){
    for(int i=0; i<MAX_CLIENTS_NUMB; i++){
        if(exist_client(i)){
            char *date = get_date();
            char dated_message[MESSAGE_SIZE];
            sprintf(dated_message, "%s %s", date, message_text);

            send_from_client(clients[i].clientID, clientID, SEND_2_FRIENDS, dated_message);
        }
    }
}

void handle_message(struct MESSAGE* msg){
    switch(msg->command_type){
        case INIT:
            init(msg->senderID, msg->message);
            break;
        case LIST:
            list(msg->senderID);
            break;
        case ECHO:
            echo(msg->senderID, msg->message);
            break;
        case FRIENDS:
            friends(msg->senderID, msg->message);
            break;
        case ADD_FRIENDS:
            add_friends(msg->senderID, msg->message);
            break;  
        case DEL_FRIENDS:
            delete_friends(msg->senderID, msg->message);
            break;  
        case SEND_2_ONE:
            send_2_one(msg->senderID, msg->message);
            break;
        case SEND_2_FRIENDS:
            send_2_friends(msg->senderID, msg->message);
            break;
        case SEND_2_ALL:
            send_2_all(msg->senderID, msg->message);
            break;
        default :
            printf("Unknown command\n");
            break;                    
    }
}

int main(int argc, char** argv){

    for(int i=0; i<MAX_CLIENTS_NUMB;i++){
        clients[i].pid = -1;
        clients[i].clientID = -1;
        clients[i].queue_key = -1;
        clients[i].friends_numb = 0;
    }

    server_queueID = msgget(getServerQueueKey(), IPC_PRIVATE | IPC_EXCL | 0666);
    if( server_queueID == -1){
        printf("Error while creating queue: %s\n", strerror(errno));
        exit(-1);
    }

    struct MESSAGE buf;
    while(is_running){
        msgrcv(server_queueID, &buf, msg_size, 0, 0);
        handle_message(&buf);
    }
    
    // some cleanings TODO
    return 0;
}