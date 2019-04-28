#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>


int run = 0;
int child_pid;

void run_loop(){
    run = 1;
    child_pid = (int)fork();
    if(child_pid == 0) execl("./loop_date", "loop_date", NULL);
}

void SIGTSTP_handler(){
    if(run){
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        kill(child_pid, SIGTERM);
        //if is still running:
        if(waitpid(child_pid,NULL,WNOHANG) == 0) kill(child_pid, SIGKILL);
        run = 0;
    }
    else{
        run_loop();
    }
}

void SIGINT_handler(){
    printf("Odebrano sygnał SIGINT\n");
    kill(child_pid, SIGTERM);
    exit(-1);
}

int main(void){

    //signal SIGTSTP
    struct sigaction act;
    act.sa_handler = SIGTSTP_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTSTP,&act,NULL);

    //signal SIGNINT
    signal(SIGINT, SIGINT_handler);

    run_loop();

    while(1){};
}