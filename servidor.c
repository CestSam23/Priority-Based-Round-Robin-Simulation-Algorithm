#include "mylib.h"

int client_socket;
#define MAX_PROCESSES 1000
#define QUANTUM 5
#define PORT 8080 // Puerto del servidor

/* SEMÁFOROS Y MEMORIA COMPARTIDA */
int shm_id, sem_id, empty_count; 
lista_t *shared_data; // Identificadores de Memoria Compartida
void down(int sem_id);
void up(int sem_id);

/* CORTO PLAZO */
lista_t listaDeProcesos = {-1,0}; // Lista para los procesos en corto plazo
void sendToAnalytics(process_t process);
void CortoPlazo();
void roundRobin();
void priority();
void sendToClient(int client_socket, process_t process);
void manejar_cliente(int client_socket);

int countProcessExecuted = 0, countProcessAnnihilated = 0, totalWaitTime = 0; // variables para contadores de procesos
FILE *ProcessDispatcherStatistics; // archivo generado para estadisticas
void estadisticas();

/* FUNCIONES DE FINALIZACIÓN */ 
void manejar_sigterm(int sig);
void freeResources(int semid, int shmid, lista_t *shm_ptr);

int main(){
    // Inicialización de la memoria compartida y semáforos
    shm_id = shmget(IPC_PRIVATE, sizeof(lista_t), IPC_CREAT | 0666);
    shared_data = (lista_t *)shmat(shm_id, NULL, 0);

    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);   // Semáforo para acceso exclusivo a la memoria compartida
    empty_count = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // Semáforo para contar si hay procesos disponibles

    semctl(sem_id, 0, SETVAL, 1);      // Inicializamos semáforo para acceso exclusivo a memoria compartida
    semctl(empty_count, 0, SETVAL, 0); // Inicializamos el contador de procesos en memoria como vacío

    signal(SIGTERM, manejar_sigterm);

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    // Crear socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error al crear el socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar el socket");
        exit(1);
    }

    // Escuchar conexiones
    if (listen(server_fd, 5) < 0) {
        perror("Error al escuchar conexiones");
        exit(1);
    }

    printf("Servidor esperando conexiones en el puerto %d...\n", PORT);

   while (1) {
        // Aceptar conexiones de los clientes
        client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Error al aceptar conexión");
        }
        printf("Conexión aceptada desde %s\n", inet_ntoa(client_addr.sin_addr));
        
        manejar_cliente(client_socket);
    }

    close(server_fd);
    return 0;
}


void manejar_cliente(int client_socket) {
    process_t process;
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, &process, sizeof(process_t), 0)) > 0) {
        printf("Proceso recibido: ID=%d, Nombre=%s, Tiempo de CPU=%d\n", process.id, process.name, process.cpuBurst);

        // Despachar el proceso
        up(empty_count); 
        down(sem_id);  
        addProcess(shared_data, process);
        up(sem_id);  
        // Llamada al algoritmo de corto plazo (despacho de procesos)
        CortoPlazo();
    }

    if (bytes_received == 0) {
        printf("Conexión cerrada por el cliente.\n");
    } else if (bytes_received < 0) {
        perror("Error al recibir el proceso");
    }

    // Cerrar socket del cliente después de recibir todos los procesos
    close(client_socket);
}

/*void manejar_cliente(int client_socket) {
    process_t process;
    ssize_t bytes_received;

    // Recibir proceso del cliente
    bytes_received = recv(client_socket, &process, sizeof(process_t), 0);
    if (bytes_received <= 0) {
        printf("Error al recibir el proceso o conexión cerrada.\n");
    }

    // Despachar el proceso
    up(empty_count); 
    down(sem_id);  
    addProcess(shared_data, process);
    up(sem_id);  
    // Llamada al algoritmo de corto plazo (despacho de procesos)
    CortoPlazo();

    // Cerrar socket del cliente después de recibir todos los procesos
    close(client_socket);
}*/

void sendToClient(int client_socket, process_t process) {
    printf("\nEnviando proceso al cliente...");

    ssize_t bytes_sent = send(client_socket, &process, sizeof(process_t), 0);
    if (bytes_sent < 0) {
        perror("Error al enviar el proceso al cliente");
    } else {
        printf("\nProceso enviado al cliente");
    }
}

/* CORTO PLAZO */
void CortoPlazo() {
    while (1) {
        down(empty_count);  // Esperar hasta que haya procesos disponibles
        down(sem_id);  // Sincronizar acceso a la memoria compartida
       
        // Leer los datos de la memoria compartida y almacenarlos en listaDeProcesos
        printf("\nProcesos disponibles para leer: %d\n", size(shared_data));  // Depuración: cuántos procesos hay
        for (int i = 0; i < size(shared_data); i++) {
            printf("Proceso leido: %s\n", actual(shared_data).name);  // Depuración: muestra los procesos leídos
            addProcess(&listaDeProcesos, actual(shared_data));
            next(shared_data);
        }

        while (isEmpty(shared_data) != 1) {
            deleteProcess(shared_data);
        }

        up(sem_id);  // Liberar acceso a la memoria compartida
        
        // Despachar procesos
        if (size(&listaDeProcesos) > 0) {
            roundRobin();
            priority();
        }
        printf("\nPROCESO DESPACHADO");
    }
}

void roundRobin(){
    // Iniciamos el Round Robin en la lista de procesos
    rewindList(&listaDeProcesos);
    int iterations = size(&listaDeProcesos);
    for (int i = 0; i < iterations; i++) {
       printf("Despachando proceso %s\n", actual(&listaDeProcesos).name);
        // Aquí es donde manejas el tiempo de CPU
        aumentarEspera(&listaDeProcesos, QUANTUM);
        if (restarEjecucion(&listaDeProcesos, QUANTUM) == -1) {
            printf("El proceso %s ha sido despachado por completo con RoundRobin\n", actual(&listaDeProcesos).name);
            sendToAnalytics(deleteProcess(&listaDeProcesos));
        } else {
            aumentarTerminacion(&listaDeProcesos, QUANTUM);
            next(&listaDeProcesos);
        }
    }
}

void priority(){
    // Despachar por prioridades
    int iterations = size(&listaDeProcesos);
    for (int i = 0; i < iterations; i++) {

        int remain = actual(&listaDeProcesos).cpuBurst;
        aumentarEspera(&listaDeProcesos, remain);
        aumentarTerminacion(&listaDeProcesos, remain);
        restarEjecucion(&listaDeProcesos, remain);

        printf("El proceso %s ha sido despachado por completo con prioridades\n", actual(&listaDeProcesos).name);
        sendToClient(client_socket, actual(&listaDeProcesos));
        sendToAnalytics(deleteProcess(&listaDeProcesos));
    }
}

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

void sendToAnalytics(process_t process){
    if (process.tCompletition > 0) { 
        countProcessExecuted++; 
    } else { 
        countProcessAnnihilated++; 
    }
    totalWaitTime += process.tWaiting;
   //estadisticas();
}

void down(int sem_id) {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void up(int sem_id) {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}

/* FUNCIONES DE FINALIZACIÓN */
void manejar_sigterm(int sig) {
    printf("\nRecibiendo señal SIGTERM, generando estadísticas...\n");
    estadisticas();
    freeResources(sem_id, shm_id, shared_data);
    exit(0);
}

void freeResources(int semid, int shmid, lista_t *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
    semctl(semid, IPC_RMID, 0);  // Elimina el semáforo
}