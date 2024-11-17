#include "mylib.h"

// Variables globales para estadísticas
int countProcessExecuted = 0, countProcessAnnihilated = 0, totalWaitTime = 0;

// Función para generar la clave de la memoria compartida
key_t generateKey(const char *path, int id) {
    key_t key = ftok(path, id);
    if (key == -1) {
        perror("Error al generar clave de memoria compartida\n");
        exit(1);
    }
    return key;
}

// Función para acceder a la memoria compartida
int accessSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, 0666);
    if (shmid == -1) {
        perror("Error al acceder a memoria compartida");
        exit(1);
    }
    return shmid;
}

// Función para atar la memoria compartida
process_t *atSharedMemory(int shmid) {
    process_t *shm_ptr = (process_t *)shmat(shmid, 0, 0);
    if (shm_ptr == (void *)-1) {
        perror("Error al asociar memoria compartida\n");
        exit(1);
    }
    return shm_ptr;
}

// Función para liberar los recursos de memoria compartida
void freeResources(int shmid, int semid, process_t *shm_ptr) {
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
}

// Función para procesar las estadísticas de un proceso
void processStats(process_t process) {
    if (process.tCompletition == 0) {
        countProcessAnnihilated++;
    } else {
        countProcessExecuted++;
        int waitTime = process.tCompletition - process.cpuBurst;
        totalWaitTime += waitTime;
    }
}

// Función para generar el reporte en un archivo
void generateReport() {
    FILE *fp = fopen("report.txt", "w");
    if (fp == NULL) {
        perror("Error al abrir archivo para escritura\n");
        exit(1);
    }

    fprintf(fp, "Número de procesos ejecutados: %d\n", countProcessExecuted);
    fprintf(fp, "Número de procesos aniquilados: %d\n", countProcessAnnihilated);
    fprintf(fp, "Tiempo promedio de espera: %.2f\n",
            countProcessExecuted > 0 ? (double)totalWaitTime / countProcessExecuted : 0.0);
    fclose(fp);

    printf("Reporte generado en 'report.txt'.\n");
}

// Función para manejar las señales y generar el reporte
void signal_handler(int sig) {
    generateReport();
    exit(0);
}

// Función para crear y obtener el semáforo
int createSemaphore(const char *path, int id) {
    key_t key_sem = generateKey(path, id);
    int semid = semget(key_sem, 2, 0666);  // Dos semáforos: uno para lectura, otro para escritura
    if (semid == -1) {
        perror("Error al obtener semáforos");
        exit(1);
    }
    return semid;
}

// Función para realizar la operación en el semáforo
void up(int semid, int semNum) {
    struct sembuf sem_op = {semNum, 1, 0};
    if (semop(semid, &sem_op, 1) == -1) {
        perror("Error en la operación con semáforo");
        exit(EXIT_FAILURE);
    }
}

void down(int semid, int semNum) {
    struct sembuf sem_op = {semNum, -1, 0};
    if (semop(semid, &sem_op, 1) == -1) {
        perror("Error en la operación con semáforo");
        exit(EXIT_FAILURE);
    }
}

// Función principal que maneja el ciclo de lectura y procesamiento de procesos
void processLoop(process_t *shm_ptr_stats, int semid) {
    while (1) {
        if (shm_ptr_stats != NULL) {
            // Espera semáforo de escritura
            down(semid, 0);

            // Procesar estadísticas
            processStats(*shm_ptr_stats);

            // Avanzar a la siguiente estructura de proceso
            shm_ptr_stats++;

            // Señaliza semáforo de lectura para permitir que el despachador escriba el siguiente proceso
            up(semid, 1);
        }
    }
}

int main() {
    // Manejador de señales
    signal(SIGTERM, signal_handler);

    // Configuración de memoria compartida
    key_t keyShmStats = generateKey("/bin/ls", 2);
    int shmidStats = accessSharedMemory(keyShmStats, sizeof(process_t));
    process_t *shm_ptr_stats = atSharedMemory(shmidStats);

    // Crear semáforos para sincronización
    int semid = createSemaphore("/bin/ls", 3);

    // Procesamiento de los procesos
    processLoop(shm_ptr_stats, semid);

    // Liberación de recursos
    freeResources(shmidStats, semid, shm_ptr_stats);
    return 0;
}