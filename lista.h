#ifndef LISTA_H
#define LISTA_H
#include "mylib.h"
#define MAX 50


/*
Definición de cabecera de la estructura de datos Lista
Se usará una lista de tamaño MAX para esto.
En este archivo esta la definición de las funciones

*/

typedef struct{
	int prev;
	int actual;
	int next;
	int size;
	struct Process procesos[MAX];
}lista_t;

int addProcess(struct Process process);
int addProcesses(struct Process process[], int n);
process_t deleteProcess();
process_t getProcess(int n);
int aumentarEspera(int s);
int restarEjecucion(int s);
int aumentarTerminacion(int s);
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
