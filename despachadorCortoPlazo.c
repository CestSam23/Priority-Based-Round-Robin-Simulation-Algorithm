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
	process_t p1 = {1,23,0,0,0,"pro1\0"};
	process_t p2 = {2,5,0,0,0,"pro2\0"};
	process_t p3 = {3,6,0,0,0,"pro3\0"};
	process_t p4 = {4,9,0,0,0,"pro4\0"};
	process_t p5 = {5,2,0,0,0,"pro5\0"};
	process_t p6 = {6,10,0,0,0,"pro6\0"};
	process_t p7 = {7,35,0,0,0,"pro7\0"};
	

	process_t prs[7] = {p1,p2,p3,p4,p5,p6,p7};
	addProcesses(prs,7);
	//inicio de despachador
	roundRobin();

	printf("Round Robin Terminado. LISTA ACTUAL\n");
	toString();

	printf("INICIANDO PRIORIDADES\n\n");
	ordenarPorPrioridad();
	toString();
	printf("INICIO\n\n");
	priority();
	printf("Finalizacion");

	//Espera de otro lote de memoria
}

void roundRobin(){
	/*
	Primera y única ronda de Round Robin
	Recordemos que por documentacion addProcesses deja
	apuntando actual al primer elemento agregado
	*/
	rewindList();
	for(int i=0; i<size(); i++){
		printf("****Despachando Proceso %d con Round Robin****\n",i);
		aumentarEspera(QUANTUM);
		aumentarTerminacion(QUANTUM);
		//Se terminó Proceso, enviar a modulo de estadistica
		if(restarEjecucion(QUANTUM)==-1){
			printf("El proceso %d ha sido eliminado\n",i);
			sendToAnalytics(deleteProcess());
			toString();
			//No es necesario avanzar. La función delete process
			//Recorre en automático
		} else {
			//Avanzamos a la siguiente estructura
			//En ultimo elemento, regresamos al inicio
			printf("ListaFinal\n");
			next();
			toString();
		}
		printf("***********************************\n");
	}
}

void priority(){
	/*
	Para este punto, los procesos ya están en orden
	Solamente es despacharlos
	
	 */
	rewindList();
	int iterations = size();
	for(int i=0;i<iterations;i++){
		printf("**********Despachando Proceso %d con priority\n*************",i);
		int remain = actual().cpuBurst;
		printf("Despachando %d con %d\n\n",i,remain);
		aumentarEspera(remain);
		aumentarTerminacion(remain);
		restarEjecucion(remain);

		
		printf("Proceso %d eliminado \n",i);
		sendToAnalytics(deleteProcess());
		toString();
		
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
    SharedMemory->actual = actual;
    SharedMemory->size = size;
}

// Función para leer una lista 
void readList(lista_t *SharedMemory) {

   
}
