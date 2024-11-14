#ifndef LISTA_H
#define LISTA_H
#include <sys/types.h>
#define MAX 50


/*
Definición de cabecera de la estructura de datos Lista
Se usará una lista de tamaño MAX para esto.
En este archivo esta la definición de las funciones

*/

typedef struct Process{
	pid_t id;
	int cpuBurst;
	int tCompletition;
	int tWaiting;
	int priority;
	char name[5];
}process_t;

typedef struct{
	int prev;
	int actual;
	int next;
	int size;
	struct Process procesos[MAX];
}lista_t;

int addProcess(struct Process process);
int addProcesses(struct Process *process);
process_t deleteProcess();
process_t getProcess(int n);
int aumentarEspera(int s);
int restarEjecucion(int s);
void ordenarPorPrioridad();
int size();
int isEmpty();
int next();
int prev();
process_t actual();
int actualN();
int isLast();

void toString();

#endif
