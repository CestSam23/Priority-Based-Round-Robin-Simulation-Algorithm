#include <stdio.h>
#include "lista.h"
#include "mylib.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define QUANTUM 5

//Para memoria compartida
void freeResources(int semid, int shmid, char *shm_ptr);
void AtSharedMemory(key_t key, lista_t *shm_ptr, int shmid);
void writeProcess(lista_t *SharedMemory, int index, process_t process);
process_t readProcess(lista_t *SharedMemory, int index);
void writeList(lista_t *SharedMemory, int prev, int actual, int next, int size);
void readList(lista_t *SharedMemory);

void roundRobin();
void priority();
void sendToAnalytics(process_t process);

lista_t listaDeProcesos;

int main(){

	//Los procesos guardados por la memoria compartida son guardados en listaDeProcesos

    key_t key_shm_2 = ftok("/bin/ls", 2); // llave memoria compartida
    int shmid = shmget(key_shm_2, sizeof(struct Process), IPC_CREAT | 0666);  // Crear memoria compartida
    if (shmid == -1) {
        perror("Error al crear memoria compartida");
        //exit(1);
    }


	/*
	Entraremos en un estado constante de espera de lotes de procesos por el despachador de largo plazo
	Con ayuda de semaforos sabremos cuando hay listo. Y con memoria compartida, los leemos en lista
	*/
	
	/*Por mientras, creamos una estructura de ejemplo*/
	printf("Despachador\n");
	process_t p1 = {1,43,0,0,0,"pro1\n"};
	process_t p2 = {2,12,0,0,0,"pro1\n"};
	process_t p3 = {3,6,0,0,0,"pro1\n"};
	process_t p4 = {4,4,0,0,0,"pro1\n"};
	process_t p5 = {5,7,0,0,0,"pro1\n"};

	process_t prs[5] = {p1,p2,p3,p4,p5};
	addProcesses(prs,5);
	printf("Procesos añadidos\nDespachadno\n");
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
	for(int i=0; i<size(); i++){
		printf("Despachando proceso %d con rr\n",i);
		aumentarEspera(QUANTUM);
		aumentarTerminacion(QUANTUM);
		//Se terminó Proceso, enviar a modulo de estadistica
		if(restarEjecucion(QUANTUM)==-1){
			sendToAnalytics(deleteProcess());
			printf("Proceso %d eliminado con rr\n",i);
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

	for(int i=0;i<size();i++){
		printf("Despachando Proceso %d con priority\n",i);
		int remain = listaDeProcesos.procesos[listaDeProcesos.actual].cpuBurst;
		aumentarEspera(remain);
		aumentarTerminacion(remain);

		restarEjecucion(remain);

		
		printf("Proceso %d eliminado \n",i);
		sendToAnalytics(deleteProcess());
	}
}

void sendToAnalytics(process_t process){
	//MEMORIA COMPARTIDA. ENVIADA A ANALISIS
}

void freeResources(int semid, int shmid, char *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
}

void AtSharedMemory(key_t key, lista_t *shm_ptr, int shmid){
    shm_ptr = shmat(shmid, 0, 0);  // Asociar memoria compartida
    /*if (shm_ptr == (char *)-1) {
        perror("Error al asociar memoria compartida");
        exit(1);
    }*/
}

// Funcion para esccribir los procesos en la memoria compartida mediante su indice
void writeProcess(lista_t *SharedMemory, int index, process_t process) {
    // Verificar que el índice esté dentro de los límites
    if (index < 0 || index >= MAX) {
        printf("Índice fuera de rango.\n");
        return;
    }

    // Escribir los datos del proceso en la memoria compartida
    SharedMemory->procesos[index] = process;
}

// Funcion para leer los procesos en la memoria compartida mediante su indice
process_t readProcess(lista_t *SharedMemory, int index) {
    // Verificar que el índice esté dentro de los límites
    if (index < 0 || index >= MAX) {
        printf("Índice fuera de rango.\n");
        // Devolver un valor por defecto o nulo
        process_t procesoNulo = {0};
        return procesoNulo;
    }

    // Leer el proceso de la memoria compartida
    return SharedMemory->procesos[index];
}

// Función para escribir una lista (lote) e
 void writeList(lista_t *SharedMemory, int prev, int actual, int next, int size) {
    SharedMemory->prev = prev;
    SharedMemory->actual = actual;
    SharedMemory->next = next;
    SharedMemory->size = size;
}

// Función para leer una lista 
void readList(lista_t *SharedMemory) {
    printf("prev: %d, actual: %d, next: %d, size: %d\n", 
    SharedMemory->prev, 
    SharedMemory->actual, 
    SharedMemory->next, 
    SharedMemory->size);
}
