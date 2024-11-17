#include "mylib.h"

#define MAX_PROCESSES 500
#define QUANTUM 5

// Variables globales para estadísticas
int procesos_ejecutados = 0, procesos_aniquilados = 0, tiempo_total_espera = 0;

// Identificadores de memoria compartida y semáforos
int shm_id, sem_id;
lista_t *shared_data;

// Archivo de estadísticas
FILE *estadisticas_file; // generado

// Funciones de manejo de semáforos
void down() {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void up() {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}


// Proceso lector recibe como parametro el archivo processRequest.dat(en función)
void LargoPlazo(char *name){
FILE *fPtr = fopen(name, "rb");
}

// Proceso despachador (en función)
void CortoPlazo(){

}

// Módulo estadistica
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
        despachador();
    }

}