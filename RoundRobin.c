#include "mylib.h"

#define MAX_PROCESSES 500
#define QUANTUM 5

/*
    *************DEFINICION DE FUNCIONES QUE OCUPAREMOS**********
*/

//FUNCIONES PARA AUMENTAR/DECREMENTAR SEMAFOROS
void down();
void up();

//FUNCIONES DE FINALIZACIÓN
void manejar_sigterm(int sig);
void freeResources(int semid, int shmid, lista_t *shm_ptr);

//MODULOS DE LOGICA DE DESPACHADOR
void LargoPlazo(char *name);
void CortoPlazo();
void estadisticas();
/*  
    ***************FIN DE DEFINICIONES************
*/


// Identificadores de memoria compartida y semáforos
int shm_id, sem_id;
lista_t *shared_data;


/*
    ****************PROGRAMA PRINCIPAL****************
*/

int main(){
    char *archivo_entrada= "processRequest.dat";
   
    // Inicialización de la memoria compartida y semáforos
    shm_id = shmget(IPC_PRIVATE, sizeof(lista_t), IPC_CREAT | 0666);
    shared_data = (lista_t *)shmat(shm_id, NULL, 0);

    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(sem_id, 0, SETVAL, 1);

    // Configuración de la memoria compartida (inicializar puntero)
    shared_data->actual = 0;
    shared_data->size = 0;

    signal(SIGTERM, manejar_sigterm);
    if (fork() == 0) {
        LargoPlazo(archivo_entrada);
    } else {
        CortoPlazo();
    }

}



/**
    *******************MANEJO DE SEMAFOROS********************
 */
void down() {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void up() {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}




/*
    ****************DESPACHADOR LARGO PLAZO*************************
*/

void LargoPlazo(char *name){
    
    FILE *fPtr = fopen(name,"rb");
    int n,k;
    if(fPtr==NULL){
        perror("Error openin file of data\n");
    } else {
        while(1){
            n = (rand()%20) + 1;
            k = (rand()%10) + 1;
            process_t readed[n];
            if((fread(readed,sizeof(process_t),n,fPtr))==n){
                //Proceso Leido y Almacenado en readed
                //Enviar a memoria compartida readed

            } else {
                if(feof(fPtr)){
                    perror("Unexpected EOF\n");
                } else if(ferror(fPtr)){
                    perror("Error reading file\n");
                    break;
                }
            }
            sleep(k);
        }
    }

    //CONSIDERAR
    //Nunca se cerrará fclose, ya que se ejecuta sigterm
    fclose(fPtr);
    
}




/*
    **************DESPACHADOR CORTO PLAZO**************************
*/

void roundRobin();
void priority();
void sendToAnalytics(process_t process);
lista_t listaDeProcesos;


void CortoPlazo(){
    while(1){
        //Recibir datos constantemente de memoria compartida
        //Almacenarlos en listaDeProcesos
    	roundRobin();
	    ordenarPorPrioridad();
	    priority();
	    printf("\tLOTE DESPACHADO\n");
    }
}

void roundRobin(){
    /*
	Primera y única ronda de Round Robin
	Recordemos que por documentacion addProcesses deja
	apuntando actual al primer elemento agregado
	*/
	rewindList();
	int iterations = size();
	for(int i=0; i<iterations; i++){
		printf("---------------------------Proceso %s ----------------------\n", actual().name);
		aumentarEspera(QUANTUM);
		aumentarTerminacion(QUANTUM);
		//Se terminó Proceso, enviar a modulo de estadistica
		if(restarEjecucion(QUANTUM)==-1){
			printf("El proceso %s ha sido despachado por completo con RoundRobin\n",actual().name);
			sendToAnalytics(deleteProcess());
			printf("*******************************************************\n\n");
			//No es necesario avanzar. La función delete process
			//Recorre en automático
		} else {
			//Avanzamos a la siguiente estructura
			//En ultimo elemento, regresamos al inicio
			
			printf("\t\tDespachando proceso:%s\n", actual().name);
			next();
			printf("*******************************************************\n\n");
		}
		
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
		printf("---------------------------Proceso %s ----------------------\n", actual().name);
		printf("\t\tDespachando proceso:%s\n", actual().name);

		int remain = actual().cpuBurst;
		
		aumentarEspera(remain);
		aumentarTerminacion(remain);
		restarEjecucion(remain);

		
		printf("El proceso %s ha sido despachado por completo con prioridades\n",actual().name);

		sendToAnalytics(deleteProcess());
	
	}
}

/*
    Función que se encarga de enviar proceso por proceso a la memoria compartida para que la funcion de estadistica pueda leer y almacenar
*/
void sendToAnalytics(process_t process){
	
}


/*
    **************MODULO DE ESTADISTICA*****************
*/

// Variables para estadistica
int procesos_ejecutados = 0, procesos_aniquilados = 0, tiempo_total_espera = 0;

// Módulo estadistica
FILE *estadisticas_file; // generado

void estadisticas(){
    // Cálculo del tiempo promedio de espera
    double tiempo_promedio_espera = procesos_ejecutados > 0 ? 
        (double)tiempo_total_espera / procesos_ejecutados : 0.0;

    // Imprimir estadísticas en pantalla
    printf("Estadísticas del despachador:\n");
    printf("Procesos ejecutados: %d\n", procesos_ejecutados);
    printf("Procesos aniquilados: %d\n", procesos_aniquilados);
    printf("Tiempo promedio de espera: %.2f\n", tiempo_promedio_espera);

    // Escribir estadísticas en archivo
    estadisticas_file = fopen("estadisticas.txt", "w");
    if (estadisticas_file != NULL) {
        fprintf(estadisticas_file, "Estadísticas del despachador:\n");
        fprintf(estadisticas_file, "Procesos ejecutados: %d\n", procesos_ejecutados);
        fprintf(estadisticas_file, "Procesos aniquilados: %d\n", procesos_aniquilados);
        fprintf(estadisticas_file, "Tiempo promedio de espera: %.2f\n", tiempo_promedio_espera);
        fclose(estadisticas_file);
    }
}







//FINALiZACIÓn DE PROGRAMA
// Función para manejar SIGTERM
void manejar_sigterm(int sig) {
    printf("\nRecibiendo señal SIGTERM, generando estadísticas...\n");
    estadisticas(); 
    // Liberar memoria compartida y semáforos
    freeResources(sem_id, shm_id, shared_data);
    exit(0);
}

void freeResources(int semid, int shmid, lista_t *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
	 semctl(semid, 0, IPC_RMID); // elimina los semaforos 
}