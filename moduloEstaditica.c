#include "mylib.h"
#define MAX_LINE 1024
//corto-largo tamaño lista (tamaño memoria compartida)
// 2 regiones de memoria compartida

int addition = 0, countProcessAnnihilated = 0, countProcessExecuted = 0;
lista_t *SharedMemory;

void signal_handler(int sig) {
    printFile();
    readFile();
    exit(0);
}

// Crea el archivo del reporte y escribe en el archivo
void printFile() {
    FILE *fp_write = fopen("report.txt", "w");
    if (fp_write == NULL) {
        perror("can't open file for writing");
        exit(1);
    }
    fprintf(fp_write, "Número de procesos ejecutados: %d\n", countProcessExecuted);
    fprintf(fp_write, "Número de procesos aniquilados: %d\n", countProcessAnnihilated);
    if (countProcessAnnihilated > 0) {
        fprintf(fp_write, "Tiempo promedio de espera: %.2f\n", (float)addition/countProcessAnnihilated);
    } else {
        fprintf(fp_write, "Tiempo promedio de espera: %.2f\n", 0.0);
    }
    fclose(fp_write);
}

// Abrir archivo para leer e imprimir en terminal 
void readFile(){
    FILE *fp_read = fopen("report.txt", "r");
    if (fp_read == NULL) {
        perror("can't open file for reading");
        exit(1);
    }
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp_read)) {
        printf("%s", line);
    }
    fclose(fp_read);
}

// Calcula las estadisticas de los procesos
void getProcessStatistic(int cpuBurst, int tCompletition,int tWaiting){   
// Completition - CpuBurst
    if(tCompletition==0){
            countProcessAnnihilated++;
    }
    if(tCompletition>0){
            countProcessExecuted++;
    }

    tWaiting=tCompletition-cpuBurst;
    addition+=tWaiting;
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


// Función para leer una lista 
void readList(lista_t *SharedMemory) {
    printf("prev: %d, actual: %d, next: %d, size: %d\n", 
    //SharedMemory->prev, 
    SharedMemory->actual, 
    //SharedMemory->next, 
    SharedMemory->size);
}

int main(){
    signal(SIGTERM,signal_handler);
    for(;;){
        for(int i=0; i<SharedMemory->size;i++){
            process_t process = readProcess(SharedMemory, i);
            getProcessStatistic(process.cpuBurst, process.tCompletition, process.tWaiting);
        }
    }
    //freeResources();
    return 0;
}