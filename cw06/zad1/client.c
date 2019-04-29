#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include "communication.h"


int is_running = 1;
int clientID = -1;
int client_queue_key = -1;
int server_queue_key = -1;

void to_upper(char *str){
    for(int i=0; i<strlen(str); i++) str[i] = (char)(toupper(str[i]));
}

int send_to_server(enum COMMAND command_type, char message_text[MESSAGE_SIZE]){
    struct MESSAGE msg;
    msg.command_type = command_type;
    msg.senderID = clientID;
    strcpy(msg.message, message_text);
    if (msgsnd(server_queue_key, &msg, msg_size, IPC_NOWAIT) == -1) {
        printf("Error while sending to server\n");
        return 1;
    }
    return 0;
}

int receive(struct MESSAGE *msg) {
    if (msgrcv(client_queue_key, msg, msg_size, -(MAX_COMMAND_ID + 1), 0) == -1) {
        printf("Error while receiving message\n");
        return 1;
    }
    return 0;
}

char* read_line_from_cmd(FILE* fd){
    char* line = calloc(MAX_COMMAND_SIZE,sizeof(char));
    line = fgets(line, MAX_COMMAND_SIZE * sizeof(char), fd);
    // char format[25];
    // char c = '%';
    // sprintf(format, "%c%i[^\n]", c, MAX_COMMAND_SIZE-1);
    // if(scanf(format, line) == EOF){
    //     printf("Error while reading from cmd\n");
    //     return NULL;
    // }
    return line;
}

int count_args(char args[MAX_COMMAND_SIZE]){
    int argc = 0;
    for(int i=0; i<MAX_COMMAND_SIZE && args[i] != '\0'; i++){
        if(args[i] == ' ') argc++;
    }
    return argc;
}

/////////////////////////////////////////
// COMMENDS HANDLERS
/////////////////////////////////////////

void init(){
    struct MESSAGE msg;
    char message_text[MESSAGE_SIZE];
    msg.command_type = INIT;
    sprintf(message_text, "%i", client_queue_key);
    strcpy(msg.message, message_text);
    msg.senderID = getpid();
    if (msgsnd(server_queue_key, &msg, msg_size, IPC_NOWAIT)){
        printf("Error while sending INIT to server\n");
        return;
    }
    receive(&msg);
    sscanf(msg.message, "%i", &clientID);
}

void stop(){
    is_running = 0;
    send_to_server(STOP, "");
}

void list(){
    send_to_server(LIST, "");
    
    struct MESSAGE msg;
    receive(&msg);

    printf("%s", msg.message);
}

void send_2_one(char args[MAX_COMMAND_SIZE]){

    if(count_args(args) < 2){
        printf("Too few arguments in 2ONE call\n");
        return;
    }
    send_to_server(SEND_2_ONE, args);
}

void send_2_friends(char args[MAX_COMMAND_SIZE]){
    if(count_args(args) < 1){
        printf("Too few arguments in 2FRIENDS call\n");
        return;
    }
    send_to_server(SEND_2_FRIENDS, args);
}

void send_2_all(char args[MAX_COMMAND_SIZE]){
    send_2_all(args);
}

void friends(char args[MAX_COMMAND_SIZE]){
    if(count_args(args) < 1){
        printf("Too few arguments in FRIENDS call\n");
        return;
    }
    send_to_server(FRIENDS, args);
}

void add(char args[MAX_COMMAND_SIZE]){
    if(count_args(args) < 1){
        printf("Too few arguments in ADD call\n");
        return;
    }
    send_to_server(ADD_FRIENDS, args);
}

void del(char args[MAX_COMMAND_SIZE]){
    if(count_args(args) < 1){
        printf("Too few arguments in DEL call\n");
        return;
    }
    send_to_server(DEL_FRIENDS, args);
}

void echo(char args[MAX_COMMAND_SIZE]){
    send_to_server(ECHO, args);
    
    struct MESSAGE msg;
    receive(&msg);
    printf("%s", msg.message);
}

/////////////////////////////////////////

void execute(char line[MAX_COMMAND_SIZE]){
    char command[8];
    /*
    commands are like to the following:
        -init 
        -stop 
        -list
        -echo string
        -friends lista
        -2friends message
        -2one id message
        -2all message
        -add lista
        -del lista
    */
    char args[MAX_COMMAND_SIZE];
    sscanf(line, "%s %s", command, args);
    to_upper(command);

    if(strcmp(command, "2ONE") == 0){
        send_2_one(args);
    }else if(strcmp(command, "2ALL") == 0){
        send_2_all(args);
    }else if(strcmp(command, "2FRIENDS") == 0){
        send_2_friends(args);
    }else if(strcmp(command, "ADD") == 0){
        add(args);
    }else if(strcmp(command, "DEL") == 0){
        del(args);
    }else if(strcmp(command, "FRIENDS") == 0){
        friends(args);
    }else if(strcmp(command, "ECHO") == 0){
        echo(args);
    }else if(strcmp(command, "LIST") == 0){
        list();
    }else if(strcmp(command, "STOP") == 0){
        stop();
    }else if(strcmp(command, "INIT") == 0){
        init();
    }
}

void sigint_handler(){
    stop();
}

int main(int argc, char** argv){
    signal(SIGINT, sigint_handler);

    if ((server_queue_key = msgget(getServerQueueKey(), 0)) == -1){
        printf("Error while opening server queue\n");
    }
    if ((client_queue_key = msgget(getClientQueueKey(), IPC_CREAT | IPC_EXCL | 0666)) == -1){
        printf("Error while creating client queue\n");
    }
    init();
    
    while (is_running) {
        char* line = read_line_from_cmd(fdopen(STDIN_FILENO, "r"));
        execute(line);
    }

    //some cleanings 
    return 0;
}