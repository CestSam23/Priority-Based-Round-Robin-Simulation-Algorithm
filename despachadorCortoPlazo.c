#include "mylib.h"

#define QUANTUM 5
int semid;

//Para memoria compartida
key_t generateKey(const char *path, int id);
int createdSharedMemory(key_t key, size_t size);
process_t *atSharedMemory(int shmid);
void freeResources(int shmid, int semid, process_t *shm_ptr);

// Para los semáforos
int createdSemaphore(key_t key, int num_sem);
void down(int semid, int sem_num);
void up(int semid, int sem_num);

void roundRobin();
void priority();
void sendToAnalytics(process_t process);
void despachar();
void finish();


lista_t listaDeProcesos;

int main(){
	signal(SIGTERM, finish);

	key_t keyShmDispatcher = generateKey("/bin/ls", 2);
    int shmid=createdSharedMemory(keyShmDispatcher, sizeof(process_t));
	process_t *shm_ptr_dispatcher= atSharedMemory(shmid);

    // Crear semáforos para sincronización
    key_t keySemDispatcher = ftok("/bin/ls", 3);
    semid= createdSemaphore(keySemDispatcher, 2);

    // Inicializar semáforos
    semctl(semid, 0, SETVAL, 0);  // Semáforo de escritura (inicializado a 0)
    semctl(semid, 1, SETVAL, 1);  // Semáforo de lectura (inicializado a 1)

	/*
	Entraremos en un estado constante de espera de lotes de procesos por el despachador de largo plazo
	Con ayuda de semaforos sabremos cuando hay listo. Y con memoria compartida, los leemos en lista
	*/

	/*EJEMPLO*/
	/*Por mientras, creamos una estructura de ejemplo*/
	process_t p1 = {1,23,0,0,0,"pro1\0"};
	process_t p2 = {2,5,0,0,0,"pro2\0"};
	process_t p3 = {3,6,0,0,0,"pro3\0"};
	process_t p4 = {4,9,0,0,0,"pro4\0"};
	process_t p5 = {5,2,0,0,0,"pro5\0"};
	process_t p6 = {6,10,0,0,0,"pro6\0"};
	process_t p7 = {7,35,0,0,0,"pro7\0"};
	

	process_t prs[7] = {p1,p2,p3,p4,p5,p6,p7};

	//La funcion addProcesses añada un arreglo de procesos a la lista. De igual forma, recibe el tamaño
	//De lotes con la que se leera
	addProcesses(prs,7);
	printf("\t\t PROCESOS AÑADIDOS\n");

	
	/*
	Despachar hace la ejecución de Round Robin y prioridades
	Ambas envian los procesos terminados a la funcion para enviar al modulo de
	estadística.
	Cuando haya lotes listos para leer, podemos solamente llamar a despachar
	*/
	down(semid,0); // Se bloquea para escritura
	despachar();
	up(semid,1); // Se desbloquea para lectura

	//Espera de otro lote de memoria
	
	freeResources(semid, shmid, shm_ptr_dispatcher);
	return 0;
}

key_t generateKey(const char *path, int id) {
    key_t key = ftok(path, id);
    if (key == -1) {
        perror("Error al generar clave de memoria compartida\n");
        exit(EXIT_FAILURE);
    }
    return key;
}

int createdSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error al acceder a memoria compartida");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

process_t *atSharedMemory(int shmid) {
    process_t *shm_ptr = (process_t *)shmat(shmid, 0, 0);
    if (shm_ptr == (void *)-1) {
        perror("Error al asociar memoria compartida\n");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

int createdSemaphore(key_t key, int num_sem){
  int semid = semget(key, num_sem, IPC_CREAT | 0666);  // Dos semáforos: uno para lectura, otro para escritura
    if (semid == -1) {
        perror("Error al crear semáforos");
        exit(EXIT_FAILURE);
    }
}

void down(int semid, int sem_num) {
    struct sembuf sops = {sem_num, -1, 0}; // Espera en semáforo
    semop(semid, &sops, 1);
}

void up(int semid, int sem_num) {
    struct sembuf sops = {sem_num, 1, 0}; // Señaliza que se terminó la operación
    semop(semid, &sops, 1);
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
	key_t keyShmDispatcher = generateKey("/bin/ls", 2);
    int shmid = createdSharedMemory(keyShmDispatcher, sizeof(process_t));

	process_t *shm_ptr = atSharedMemory(shmid);
    // Escribir el proceso en la memoria compartida
    *shm_ptr = process;
    // Sincronizar semáforos si es necesario antes de que otro proceso lea este dato
    up(semid, 0);
}

void freeResources(int semid, int shmid, process_t *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
	shmctl(semid, IPC_RMID, 0);  
}

void finish(){
	printf("SALIENDO DEL DESPACHADOR...\n");
	exit(EXIT_FAILURE);
}
