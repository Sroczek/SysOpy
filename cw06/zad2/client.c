#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <mqueue.h>
#include "communication.h"

#define DEBUG 0

int is_running = 1;
int clientID = -1;
int client_queue_key = -1;
mqd_t server_queue_key = -1;
char *queueName;

void to_upper(char *str){
    for(int i=0; i<strlen(str); i++) str[i] = (char)(toupper(str[i]));
}

int send_to_server(enum COMMAND command_type, char message_text[MESSAGE_SIZE]){
    struct MESSAGE msg;
    msg.command_type = command_type;
    msg.senderID = clientID;
    strcpy(msg.message, message_text);
    if (mq_send(server_queue_key, (char *) &msg, MESSAGE_SIZE, commandPiority(command_type)) == -1) {
        printf("Error while sending to server\n");
        return 1;
    }
    return 0;
}

int receive(struct MESSAGE *msg) {
    struct MESSAGE msg_1;
    if (mq_receive(client_queue_key, (char *) &msg_1, MESSAGE_SIZE, NULL) == -1) {
        printf("Error while receiving message\n");
        return 1;
    }
    
    printf("Receive\n");
    *msg = msg_1;
    return 0;
}


int count_args(char args[MAX_COMMAND_SIZE]){
    int argc = 0;
    int i=0;
    int before = 0; //0 stands for white character, 1 stands for argument
    for(; i<MAX_COMMAND_SIZE && args[i] != '\0'; i++){
        if(before == 0){
            if(args[i] != ' '){
                before = 1;
                argc++;
            }
        }
        else{
            if(args[i] == ' '){
                before = 0;
            }
        }
    }
    if(args[i-2] == ' ') argc--;
    return argc;
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
    if(DEBUG) printf("Line readed.\n\n");
    return line;
}

void execute(char line[MAX_COMMAND_SIZE]);

void read_from_file(char args[MAX_COMMAND_SIZE]){
    
    if(count_args(args) < 1){
        printf("Too few arguments in READ call\n");
        return;
    }

    char* file_name;
    char delim[] = " \n";
    file_name = strtok(args,delim);

    FILE* file = fopen(file_name, "rw");

    if(file == NULL){
        printf("%s\n", strerror(errno));
        return;
    }
    
    char *line = NULL;
    size_t size;
    while(getline(&line,&size,file) != -1){
        execute(line);
    }
}

/////////////////////////////////////////
// COMMENDS HANDLERS
/////////////////////////////////////////

void init(){
    struct MESSAGE msg;
    msg.command_type = INIT;
    printf("%s\n",queueName);
    strcpy(msg.message, queueName);
    printf("%s\n",msg.message);

    msg.senderID = getpid();
    if (mq_send(server_queue_key, (char *) &msg, MESSAGE_SIZE, commandPiority(INIT))){
        printf("Error while sending INIT to server\n");
        printf("%s/n", strerror(errno));
        return;
    }
    // receive(&msg);
    // sscanf(msg.message, "%i", &clientID);
}

void stop(){
    is_running = 0;
    send_to_server(STOP, "");
}

void list(){
    send_to_server(LIST, "");
    // struct MESSAGE msg;
    // receive(&msg);

    // printf("%s", msg.message);
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
    send_to_server(SEND_2_ALL, args);
}

void add(char args[MAX_COMMAND_SIZE]){
    if(DEBUG) printf("NUMBER OF ARGUMNETS:  %i\n\n", count_args(args));
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

void friends(char args[MAX_COMMAND_SIZE]){
    if(count_args(args) < 0){
        printf("Too few arguments in DEL call\n");
        return;
    }
    send_to_server(FRIENDS, args);    
}

void echo(char args[MAX_COMMAND_SIZE]){
    send_to_server(ECHO, args);
    
    // struct MESSAGE msg;
    // receive(&msg);
    // printf("%s", msg.message);
}

/////////////////////////////////////////

void execute(char line[MAX_COMMAND_SIZE]){
    char command[16];
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
    if(DEBUG) printf("LINE:   %s", line);
/////////////////////////////
    int i=0;
    while(i<16 && line[i] != ' ' && line[i] != '\n'){
        command[i] = line[i];
        i++;
    }

    if(i<16) command[i] = '\0';
    i++;
    int shift = i;
    while(i<MAX_COMMAND_SIZE){
        args[i-shift] = line[i];
        i++;
    }

    if(DEBUG) printf("COMMAND:   %s  ARGS:%s", command, args);
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
    }else if(strcmp(command, "READ") == 0){
        read_from_file(args);
    }else{
        printf("Unknown command\n");
    }
}

void handle_message(struct MESSAGE msg){
    switch(msg.command_type){
        case INIT:
            sscanf(msg.message, "%i", &clientID);
            break;
        case ECHO:
            printf("%s", msg.message);
            break;
        case LIST:
            printf("%s", msg.message);
            break;
        case SEND_2_ALL:
            printf("%s", msg.message);
            break;
        case SEND_2_FRIENDS:
            printf("%s", msg.message);
            break;
        case SEND_2_ONE:
            printf("%s", msg.message);
            break;
        default:
            printf("Received unknown type of message");
            break;
    }
}

void sigrtmin_handler(){
    struct MESSAGE msg;
    if(receive(&msg) == 0){
        handle_message(msg);
    }
}

void sigint_handler(){
    stop();
    exit(-1);
}

int main(int argc, char** argv){

    signal(SIGRTMIN, sigrtmin_handler);
    signal(SIGINT, sigint_handler);

    queueName=getClientQueueName();


    if ((server_queue_key = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1){
        printf("Error while opening server queue\n");
        exit(-1);
    }

    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = MAX_QUEUE_SIZE;
    queue_attr.mq_msgsize = MESSAGE_SIZE;
    if ((client_queue_key = mq_open(queueName, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr)) == -1){
        printf("Error while creating client queue\n");
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    init();

    while (is_running) {
        char* line = read_line_from_cmd(fdopen(STDIN_FILENO, "r"));
        execute(line);
    }

    if (mq_close(server_queue_key) == -1){
        printf("Error while closing server queue\n");
        exit(-1);
    }
    if (mq_close(client_queue_key) == -1){
        printf("Error while closing client queue\n");
        exit(-1);
    }
    if (mq_unlink(queueName) == -1){
        printf("Error while deleting client queue\n");
        exit(-1);
    }
    free(queueName);
    return 0;
}