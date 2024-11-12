#ifndef LISTA_H
#define LISTA_H
#include <sys/types.h>
#define MAX 50


/*
Definición de cabecera de la estructura de datos Lista
Se usará una lista de tamaño MAX para esto.
En este archivo esta la definición de las funciones

*/

struct Process{
	pid_t id;
	int cpuBurst;
	int tCompletition;
	int tWaiting;
	int priority;
	char name[5];
};

typedef struct{
	int prev;
	int actual;
	int next;
	int size;
	struct Process procesos[MAX];
}lista_t;

int addProcess(struct Process process);
int addProcesses(struct Process *process);
struct Process deleteProcess();
struct Process getProcess(int n);
int aumentarEspera(int s);
int restarEjecucion(int s);
void ordenarPorPrioridad();
int size();
int isEmpty();
const char *toString();

#endif
