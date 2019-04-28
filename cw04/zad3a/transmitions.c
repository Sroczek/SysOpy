#include "transmitions.h"

int get_mode(char *str, enum MODE *mode){
    //to_uper
    for(int i=0; i<strlen(str); i++)  str[i] = (char)(toupper(str[i]));

    if(strcmp("KILL", str) == 0){
        *mode = KILL;
        return 0;
    } 
    if(strcmp("SIGQUEUE", str) == 0){
        *mode = SIGQUEUE;
        return 0;
    }
    if(strcmp("SIGRT", str) == 0){
        *mode = SIGRT;
        return 0;
    }
    return -1;
}


void transmit_kill(long int PID, long int signals_numb){
    for(int i=0; i<signals_numb; i++){
        kill(PID, SIGUSR1);
    }
    kill(PID,SIGUSR2);
}


void transmit_sigqueue(long int PID, long int signals_numb){
    union sigval val;

    for(int i=0; i<signals_numb; i++){
        val.sival_int = i;
        sigqueue(PID, SIGUSR1, val);
    }
    val.sival_int = signals_numb;
    sigqueue(PID, SIGUSR2, val);
}


void transmit_sigrt(long int PID, long int signals_numb){
    for(int i=0; i<signals_numb; i++){
        kill(PID, SIGRTMIN);
    }
    kill(PID,SIGRTMIN+1);
}


void transmit(long int PID, long int signals_numb, enum MODE mode){
    switch (mode)
    {
        case KILL:
            transmit_kill(PID, signals_numb);
            break;
    
        case SIGQUEUE:
            transmit_sigqueue(PID, signals_numb);
            break;

        case SIGRT:
            transmit_sigrt(PID, signals_numb);
            break;
    }
}