#include "lista.h"
#include <stdio.h>
#include <sys/types.h>
#define MAX 50

/*
Implementación de las funciones del archivo cabecera
Parte del código. Recordemos que usaremos un MAX
*/

static lista_t lista = {.prev = -1, .actual = -1, .next = -1, .size = 0};



int addProcess(struct Process process){
	return 1;
}

/*
Añade procesos de un arreglo de procesos.
Devuelve -1 si ocurrió un error
Devuelve la cantidad de elementos copiados
Deja apuntando en la posición incial del primer elemento agregado
*/
int addProcesses(struct Process *process){
	int sizeOfArray = sizeof(process)/sizeof(process[0]);
	if(lista.size >= MAX || lista.size + sizeOfArray >= MAX){
		return -1;
	}

	lista.actual = lista.size;

	if(lista.actual==0){
		lista.prev=-1;
	}

	lista.next = lista.actual+1;


	for(int i=0;i<sizeOfArray;i++){
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
		lista.size--;
	}
	//Devolvemos la estructura original
	return toReturn;
}

process_t getprocess(int n){

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
}

int restarEjecucion(int s){

}

void ordenarPorPrioridad(){

}

int size(){
	return lista.size;
}

int isEmpty(){

}

const char *toString(){

}
