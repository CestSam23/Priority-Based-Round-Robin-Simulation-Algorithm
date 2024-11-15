#include "mylib.h"
#include "lista.h"

void freeResources(int semid, int shmid, char *shm_ptr) {
    shmdt(shm_ptr);              // Desvincula la memoria compartida
    shmctl(shmid, IPC_RMID, 0);  // Elimina la memoria compartida
}

void AtSharedMemory(key_t key, lista_t *shm_ptr, int shmid){
    shm_ptr = shmat(shmid, 0, 0);  // Asociar memoria compartida
    if (shm_ptr == (char *)-1) {
        perror("Error al asociar memoria compartida");
        exit(1);
    }
}

// Funcion para esccribir los procesos en la memoria compartida mediante su indice
void writeProcess(lista_t *SharedMemory, int index, process_t process) {
    // Verificar que el índice esté dentro de los límites
    if (index < 0 || index >= MAX) {
        printf("Índice fuera de rango.\n");
        return;
    }

    // Escribir los datos del proceso en la memoria compartida
    SharedMemory->procesos[index] = process;
}

// Funcion para leer los procesos en la memoria compartida mediante su indice
process_t readProcess(lista_t *SharedMemory, int index) {
    // Verificar que el índice esté dentro de los límites
    if (index < 0 || index >= MAX) {
        printf("Índice fuera de rango.\n");
        // Devolver un valor por defecto o nulo
        process_t procesoNulo = {0};
        return procesoNulo;
    }

    // Leer el proceso de la memoria compartida
    return SharedMemory->procesos[index];
}

// Función para escribir una lista (lote) e
 void writeList(lista_t *SharedMemory, int prev, int actual, int next, int size) {
    SharedMemory->prev = prev;
    SharedMemory->actual = actual;
    SharedMemory->next = next;
    SharedMemory->size = size;
}

// Función para leer una lista 
void readList(lista_t *SharedMemory) {
    printf("prev: %d, actual: %d, next: %d, size: %d\n", 
    SharedMemory->prev, 
    SharedMemory->actual, 
    SharedMemory->next, 
    SharedMemory->size);
}

int main(){
    key_t key_shm_2 = ftok("/bin/ls", 2); // llave memoria compartida
    int shmid = shmget(key_shm_2, sizeof(struct Process), IPC_CREAT | 0666);  // Crear memoria compartida
    if (shmid == -1) {
        perror("Error al crear memoria compartida");
        exit(1);
    }

    return 0; 
}
