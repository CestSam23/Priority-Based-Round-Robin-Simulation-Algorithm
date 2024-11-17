#include "mylib.h"

// Variables globales para estadísticas
int countProcessExecuted = 0, countProcessAnnihilated = 0, totalWaitTime = 0;

// Función para liberar los recursos de memoria compartida
extern void freeResources(int semid, int shmid, process_t *shm_ptr);
// Función para realizar la operación en el semáforo
extern void up(int semid, int sem_num); 
extern void down(int semid, int sem_num);

extern key_t generateKey(const char *path, int id);
int accessSharedMemory(key_t key, size_t size);
extern process_t *atSharedMemory(int shmid);
void processStats(process_t process);
void generateReport();
void readFileEstadistica();
void signal_handler(int sig);
int createSemaphore(const char *path, int id);
void processLoop(process_t *shm_ptr_stats, int semid);



int threemain() {
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

// Función para acceder a la memoria compartida
int accessSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, 0666);
    if (shmid == -1) {
        perror("Error al acceder a memoria compartida");
        exit(1);
    }
    return shmid;
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

// Abrir archivo para leer e imprimir en terminal 
void readFileEstadistica(){
    FILE *fp_read = fopen("report.txt", "r");
    if (fp_read == NULL) {
        perror("can't open file for reading");
        exit(1);
    }
    char line[MAX];
    while (fgets(line, sizeof(line), fp_read)) {
        printf("%s", line);
    }
    fclose(fp_read);
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
