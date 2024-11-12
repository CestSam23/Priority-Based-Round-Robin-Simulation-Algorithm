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
	lista.prev=lista.actual;
	lista.actual=lista.size - 1;
	if(lista.size<MAX){
		lista.next=lista.size;
	}else{
		lista.next=-1;
	}
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
}

/*
Esta función resta el tiempo de ejecución al proceso actual,
es decir al que está siendo despachado, si el cpuBurst restante es 
menor o igual que cero no es necesario seguirle restando, por lo que
lo eliminamos
*/
int restarEjecucion(int s){
	if(isEmpty) return 0;

	for(int i=0; i<lista.size; i++){
		if(i==lista.actual){
			if(lista.procesos[i].cpuBurst<=0) deleteProcess(lista.procesos[i]);
			lista.procesos[i].cpuBurst-=s;
			return 1;
		} 
	}
	return 0;
}

void ordenarPorPrioridad(){

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

const char *toString(){

}
