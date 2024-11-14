#include "lista.h"
#include <stdio.h>
#include <sys/types.h>
#define MAX 50

/*
Implementación de las funciones del archivo cabecera
Parte del código. Recordemos que usaremos un MAX
*/

static lista_t lista = {.prev = -1, .actual = -1, .next = -1, .size = 0};


/*
Añade la estructura de un proceso individualmente, 
actualiza los punteros, retorna 1 si se agregó correctamente, 
0 en caso contrario
*/
int addProcess(struct Process process){
	if(lista.size>=MAX){
		return 0; 
	}
	lista.procesos[lista.size]=process;
	lista.size++;
	next();
}

/*
Añade procesos de un arreglo de procesos.
Devuelve -1 si ocurrió un error
Devuelve la cantidad de elementos copiados
Deja apuntando en la posición incial del primer elemento agregado
*/
int addProcesses(struct Process process[], int n){
	int sizeOfArray = n;
	if(lista.size >= MAX || lista.size + sizeOfArray >= MAX){
		return -1;
	}
	lista.procesos[lista.size] = process[0];
	lista.size++;
	next();
	for(int i=1;i<sizeOfArray;i++){
		lista.procesos[lista.size] = process[i];
		lista.size++;
	}
	
	return sizeOfArray;
}
/*
Elimina el proceso apuntado por actual
Recorre los demás elementos
Deja actual como el elemento siguiente inmediato al eliminado
*/
process_t deleteProcess(){
	//Verifica si la lista no esta vacia
	if(isEmpty()){
		process_t empty = {0};
		return empty;
	}
	//Guardamos el tamaño original y la estructura eliminada
	int originalSize = lista.size;
	process_t toReturn = lista.procesos[lista.actual];

	//Hacemos corrimiento de lugares
	for(int i=lista.actual;i<originalSize-1;i++){
		lista.procesos[i] = lista.procesos[i+1];
	}
	lista.size--;
	prev();
	next();
	//Devolvemos la estructura original
	return toReturn;
}
/*
Función que obtiene la estructura del proceso dado su indice
*/
process_t getprocess(int n){
	if(n>=0 && n<=lista.size){
		return lista.procesos[n];
	}
	printf("No existe el proceso\n");
	return (process_t){0}; 
}


/*
Esta función aumenta el tiempo de espera de todos los procesos
excepto del actual despachado.
*/
int aumentarEspera(int s){
	for(int i=0; i<lista.size;i++){
		if(i!=lista.actual)
			lista.procesos[i].tWaiting += s;
	}

	return 1;
}

int aumentarTerminacion(int s){
	lista.procesos[lista.actual].tCompletition += s;
	return 1;
}

/*
Esta función resta el tiempo de ejecución al proceso actual,
es decir al que está siendo despachado, si el cpuBurst restante es 
menor o igual que cero no es necesario seguirle restando, por lo que
lo eliminamos
*/
int restarEjecucion(int s){
	if(isEmpty()) return 0;


	if(lista.procesos[lista.actual].cpuBurst-s <= 0){
		int time = lista.procesos[lista.actual].cpuBurst % s;
		lista.procesos[lista.actual].cpuBurst = 0;
		aumentarTerminacion(time);
		deleteProcess();
	} else {
		lista.procesos[lista.actual].cpuBurst-=s;

	}

	return 1;
}
/*Función que ordena los procesos mediante su cpu burstime restante, lo hace por medio del algoritmo de ordenación burbuja por su eficiencia en vectores*/
void ordenarPorPrioridad(){
	/*Verifica y elimina los procesos que ya no necesitan ser ordenados*/
	for(int k=0; k<lista.size; k++){
		if(lista.procesos[k].cpuBurst<=0){
			deleteProcess(lista.procesos[k]);
			k--;
		}
	}

	for(int i=0; i<lista.size-1;i++){
		for(int j=0; j<lista.size-i-1;j++){
			if(lista.procesos[j].cpuBurst> lista.procesos[i+1].cpuBurst){
				process_t aux=lista.procesos[j];
				lista.procesos[j]=lista.procesos[j+1];
				lista.procesos[j+1]=aux;
			}
		}
	}
}

/*
Esta función devuelve el tamaño de la lista, cada que se agrega un proceso
size aumenta, por lo que nos dice el tamaño de lista
*/
int size(){
	return lista.size;
}

/*Esta función devuelve si la lista esta vacia.
En cuyo caso devuelve 1, contrario 0.*/
int isEmpty(){
	return lista.size == 0;
}
/*IMPRIME TODA LA LISTA*/
void toString(){
	if(isEmpty()){
		printf("List Empty");
		return;
	}else{
		printf("\tLIST: \n");
		for(int i=0; i<lista.size;i++){
			process_t procesoAux=lista.procesos[i];
			printf("Id: %d, Nombre: %s, CPU Burst: %d, TimeCompletition: %d, TimeWaiting: %d, Priority: %d\n", procesoAux.id, procesoAux.name, procesoAux.cpuBurst, procesoAux.tCompletition, procesoAux.tWaiting, procesoAux.priority);
		}
	}
}


/*
Funciones para avanzar en la lista enlazada
Funciona para casos esquina, supuestamente jaja
Agregamos la funcion actual y actualN, que devuelve
la estructura actual, y el entero actual
*/
int next(){
	if(lista.size>0){
		lista.prev = lista.actual;
		lista.actual = lista.next;
		lista.next = (lista.next+1)% lista.size;
	} else{
		lista.prev = -1;
		lista.actual = -1;
		lista.next = -1;
	}

	return lista.actual;
}

int prev(){
	if(lista.size > 0){
		lista.next = lista.actual;
		lista.actual = lista.prev;
		lista.prev = (lista.prev-1+lista.size)%lista.size;
	} else {
		lista.prev = -1;
		lista.actual = -1;
		lista.next = -1;
	}

	return lista.actual;
}

process_t actual(){
	return lista.procesos[lista.actual];
}

int actualN(){
	return lista.actual;
}

/*
Funcion que devuelve si el elemento actual es el elemento final
*/
int isLast(){
	return lista.actual == lista.size;
}
