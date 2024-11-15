#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

struct Process{
	pid_t id;
	int cpuBurst;
	int tCompletition;
	int tWaiting;
	int priority;
	char name[5];
};

void finish(){
	printf("Program Finished\n");
	exit(1);
}

/*
				Despachador de Largo Plazo
	Este despachador se apoya del archivo processRequest.dat para leer procesos
	Lee n cantidad de datos (De 1 a 20) cada k segundos (1-10) m veces
	Estos datos leidos son enviados al despachador de Corto Plazo
	Siempre esta a la espera de leer nuevos datos

*/

int main(){
	key_t key_sem = ftok("/bin/ls", 1); // llave semaforo 
    key_t key_shm = ftok("/bin/ls", 2); // llave memoria compartida

	FILE *fPtr;
	int n, k;
	fPtr = fopen("processRequest.dat","rb");
	signal(SIGTERM,finish);
	if(fPtr==NULL){
		perror("Error opening file of data\n");
	} else {
		//FILE IS OPENED

		//Simulando el estado permanente de lectura, usaremos while 1
		//Proximamente reemplazado por el semaforo
		printf("%-6s%-16s%-16s%-6s%-6s\n","Id", "CPU Burst", "Name","CTime","WTime");
		while(1){
			//Leemos n cantidad de datos. De 1 a 20
			n = (rand()%20) + 1;
			//Esperaremos k segundos. de 1-10
			k = (rand()%10) + 1;
			//Lee n cantidad de datos
			struct Process readed[n];
			if((fread(readed, sizeof(struct Process), n, fPtr))==n){
				//Leyó de forma correcta
				/*
					Los datos fueron leidos y estan almacenados
					en un arreglo, se tiene que compartir a
					memoria compartida.
					Status: Impresion para prueba
				*/
				printf("Lote Leido\n");
				for(int i=0; i<n; i++){
					printf("%-6d%-16d%-16s%-6d%-6d\n", readed[i].id,readed[i].cpuBurst, readed[i].name, readed[i].tCompletition, readed[i].tWaiting);
				}
			} else {	//Error handling
				if(feof(fPtr)){
					perror("Unexpected EOF\n");
				} else if (ferror(fPtr)){
					perror("Error reading file\n");
				}
			}
			sleep(k);
		}
	}
}
