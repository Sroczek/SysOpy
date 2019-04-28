#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include "transmitions.h"

int DEBUG = 0;

long int receive_signals_numb = 0;
int is_receiving;
long int sender_PID;

void SIG1_handler(int signal, siginfo_t *info, void* ucontext){
    if(DEBUG) printf("Odebrałem sygnał 1\n");
    receive_signals_numb ++;
}

void SIG2_handler(int signal, siginfo_t *info, void* ucontext){
    if(DEBUG) printf("Odebrałem sygnał 2\n");
    is_receiving = 0;
    sender_PID = (long int)(info->si_pid);
}

int main(int argc, char **argv){

    if(argc < 2){
        printf("To few argument in function call\n");
        exit(-1);
    }

    enum MODE mode;
    if(get_mode(argv[1], &mode) == -1){
        printf("Invalid value of mode\n");
        exit(-1);
    }; 

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

    printf("Catcher:  PID: %ld\n",(long int)(getpid()));

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
    
    is_receiving = 1;
    while(is_receiving){}
    
    if(DEBUG) printf("Catcher: starting sending\n");

    transmit(sender_PID, receive_signals_numb, mode);

    printf("Catcher: receive: %ld\n", receive_signals_numb);
}