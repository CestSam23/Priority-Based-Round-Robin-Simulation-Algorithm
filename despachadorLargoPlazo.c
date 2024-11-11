#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <perror.h>
#include <sys/types.h>

struct Process{
	pid_t id;
	int cpuBurst;
	int tCompletition;
	int tWaiting;
	int priority;
	char name[5];
}

/*
				Despachador de Largo Plazo
	Este despachador se apoya del archivo processRequest.dat para leer procesos
	Lee n cantidad de datos (De 1 a 20) cada k segundos (1-10) m veces
	Estos datos leidos son enviados al despachador de Corto Plazo
	Siempre esta a la espera de leer nuevos datos

*/

int main(){
	FILE *fPtr();
	fPtr = fopen("processRequest.dat","rb");
	if(fPtr==NULL){
		perror("Error opening file of data\n");
	} else {
		//FILE IS OPENED
	}
}
