#include "mylib.h"

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

// Función para liberar los recursos
void freeResources(int semid, int shmid, char *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
	 semctl(semid, 0, IPC_RMID); // elimina los semaforos 
}

// Función para inicializar la memoria compartida
void createdSharedMemory(key_t key, lista_t *shm_ptr){
	int shmid = shmget(key, sizeof(lista_t), IPC_CREAT | 0666);  // Crear memoria compartida
    if (shmid == -1) {
        perror("Error al crear memoria compartida");
        exit(1);
    }
}


// funciones para los semaforos 
void semDown(int num_sem, int semid){
	struct sembuf down = {num_sem, -1, 0}; 
	semop(semid, &down, 1);
}

void semUp(int num_sem, int semid){
	struct sembuf up = {num_sem, 1, 0}; 
	semop(semid, &up, 1);

}

int main(){
	key_t key_sem = ftok("/bin/ls", 1); // llave semaforo 
    key_t key_shm_1 = ftok("/bin/ls", 2); // llave memoria compartida

	int semid=semget(key_sem, 3, IPC_CREAT | 0600);
	if (semid == -1) {
        perror("Error al crear semáforos");
        exit(1);
    }

	semctl(semid, 0, SETVAL, 1); // Inicializa el primer semáforo a 1
	semctl(semid, 1, SETVAL, 0); // Inicializa el segundo semáforo a 0
	semctl(semid, 2, SETVAL, 0); // Inicializa el tercer semáforo a 0



	FILE *fPtr;
	int n, k;
	fPtr = fopen("processRequest.dat","rb");
	signal(SIGTERM,finish);
	if(fPtr==NULL){
		perror("Error opening file of data\n");
		// tenemos de liberar los recursos  si hay algun error
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
	fclose(fPtr);
	return 0;
}
