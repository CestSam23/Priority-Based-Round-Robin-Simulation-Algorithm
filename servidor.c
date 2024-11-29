#include "mylib.h"

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
void sendToClient(int client_socket, process_t response);
void manejar_cliente(int client_socket);

int countProcessExecuted = 0, countProcessAnnihilated = 0, totalWaitTime = 0; // variables para contadores de procesos
FILE *ProcessDispatcherStatistics; 
void estadisticas();

/* FUNCIONES DE FINALIZACIÓN */ 
void manejar_sigterm(int sig);
void freeResources(int semid, int shmid, lista_t *shm_ptr);
void manejar_sigchld(int sig);

int client_socket;

int main(){
    // Inicialización de la memoria compartida y semáforos
    shm_id = shmget(IPC_PRIVATE, sizeof(lista_t), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Error al obtener memoria compartida");
        exit(1);
    }

    shared_data = (lista_t *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Error al asignar memoria compartida");
        exit(1);
    }

    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);   // Semáforo para acceso exclusivo a la memoria compartida
    empty_count = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // Semáforo para contar si hay procesos disponibles
    if (sem_id == -1 || empty_count == -1) {
        perror("Error al crear semáforos");
        exit(1);
    }

    semctl(sem_id, 0, SETVAL, 1);      // Inicializamos semáforo para acceso exclusivo a memoria compartida
    semctl(empty_count, 0, SETVAL, 0); // Inicializamos el contador de procesos en memoria como vacío

    signal(SIGTERM, manejar_sigterm);
    signal(SIGCHLD, manejar_sigchld);  // Manejar procesos hijos terminados

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
        close(server_fd);
        exit(1);
    }

    // Escuchar conexiones
    if (listen(server_fd, 5) < 0) {
        perror("Error al escuchar conexiones");
        close(server_fd);
        exit(1);
    }

    printf("Servidor esperando conexiones en el puerto %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        printf("Conexión aceptada desde %s\n", inet_ntoa(client_addr.sin_addr));

        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al crear proceso hijo");
            close(client_socket);
        } else if (pid == 0) {
            manejar_cliente(client_socket);
            close(client_socket);
            exit(0);
        } else {
            close(client_socket);
        }
    }
    close(server_fd);
    return 0;
}

void manejar_cliente(int client_socket) {
    process_t proceso, response;

    printf("Servidor listo para recibir procesos del cliente.\n");

    while (1) {
        // Recibir el proceso desde el cliente
        ssize_t bytes_received = recv(client_socket, &proceso, sizeof(process_t), 0);
        
        if (bytes_received == 0) {
            // Cliente cerró la conexión
            printf("El cliente cerró la conexión.\n");
            break;
        } else if (bytes_received < 0) {
            perror("Error al recibir el proceso");
            break;
        }

        printf("Proceso recibido: ID=%d, Nombre=%s, CPU Burst=%d\n", 
                proceso.id, proceso.name, proceso.cpuBurst);

        // Sincronizar acceso a la memoria compartida
        down(sem_id);
        addProcess(shared_data, proceso); // Agregar proceso a la lista compartida
        up(sem_id);

        // Despachar procesos usando el planificador de corto plazo
        CortoPlazo();

        // Crear y enviar respuesta al cliente
        response.id = proceso.id;
        response.tWaiting = proceso.tWaiting;
        response.cpuBurst = proceso.cpuBurst;
        
        sendToClient(client_socket, response);
    }

    close(client_socket); // Cerrar el socket del cliente
    printf("Conexión con cliente terminada.\n");
}


void manejar_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void down(int sem_id) {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void up(int sem_id) {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}

void sendToClient(int client_socket, process_t response) {
    printf("\nEnviando respuesta al cliente...");

    ssize_t bytes_sent = send(client_socket, &response, sizeof(process_t), 0);
    if (bytes_sent < 0) {
        perror("Error al enviar la respuesta al cliente");
    } else {
        printf("\nRespuesta enviada al cliente");
    }
}

/* CORTO PLAZO */
void CortoPlazo() {
        while (!isEmpty(shared_data)) {
        down(sem_id);
        if (isEmpty(shared_data)) {
            up(sem_id);
            break;
        }
        // Leer los datos de la memoria compartida y almacenarlos en listaDeProcesos
        printf("\nProcesos disponibles para leer: %d\n", size(shared_data));  // Depuración: cuántos procesos hay
        for (int i = 0; i < size(shared_data); i++) {
            printf("Proceso leido: %s\n", actual(shared_data).name);  // Depuración: muestra los procesos leídos
            addProcess(&listaDeProcesos, actual(shared_data));
            next(shared_data);
        }
        // Despachar procesos según la política
        process_t proceso_actual = deleteProcess(shared_data);
        up(sem_id);

        printf("Ejecutando proceso %s...\n", proceso_actual.name);
        roundRobin();
        priority();
        sleep(proceso_actual.cpuBurst); // Simula ejecución
        proceso_actual.tCompletition = time(NULL);
        printf("Proceso %s terminado.\n", proceso_actual.name);

        // Enviar datos a estadísticas
        sendToAnalytics(proceso_actual);
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

    process_t response;
    response.id = actual(&listaDeProcesos).id;
    response.tWaiting= actual(&listaDeProcesos).tWaiting;
    response.cpuBurst = actual(&listaDeProcesos).cpuBurst;
    sendToClient(client_socket, response);
}

void priority() {
    int iterations = size(&listaDeProcesos);
    for (int i = 0; i < iterations; i++) {
        int remain = actual(&listaDeProcesos).cpuBurst;
        aumentarEspera(&listaDeProcesos, remain);
        aumentarTerminacion(&listaDeProcesos, remain);
        restarEjecucion(&listaDeProcesos, remain);

        printf("El proceso %s ha sido despachado por completo con prioridades\n", actual(&listaDeProcesos).name);

        // Crear la respuesta del servidor
        process_t response;
        response.id = actual(&listaDeProcesos).id;
        response.tWaiting = actual(&listaDeProcesos).tWaiting;
        response.cpuBurst = actual(&listaDeProcesos).cpuBurst;

        // Enviar la respuesta al cliente
        sendToClient(client_socket, response);

        // Enviar los datos a Analytics y eliminar el proceso de la lista
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
}

/* FUNCIONES DE FINALIZACIÓN */
void manejar_sigterm(int sig) {
    printf("Recibiendo SIGTERM. Generando estadísticas...\n");
    estadisticas();
    freeResources(sem_id, shm_id, shared_data);
    exit(0);
}

void freeResources(int semid, int shmid, lista_t *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
    semctl(semid, IPC_RMID, 0);  // Elimina el semáforo
}