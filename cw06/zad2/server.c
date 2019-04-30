#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h>
#include "communication.h"


struct client{
    int clientID;
    mqd_t queue_key;
    pid_t pid;
    struct client* friends[MAX_FRIENDS_NUMB];
    int friends_numb;
};

struct client clients[MAX_CLIENTS_NUMB];
mqd_t server_queueID = -1;
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

    if (mq_send(clients[clientID].queue_key, (char *) &msg, MESSAGE_SIZE, commandPiority(command_type))) {
        printf("Error while sending to client\n");
        printf("%s\n", strerror(errno));
        return 1;
    }

    kill(clients[clientID].pid, SIGRTMIN);
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

    if (mq_send(clients[clientID].queue_key, (char *) &msg, MESSAGE_SIZE, commandPiority(command_type))) {
        printf("Error while sending\n");
        return 1;
    }

    kill(clients[clientID].pid, SIGRTMIN);
    return 0;
}

void init(int clientID, char message_text[MESSAGE_SIZE]){

    int i=0;
    while(i<MAX_CLIENTS_NUMB && clients[i].pid != -1) i++;

    if(clients[i].pid != -1){
        printf("Cannot add more clients\n");
        return;
    }
    
    if ((clients[i].queue_key = mq_open(message_text, O_WRONLY)) == -1) {
        printf("Error while creating queue\n");
        return;
    }
printf("dupa\n");
    clients[i].clientID = i;
    clients[i].pid = clientID;
    clients[i].friends_numb = 0;   //in case of reuse this field in list
printf("dupa\n");
    char response[MESSAGE_SIZE];
    sprintf(response, "%i", i);
    send_from_server(i, INIT, response);


    
    
   /*
    clients[clientId].pid = clientPID;
    clients[clientId].numberOfFriends = 0;

    printf("Init of client %i with id %i...\n", clients[clientId].queueId, clientId);

    char text[MESSAGE_SIZE];
    sprintf(text, "%i", clientId);
    send(clientId, INIT, text);
     */
}

void echo(int clientID, char message_text[MESSAGE_SIZE]){
    char *date = get_date();
    char response[MESSAGE_SIZE];
    sprintf(response, "%s %s", date, message_text);
    send_from_server(clientID, ECHO, response);
}

void list(int clientID){
    char response[MESSAGE_SIZE];
    for(int i=0; i< MESSAGE_SIZE; i++){
        response[i] = '\0';
    }
    char buffer[MESSAGE_SIZE];
    for(int i=0; i<MAX_CLIENTS_NUMB; i++){
        if(clients[i].pid != -1){
            sprintf(buffer, "ID: %i PID: %ld Key: %i\n", i, (long int)clients[i].pid, clients[i].queue_key);
            strcat(response, buffer);
        } 
    }
    // printf("%s", response);
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
    int friendID;
    while(tmp != NULL){
        friendID = strtol(tmp, NULL, 10);
        if(!exist_client(friendID)){
            printf("Wrong client ID %i\n", friendID);
        }
        else{
            add_friend(clientID, &clients[friendID]);
        }
        tmp = strtok(NULL, delim);
    }
}

void delete_friends(int clientID, char message_text[MESSAGE_SIZE]){
    char delim[] = " ";
    char *tmp;
    tmp = strtok(message_text,delim);
    int friendID;
    while(tmp != NULL){
        friendID = strtol(tmp, NULL, 10);
        if(!exist_client(friendID)){
            printf("Wrong client ID\n");
        }
        else{
            delete_friend(clientID, &clients[friendID]);
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
    int friendID;
    tmp = strtok(message_text,delim);
    while(tmp != NULL){
        friendID = strtol(tmp, NULL, 10);
        if(!exist_client(friendID)){
            printf("Wrong client ID\n");
        }
        else{
            add_friend(clientID, &clients[friendID]);
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
        if(clients[clientID].friends[i] != NULL){
            if(exist_client(clients[clientID].friends[i]->clientID)){
                char *date = get_date();
                char dated_message[MESSAGE_SIZE];
                sprintf(dated_message, "%s %s", date, message_text);

                send_from_client(clients[clientID].friends[i]->clientID, clientID, SEND_2_FRIENDS, dated_message);
            }
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

void del_client(int clientID){
    clients[clientID].pid = -1;
    clients[clientID].clientID = -1;
    clients[clientID].friends_numb = 0;
    clients[clientID].queue_key = -1;
    // for(int i=0; i<MAX_CLIENTS_NUMB; i++){
    //     clients[clientID].friends[i] = NULL;
    // }
}

void handle_message(struct MESSAGE* msg){
    printf("%ld", msg->command_type);
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
        case STOP:
            del_client(msg->senderID);
            break;
        default:
            // printf("Unknown command\n");
            break;
    }
}

void stop(){
    int counter = 0;
    for(int i=0; i<MAX_CLIENTS_NUMB; i++){
        if(exist_client(i)){
            // send_from_server(i, STOP, "");
            kill(clients[i].pid, SIGINT);
            counter++;
            // printf("ZABIJAM: %i", i);
        }   
    }       
           
    struct MESSAGE buf;
    while(counter>0){
        mq_receive(server_queueID, (char *) &buf, MESSAGE_SIZE, NULL);
        counter--;
    }

    is_running = 0;
}

void sigint_handler(){
    stop();

    if (mq_close(server_queueID) == -1) {
        printf("Error while closing queue");
        exit(-1);
    }
    if (mq_unlink(SERVER_QUEUE_NAME) == -1){
        printf("Error while deleting queue");
        exit(-1);
    }

    exit(-1);
}

int main(int argc, char** argv){

    signal(SIGINT, sigint_handler);

    for(int i=0; i<MAX_CLIENTS_NUMB;i++){
        clients[i].pid = -1;
        clients[i].clientID = -1;
        clients[i].queue_key = -1;
        clients[i].friends_numb = 0;
    }

    
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = MAX_QUEUE_SIZE;
    queue_attr.mq_msgsize = MESSAGE_SIZE;

    server_queueID = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr);

    if( server_queueID == -1){
        printf("Error while creating queue: %s\n", strerror(errno));
        exit(-1);
    }

    struct MESSAGE buf;
    while(is_running){
        mq_receive(server_queueID, (char *) &buf, MESSAGE_SIZE, NULL);
        handle_message(&buf);
    }
    
    if (mq_close(server_queueID) == -1) {
        printf("Error while closing queue");
        exit(-1);
    }
    if (mq_unlink(SERVER_QUEUE_NAME) == -1){
        printf("Error while deleting queue");
        exit(-1);
    }
    return 0;
}