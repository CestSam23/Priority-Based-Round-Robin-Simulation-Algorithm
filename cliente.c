#include "mylib.h"

#define PORT 8080 // Puerto del servidor
#define IP "127.0.0.1" //IP local

void enviarProceso(int socket_fd, process_t *process);
int solicitarProceso(process_t *process);
void recibirRespuesta(int socket_fd, process_t *response);

int main() {
    int socket_fd;
    struct sockaddr_in server_addr;
    process_t process, response;
    char continuar = 'y';

    // Crear el socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error al crear el socket");
        exit(1);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);  // Dirección del servidor
    server_addr.sin_port = htons(PORT);  // Puerto del servidor

    // Conectar al servidor
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar al servidor");
        exit(1);
    }

    printf("Conectado al servidor.\n");

    // Enviar procesos uno por uno hasta que el usuario decida parar
    while (continuar == 'y' || continuar == 'Y') {
        // Solicitar datos del proceso al usuario
        if (solicitarProceso(&process) == 1) {
            // Enviar el proceso al servidor
            enviarProceso(socket_fd, &process);
            printf("Proceso enviado al servidor.\n");

            // Esperar la respuesta del servidor
            recibirRespuesta(socket_fd, &response);

            // Mostrar la respuesta del servidor
            printf("\n-----Respuesta del servidor: Proceso %d-----\n", response.id);
            printf("Tiempo de espera: %d ms\n", response.tCompletition);
            printf("Tiempo de ejecución: %d ms\n", response.tWaiting);
        }

        // Preguntar si se desea continuar
        printf("\n¿Deseas enviar otro proceso? (y/n): ");
        scanf(" %c", &continuar);
    }

    // Cerrar el socket
    close(socket_fd);
    return 0;
}

// Función para solicitar los datos del proceso
int solicitarProceso(process_t *process) {
    printf("\nIngrese el nombre del proceso: ");
    scanf("%s", process->name); 

    printf("Ingrese el ID del proceso: ");
    scanf("%d", &process->id);

    printf("Ingrese el tiempo de CPU del proceso: ");
    scanf("%d", &process->cpuBurst); 
    process->tCompletition=0;
    process->tWaiting=0;
    return 1;  // Retorna 1 para indicar que el proceso ha sido solicitado correctamente
}

// Función para enviar un proceso al servidor
void enviarProceso(int socket_fd, process_t *process) {
    // Enviar los datos del proceso al servidor
    ssize_t total_sent = 0, bytes_sent;
    while (total_sent < sizeof(process_t)) {
        bytes_sent = send(socket_fd, ((char*)process) + total_sent, sizeof(process_t) - total_sent, 0);
        if (bytes_sent < 0) {
            perror("Error al enviar");
            break;
        }
        total_sent += bytes_sent;
    }
}

// Función para recibir la respuesta del servidor
void recibirRespuesta(int socket_fd, process_t *response) {
    ssize_t total_received = 0, bytes_received;
    while (total_received < sizeof(process_t)) {
        bytes_received = recv(socket_fd, ((char*)response) + total_received, sizeof(process_t) - total_received, 0);
        if (bytes_received <= 0) {
            perror("Error al recibir");
            break;
        }
        total_received += bytes_received;
    }
}
