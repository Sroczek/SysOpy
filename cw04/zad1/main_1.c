#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h> 

int run = 1;

void SIGTSTP_handler(){
    if(run) printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    run = !run;
}

void SIGINT_handler(){
    printf("Odebrano sygnał SIGINT\n");
    exit(1);
}

int main(void){
    time_t tmp;
    struct tm current_time;

    //sygnał SIGTSTP
    struct sigaction act;
    act.sa_handler = SIGTSTP_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTSTP,&act,NULL);

    //sygnał SIGNINT
    signal(SIGINT, SIGINT_handler);

    while(1){
        if(run){
            time(&tmp);
            current_time = *localtime(&tmp);
            printf("%i %i %i\n", current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
        }
    }
   
}