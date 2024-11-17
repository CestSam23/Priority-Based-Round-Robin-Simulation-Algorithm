#include "mylib.h"
int semid;
void finish(); //func principal 
key_t generateKey(const char *path, int id);

void down(int semid, int num_sem);
void up(int semid, int num_sem);

int createdSharedMemory(key_t key, size_t size);
void freeResources(int semid, int shmid, lista_t*shm_ptr);

lista_t *atSharedMemory(int shmid) {
    lista_t *shm_ptr = (lista_t *)shmat(shmid, 0, 0);
    if (shm_ptr == (void *)-1) {
        perror("Error al asociar memoria compartida\n");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}
int createdSemaphore(key_t key, int num_sem);

int main(){
    signal(SIGTERM, finish);

    key_t keyShmDispatcher = generateKey(".", 2);

	printf("Tamaño de lista_t: %zu bytes\n", sizeof(lista_t));
    int shmid = createdSharedMemory(keyShmDispatcher, sizeof(lista_t));
    lista_t *shm_ptr_dispatcher = atSharedMemory(shmid);

    // Crear semáforos para sincronización
    key_t keySemDispatcher = ftok("/bin/ls", 3);
    semid = createdSemaphore(keySemDispatcher, 2);

    semctl(semid, 0, SETVAL, 0); // Inicializa el semáforo de lectura a 0
    semctl(semid, 1, SETVAL, 1); // Inicializa el semáforo de escritura a 1

    FILE *fPtr;
    int n, k;
    fPtr = fopen("processRequest.dat", "rb");
    if (fPtr == NULL) {
        perror("Error opening file of data\n");
        freeResources(semid, shmid, shm_ptr_dispatcher);
        exit(EXIT_FAILURE);
    } else {
        printf("%-6s%-16s%-16s%-6s%-6s\n", "Id", "CPU Burst", "Name", "CTime", "WTime");
        while (1) {
            n = (rand() % 20) + 1; // Número de procesos (1 a 20)
            k = (rand() % 10) + 1; // Tiempo de espera (1 a 10 segundos)
            struct Process readed[n];

            // Leer procesos del archivo
            if ((fread(readed, sizeof(struct Process), n, fPtr)) == n) {
                printf("Lote Leído:\n");
                for (int i = 0; i < n; i++) {
                    printf("%-6d%-16d%-16s%-6d%-6d\n", readed[i].id, readed[i].cpuBurst, readed[i].name, readed[i].tCompletition, readed[i].tWaiting);
                }

                // Espera a que el despachador de corto plazo procese el lote anterior
                down(semid, 1);

                // Enviar los procesos a la memoria compartida
                addProcesses(readed, n);
                printf("Lote enviado a memoria compartida.\n");

                // Notificar al despachador de corto plazo que hay datos disponibles
                up(semid, 0);
            } else {
                if (feof(fPtr)) {
                    perror("Unexpected EOF jeje\n");
                    
                } else if (ferror(fPtr)) {
                    perror("Error reading file\n");
                    break;
                }
            }
            sleep(k); // Esperar antes de leer el siguiente lote
        }
    }

    freeResources(semid, shmid, shm_ptr_dispatcher);
    fclose(fPtr);
    return 0;
}

// Función para generar la clave de la memoria compartida
key_t generateKey(const char *path, int id) {
    key_t key = ftok(path, id);
    if (key == -1) {
        perror("Error al generar clave de memoria compartida\n");
        exit(1);
    }
    return key;
}


void finish(){
	printf("Program Finished\n");
	exit(1);
}

// funciones para los semaforos 
void down(int semid, int num_sem){
	struct sembuf down = {num_sem, -1, 0}; 
	semop(semid, &down, 1);
}

void up(int semid, int num_sem){
	struct sembuf up = {num_sem, 1, 0}; 
	semop(semid, &up, 1);

}

int createdSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error al crear la memoria compartida ejeje");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

// Función para liberar los recursos
void freeResources(int semid, int shmid, lista_t *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
	 semctl(semid, 0, IPC_RMID); // elimina los semaforos 
}

int createdSemaphore(key_t key, int num_sem){
  int semid = semget(key, num_sem, IPC_CREAT | 0666);  // Dos semáforos: uno para lectura, otro para escritura
    if (semid == -1) {
        perror("Error al crear semáforos");
        exit(EXIT_FAILURE);
    }
}
