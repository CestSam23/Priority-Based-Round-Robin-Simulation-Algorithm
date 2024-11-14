// ejemplo de ROUND ROBIN CON PRIORIDADES
    // A : T.E = 5
    // B : T.E = 12
    // C : T.E = 10
    // QUANTUM = 3

    /*
    return suma+=twaiting;
    // A : Tiempo espera = 6
    // B : Tiempo espera = 15
    // C : Tiempo espera = 14

    // 6 + 15 + 14 = 35/3 = 11.6 tiempo promedio de espera 
    
    A ***------** |   (2)  tiempo de terminacion = 11
    B ---***-----***------***-*** | (9) - (6) - (3) tiempo de terminacion = 27
    C ------***-----******---*| (7) - (4) - (1) tiempo de terminacion = 24
    
    */
   /*
    ARCHIVO TEXTO
    COMPORTAMIENTO DEL DESPACHADOR (CORTO)
    // PENDIENTE : CUANTOS PROCESOS FUERON EJECUTADOS
        
    CUANTOS PROCESOS FUERON ANIQUILADOS (TIEMPO = 0)
    CUAL FUE EL TIEMPO PROMEDIO DE ESPERA

    MEMORIA COMPARTIDA Y SEMAFOROS 

    DUDA: Si el módulo de estadística se encarga de las estadísticas de los procesos terminados
            por el despachador eso significa en dado caso que su tiempo de ejecución siempre será igual a 0
            ya que "procesos terminados por el despachador".
             
*/

#include "mylib.h"
#define MAX 1024

struct Process{
	pid_t id; // identifidor del proceso 
	int cpuBurst; // tiempo de ejecucion
	int tCompletition; // tiempo de terminacion 
	int tWaiting; //tiempo espera
	int priority; 
	char name[5];
} process;

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
    if(cpuBurst==0){
            countProcessAnnihilated++;
    }
    if(cpuBurst>0){
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