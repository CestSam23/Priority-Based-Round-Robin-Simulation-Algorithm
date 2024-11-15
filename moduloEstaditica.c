#include "mylib.h"
#define MAX 1024
//corto-largo tamaño lista (tamaño memoria compartida)
// 2 regiones de memoria compartida

int addition = 0;
int countProcessAnnihilated = 0;
int countProcessExecuted = 0;

void signal_handler(int sig){
   printFile();
   readFile();
   exit(0);
}

// Crea el archivo del reporte y escribe en el archivo
void printFile(){
    FILE *fp_write = fopen("report.txt", "w");
    if(fp_write == NULL){
        perror("can't open file for writing");
        exit(1);
    }
    fprintf(fp_write, "Número de procesos ejecutados: %d\n", countProcessExecuted);
    fprintf(fp_write, "Número de procesos aniquilados: %d\n", countProcessAnnihilated);
    fprintf(fp_write, "Tiempo promedio de espera: %.2f\n", addition/countProcessExecuted);
}

// Abrir archivo para leer e imprimir en terminal 
void readFile(){
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

// Calcula las estadisticas de los procesos
void getProcess(int cpuBurst, int tCompletition,int tWaiting){   
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

int main(){

    signal(SIGTERM,signal_handler);
    for(;;){
        getProcess(process.cpuBurst, process.tCompletition, process.tWaiting);
    }
    return 0; 
}



/* del codigo DespachadorCortoPlazo necesitamos pasarnos por medio de memoria comparida 
    */