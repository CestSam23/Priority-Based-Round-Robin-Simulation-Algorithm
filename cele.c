#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>

#define MAX_PROCESSES 500
#define QUANTUM 5

typedef struct {
    int id;
    int burst_time;
} Process;

typedef struct {
    Process queue[MAX_PROCESSES];
    int front;
    int rear;
    int count;
} SharedData;

// Variables globales para estadísticas
int procesos_ejecutados = 0;
int procesos_aniquilados = 0;
int tiempo_total_espera = 0;

// Identificadores de memoria compartida y semáforos
int shm_id, sem_id;
SharedData *shared_data;

// Archivo de estadísticas
FILE *estadisticas_file;

// Funciones de manejo de semáforos
void lock() {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void unlock() {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}

// Comparador para ordenar procesos por prioridad (bursts menores, mayor prioridad)
int comparar_prioridades(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    return p1->burst_time - p2->burst_time;
}

// Función para manejar SIGTERM
void manejar_sigterm(int sig) {
    printf("\nRecibiendo señal SIGTERM, generando estadísticas...\n");

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

    // Liberar memoria compartida y semáforos
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    exit(0);
}

// Proceso despachador
void despachador() {
    int round_robin_completado = 0;

    while (1) {
        lock();
        if (shared_data->count > 0) {
            int n = shared_data->count;
            Process temp_queue[MAX_PROCESSES];

            for (int i = 0; i < n; i++) {
                temp_queue[i] = shared_data->queue[(shared_data->front + i) % MAX_PROCESSES];
            }
            unlock();

            if (!round_robin_completado) {
                printf("Ejecutando Round Robin con quantum %d\n", QUANTUM);
                for (int i = 0; i < n; i++) {
                    Process *p = &temp_queue[i];
                    printf("Ejecutando proceso %d (ráfaga restante: %d)\n", p->id, p->burst_time);

                    if (p->burst_time > QUANTUM) {
                        sleep(QUANTUM);
                        p->burst_time -= QUANTUM;
                    } else {
                        sleep(p->burst_time);
                        p->burst_time = 0;
                        procesos_ejecutados++;
                        printf("Proceso %d completado en Round Robin\n", p->id);
                    }
                }
                round_robin_completado = 1;
            }

            printf("Asignando prioridades según ráfaga restante\n");
            qsort(temp_queue, n, sizeof(Process), comparar_prioridades);

            for (int i = 0; i < n; i++) {
                Process *p = &temp_queue[i];
                if (p->burst_time > 0) {
                    printf("Ejecutando proceso %d con prioridad alta (ráfaga restante: %d)\n", p->id, p->burst_time);
                    tiempo_total_espera += p->burst_time;
                    sleep(p->burst_time);
                    p->burst_time = 0;
                    procesos_ejecutados++;
                    printf("Proceso %d completado en despacho por prioridades\n", p->id);
                }
            }

            lock();
            shared_data->count = 0;
            unlock();
        } else {
            unlock();
            sleep(1);
        }
    }
}

// Proceso lector
void lector(const char *archivo) {
    FILE *file = fopen(archivo, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    while (1) {
        int n = rand() % 20 + 1;
        for (int i = 0; i < n; i++) {
            Process p;
            fscanf(file, "%d %d", &p.id, &p.burst_time);

            if (feof(file)) break;

            lock();
            if (shared_data->count < MAX_PROCESSES) {
                shared_data->queue[shared_data->rear] = p;
                shared_data->rear = (shared_data->rear + 1) % MAX_PROCESSES;
                shared_data->count++;
                printf("Proceso %d agregado a la cola (ráfaga: %d)\n", p.id, p.burst_time);
            }
            unlock();
        }
        sleep(rand() % 10 + 1);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_procesos>\n", argv[0]);
        exit(1);
    }

    shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(sem_id, 0, SETVAL, 1);

    shared_data->front = 0;
    shared_data->rear = 0;
    shared_data->count = 0;

    signal(SIGTERM, manejar_sigterm);

    if (fork() == 0) {
        lector(argv[1]);
    } else {
        despachador();
    }

    return 0;
}
