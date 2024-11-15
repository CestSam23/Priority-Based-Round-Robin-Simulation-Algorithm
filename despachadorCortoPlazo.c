#include <stdio.h>
#include "lista.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define QUANTUM 5


void roundRobin();
void priority();
void sendToAnalytics(process_t process);

lista_t listaDeProcesos;

int main(){

	//Los procesos guardados por la memoria compartida son guardados en listaDeProcesos


	/*
	Entraremos en un estado constante de espera de lotes de procesos por el despachador de largo plazo
	Con ayuda de semaforos sabremos cuando hay listo. Y con memoria compartida, los leemos en lista
	*/
	
	/*Por mientras, creamos una estructura de ejemplo*/
	process_t p1 = {1,43,0,0,0,"pro1\n"};
	process_t p2 = {2,12,0,0,0,"pro1\n"};
	process_t p3 = {3,6,0,0,0,"pro1\n"};
	process_t p4 = {4,4,0,0,0,"pro1\n"};
	process_t p5 = {5,7,0,0,0,"pro1\n"};

	process_t prs[5] = {p1,p2,p3,p4,p5};
	addProcesses(prs,5);

	//inicio de despachador
	roundRobin();
	ordenarPorPrioridad();
	priority();

	//Espera de otro lote de memoria
}

void roundRobin(){
	/*
	Primera y única ronda de Round Robin
	Recordemos que por documentacion addProcesses deja
	apuntando actual al primer elemento agregado
	*/
	for(int i=0; i<listaDeProcesos.size; i++){
		aumentarEspera(QUANTUM);
		aumentarTerminacion(QUANTUM);
		//Se terminó Proceso, enviar a modulo de estadistica
		if(restarEjecucion(QUANTUM)==-1){
			sendToAnalytics(deleteProcess());
			//No es necesario avanzar. La función delete process
			//Recorre en automático
		} else {
			//Avanzamos a la siguiente estructura
			//En ultimo elemento, regresamos al inicio
			next();
		}
	}
}

void priority(){
	/*
	Para este punto, los procesos ya están en orden
	Solamente es despacharlos
	
	 */

	for(int i=0;i<listaDeProcesos.size;i++){
		int remain = listaDeProcesos.procesos[listaDeProcesos.actual].cpuBurst;
		aumentarEspera(remain);
		aumentarTerminacion(remain);

		restarEjecucion(remain);

		sendToAnalytics(deleteProcess());
	}
}

void sendToAnalytics(process_t process){
	//MEMORIA COMPARTIDA. ENVIADA A ANALISIS
}
