#ifndef MYLIB_H
#define MYLIB_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>

//----

typedef struct Process{
    pid_t id; // identifidor del proceso 
    int cpuBurst; // tiempo de ejecucion
    int tCompletition; // tiempo de terminacion 
    int tWaiting; //tiempo espera
    int priority; 
    char name[5];
} process_t;

#include "lista.h"

#endif 

