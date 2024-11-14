#include <stdio.h>
#include "lista.h"
int main(){
	lista_t lista;

	process_t trial1 = {1,10,0,0,2,"hoje\0"};
	process_t trial2 = {2,12,0,0,3,"pro2\0"};
	process_t trial3 = {3,19,0,0,4,"pro3\0"};
	process_t trial[3] = {trial1,trial2,trial3};

	addProcesses(trial,3);

	next();
	aumentarEspera(5);
	restarEjecucion(5);
	aumentarTerminacion(5);
	
	printf("Size: %d",size());
	toString();
}
