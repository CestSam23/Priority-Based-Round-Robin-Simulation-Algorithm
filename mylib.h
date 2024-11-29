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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1000

typedef struct Process{
    pid_t id; // identifidor del proceso 
    int cpuBurst; // tiempo de ejecucion
    int tCompletition; // tiempo de terminacion 
    int tWaiting; //tiempo espera
    int priority; 
    char name[50];
} process_t;

typedef struct{
	int actual;
	int size;
	struct Process procesos[MAX];
}lista_t;

void addProcess(lista_t *lista, struct Process process);
int addProcesses(lista_t *lista, struct Process process[], int n);
process_t deleteProcess(lista_t *lista);
process_t getProcess(lista_t *lista, int n);
int aumentarEspera(lista_t *lista, int s);
int restarEjecucion(lista_t *lista, int s);
int aumentarTerminacion(lista_t *lista, int s);
void ordenarPorPrioridad(lista_t *lista);
int size(lista_t *lista);
int isEmpty(lista_t *lista);
int next(lista_t *lista);
int prev(lista_t *lista);
process_t actual(lista_t *lista);
int actualN(lista_t *lista);
int isLast(lista_t *lista);
void rewindList(lista_t *lista);

void toString(lista_t *lista);

#endif 