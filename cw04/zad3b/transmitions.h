#pragma once
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

int counter;

enum MODE {KILL, SIGQUEUE, SIGRT};

int get_mode(char *str, enum MODE *mode);

void transmit_kill(long int PID, long int signals_numb);

void transmit_sigqueue(long int PID, long int signals_numb);

void transmit_sigrt(long int PID, long int signals_numb);

void transmit(long int PID, long int signals_numb, enum MODE mode);

void confirm_receiving(long int PID, enum MODE mode);