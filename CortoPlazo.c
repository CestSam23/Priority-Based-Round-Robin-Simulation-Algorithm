#include "mylib.h"

#define QUANTUM 5
extern int semid;
lista_t listaDeProcesos;

//Para memoria compartida
extern key_t generateKey(const char *path, int id);
extern int createdSharedMemory(key_t key, size_t size);
extern lista_t *atSharedMemory(int shmid);
extern void freeResources(int shmid, int semid, lista_t *shm_ptr);

// Para los semáforos
extern int createdSemaphore(key_t key, int num_sem);
extern void down(int semid, int sem_num);
extern void up(int semid, int sem_num);

void roundRobin();
void priority();
void sendToAnalytics(process_t process);
void despachar();
extern void finish();

lista_t listaDeProcesos;

int twomain(){
	signal(SIGTERM, finish);

	key_t keyShmDispatcher = generateKey(".", 2);
    int shmid = createdSharedMemory(keyShmDispatcher, sizeof(lista_t));
	lista_t *shm_ptr_dispatcher = atSharedMemory(shmid);

    // Crear semáforos para sincronización
    key_t keySemDispatcher = ftok(".", 3);
    semid= createdSemaphore(keySemDispatcher, 2);

    // Inicializar semáforos
    semctl(semid, 0, SETVAL, 0);  // Semáforo de escritura (inicializado a 0)
    semctl(semid, 1, SETVAL, 1);  // Semáforo de lectura (inicializado a 1)

	// Bucle principal
    while (1) {
        down(semid, 0); // Espera a que el despachador de largo plazo escriba

        // Procesar los procesos desde la memoria compartida
		printf("Procesos recibidos, iniciando despacho...\n");
        despachar();

        up(semid, 1); // Notificar al despachador de largo plazo que puede escribir
    }

	//Espera de otro lote de memoria
	
	freeResources(semid, shmid, shm_ptr_dispatcher);
	return 0;
}

void despachar(){
	printf("\t\t ROUND ROBIN: \n\n");
	roundRobin();

	printf("\n\t\tROUND ROBIN TERMINADO.\n\n");
	

	printf("\t\tINICIANDO ALGORITMO PRIORIDADES\n\n");
	ordenarPorPrioridad();
	printf("\tINICIO\n\n");
	priority();
	printf("\tPROCESOS DESPACHADOS\n");
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
			printf("El proceso %s ha sido despachado por completo\n",actual().name);
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

		
		printf("El proceso %s ha sido despachado por completo\n",actual().name);

		sendToAnalytics(deleteProcess());
	
	}
}

void sendToAnalytics(process_t process){
	//MEMORIA COMPARTIDA. ENVIADA A ANALISIS
	key_t keyShmDispatcher = generateKey(".", 2);
    int shmid = createdSharedMemory(keyShmDispatcher, sizeof(process_t));

	process_t *shm_ptr = atSharedMemory(shmid);
    // Escribir el proceso en la memoria compartida
    *shm_ptr = process;
    // Sincronizar semáforos si es necesario antes de que otro proceso lea este dato
    up(semid, 0);
}