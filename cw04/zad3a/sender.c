#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "transmitions.h"

int DEBUG = 0;

long int receive_signals_numb = 0;
int is_receiving;

long int retransmited_signals_numb = 0;


void SIG1_handler(int signal, siginfo_t *info, void* ucontext){
    if(DEBUG) printf("Odebrałem sygnał 1\n");
    receive_signals_numb ++;
}

void SIG2_handler(int signal, siginfo_t *info, void* ucontext){
    if(DEBUG) printf("Odebrałem sygnał 2\n");
    retransmited_signals_numb = info->si_value.sival_int;
    is_receiving = 0;
}


int main(int argc, char **argv){

    if(argc < 4){
        printf("To few argument in function call\n");
        exit(-1);
    }

    long int PID = strtol(argv[1],NULL, 10);
    if(PID < 2){
        printf("Invalid value of PID\n");
        exit(-1);
    }

    long int send_signals_numb = strtol(argv[2], NULL, 10);
    if(send_signals_numb < 0){
        printf("Invalid value of number of signals to send\n");
        exit(-1);
    }

    enum MODE mode;
    if(get_mode(argv[3], &mode) == -1){
        printf("Invalid value of mode\n");
        exit(-1);
    };

    //blocking other signals
    sigset_t mask;
    sigfillset(&mask);
    if(mode == KILL || mode == SIGQUEUE){
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
    }
    if(mode == SIGRT){
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMIN+1);
    }
    sigprocmask(SIG_BLOCK, &mask, NULL);

    //setting handlers
    struct sigaction act1;
    act1.sa_sigaction = (void *) SIG1_handler;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = SA_SIGINFO;
    
    struct sigaction act2;
    act2.sa_sigaction = (void *) SIG2_handler;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = SA_SIGINFO;

    if(mode == KILL || mode == SIGQUEUE){
        sigaction(SIGUSR1, &act1, NULL);
        sigaction(SIGUSR2, &act2, NULL);
    } else
    if(mode == SIGRT){
        sigaction(SIGRTMIN, &act1, NULL); 
        sigaction(SIGRTMIN+1, &act2, NULL);
    }

    transmit(PID, send_signals_numb, mode);

    is_receiving = 1;
    while(is_receiving){}
    
    if(mode == SIGQUEUE)
    printf("Sender: send: %ld retransmited: %ld receive: %ld \n", send_signals_numb, retransmited_signals_numb,  receive_signals_numb);
    else
    printf("Sender: send: %ld receive: %ld\n", send_signals_numb, receive_signals_numb);

}