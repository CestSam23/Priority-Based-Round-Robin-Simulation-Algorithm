#include "mylib.h"
#define MAX_PROCESSES 1000
#define QUANTUM 5

/* SEMAFOROS Y MEMORIA COMPARTIDA */
int shm_id, sem_id, empty_count; // Identificadores de Semáforos
lista_t *shared_data; // Identificadores de Memoria Compartida
void down(int sem_id);
void up(int sem_id);

/* MODULO LARGO PLAZO */
void LargoPlazo(char *name);

/* MODULO CORTO PLAZO */
void CortoPlazo();

/*  MODULO ESTADISTICA*/
int countProcessExecuted = 0, countProcessAnnihilated = 0, totalWaitTime = 0; // variables para contadores de procesos
FILE *ProcessDispatcherStatistics; // archivo generado para estadisticas
void estadisticas();

/* FUNCIONES DE FINALIZACIÓN */ 
void manejar_sigterm(int sig);
void freeResources(int semid, int shmid, lista_t *shm_ptr);


int main(){
    char *archivo_entrada= "processRequest.dat";
    // Inicialización de la memoria compartida y semáforos
    shm_id = shmget(IPC_PRIVATE, sizeof(lista_t), IPC_CREAT | 0666);
    shared_data = (lista_t *)shmat(shm_id, NULL, 0);

    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);   // Semáforo para acceso exclusivo a la memoria compartida
    empty_count = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // Semáforo para contar si hay procesos disponibles

    semctl(sem_id, 0, SETVAL, 1);      // Inicializamos semáforo para acceso exclusivo a memoria compartida
    semctl(empty_count, 0, SETVAL, 0); // Inicializamos el contador de procesos en memoria como vacío

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

/* MANEJO DE SEMÁFOROS */
void down(int sem_id) {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void up(int sem_id) {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}

/* LARGO PLAZO */
void LargoPlazo(char *name){
    FILE *fPtr = fopen(name, "rb");
    int n, k;
    if (fPtr == NULL) {
        perror("Error abriendo el archivo de datos\n");
        exit(1);
    } else {
        while (1) {
            n = (rand() % 20) + 1;  // Número aleatorio de procesos
            k = (rand() % 10) + 1;  // Tiempo aleatorio de espera
            process_t readed[n];
            
            if (fread(readed, sizeof(process_t), n, fPtr) == n) {
                // Procesos leídos correctamente
                printf("Procesos leídos: %d\n", n);  // Depuración: cuántos procesos leímos
                down(sem_id);  // Sincronizar acceso a la memoria
                for (int i = 0; i < n; i++) {
                    shared_data->procesos[shared_data->size++] = readed[i];  // Escribir en la memoria compartida
                }
                up(empty_count);  // Señalar que hay nuevos procesos disponibles
                up(sem_id);       // Liberar acceso a la memoria compartida
                printf("Procesos almacenados en la memoria compartida.\n");  // Depuración
            } else {
                if (feof(fPtr)) {
                    printf("Fin de archivo alcanzado.\n");  // Depuración: fin de archivo
                    break;
                } else if (ferror(fPtr)) {
                    perror("Error al leer el archivo\n");
                    break;
                }
            }
            sleep(k);  // Simula el tiempo de espera
        }
    }
}

/* CORTO PLAZO */

lista_t listaDeProcesos;
void roundRobin();
void priority();
void sendToAnalytics(process_t process);

void CortoPlazo(){
    while (1) {
        down(empty_count);  // Esperar hasta que haya procesos disponibles
        down(sem_id);  // Sincronizar acceso a la memoria compartida
        
        // Leer los datos de la memoria compartida y almacenarlos en listaDeProcesos
        printf("Procesos disponibles para leer: %d\n", shared_data->size);  // Depuración: cuántos procesos hay
        for (int i = 0; i < shared_data->size; i++) {
            printf("Proceso leido: %s\n", shared_data->procesos[i].name);  // Depuración: muestra los procesos leídos
            listaDeProcesos.procesos[i] = shared_data->procesos[i];
        }
        listaDeProcesos.size = shared_data->size;
        shared_data->size = 0;  // Limpiar memoria compartida después de leer los procesos

        up(sem_id);  // Liberar acceso a la memoria compartida
        
        // Despachar procesos
        if (listaDeProcesos.size > 0) {
            roundRobin();
            priority();
        }
        printf("\tLOTE DESPACHADO\n");
    }
}


void roundRobin(){
    // Iniciamos el Round Robin en la lista de procesos
    rewindList();
    int iterations = listaDeProcesos.size;
    for (int i = 0; i < iterations; i++) {
        printf("---------------------------Proceso %s ----------------------\n", listaDeProcesos.procesos[i].name);
        // Aquí es donde manejas el tiempo de CPU
        aumentarEspera(QUANTUM);
        aumentarTerminacion(QUANTUM);
        if (restarEjecucion(QUANTUM) == -1) {
            printf("El proceso %s ha sido despachado por completo con RoundRobin\n", listaDeProcesos.procesos[i].name);
            sendToAnalytics(deleteProcess());
            printf("*******************************************************\n\n");
        } else {
            printf("\t\tDespachando proceso: %s\n", listaDeProcesos.procesos[i].name);
            printf("*******************************************************\n\n");
        }
    }
}

void priority(){
    // Despachar por prioridades
    int iterations = listaDeProcesos.size;
    for (int i = 0; i < iterations; i++) {
        printf("---------------------------Proceso %s ----------------------\n", listaDeProcesos.procesos[i].name);
        printf("\t\tDespachando proceso:%s\n", listaDeProcesos.procesos[i].name);

        int remain = listaDeProcesos.procesos[i].cpuBurst;
        aumentarEspera(remain);
        aumentarTerminacion(remain);
        restarEjecucion(remain);

        printf("El proceso %s ha sido despachado por completo con prioridades\n", listaDeProcesos.procesos[i].name);
        sendToAnalytics(deleteProcess());
    }
}


void sendToAnalytics(process_t process){
    if (process.cpuBurst > 0) { 
        countProcessExecuted++; 
    } else { 
        countProcessAnnihilated++; 
    }
    totalWaitTime += process.tWaiting;
    estadisticas();
}



/* ESTADISTICA */
void estadisticas() {
    double averageWaitingTime = countProcessExecuted > 0 ? 
        (double)totalWaitTime / countProcessExecuted : 0.0;

    printf("Estadísticas del despachador:\n");
    printf("Procesos ejecutados: %d\n", countProcessExecuted);
    printf("Procesos aniquilados: %d\n", countProcessAnnihilated);
    printf("Tiempo promedio de espera: %.2f\n", averageWaitingTime);

    ProcessDispatcherStatistics = fopen("estadisticas.txt", "w");
    if (ProcessDispatcherStatistics != NULL) {
        fprintf(ProcessDispatcherStatistics, "Estadísticas del despachador:\n");
        fprintf(ProcessDispatcherStatistics, "Procesos ejecutados: %d\n", countProcessExecuted);
        fprintf(ProcessDispatcherStatistics, "Procesos aniquilados: %d\n", countProcessAnnihilated);
        fprintf(ProcessDispatcherStatistics, "Tiempo promedio de espera: %.2f\n", averageWaitingTime);
        fclose(ProcessDispatcherStatistics);
    }
}

/* FINALIZACION */
// Función para manejar SIGTERM
void manejar_sigterm(int sig) {
    printf("\nRecibiendo señal SIGTERM, generando estadísticas...\n");
    estadisticas();
    freeResources(sem_id, shm_id, shared_data);
    exit(0);
}

// Función de liberación de recursos
void freeResources(int semid, int shmid, lista_t *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
    semctl(semid,IPC_RMID,0);
}