#include <stdio.h>
#include "lista.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void finished(){
	exit(1);
}

int main(){

	//Los procesos guardados por la memoria compartida son guardados en listaDeProcesos
	lista_t listaDeProcesos;

	signal(SIGTERM, finished);

	printf("Size: %d", sizeof(listaDeProcesos));
}
