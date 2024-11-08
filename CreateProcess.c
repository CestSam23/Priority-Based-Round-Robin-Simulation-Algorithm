#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

struct Process{
	pid_t id;
	int cpuBurst;
	char name[5];
};

struct ProcessParameter{
	int numberOfProcesses;
	int minimumBurst;
	int maximumBurst;
};

int menu();
void createFile(struct ProcessParameter parameters);
void readFile();

int main(){
	int option;
	struct ProcessParameter parameters;

	while((option=menu())!=3){
		switch(option){
			case 1:
				printf("********Create File of Processes***********\n");
				printf("Number of processes: ");
				scanf("%d",&parameters.numberOfProcesses);
				printf("Minimum Burst Time: ");
				scanf("%d",&parameters.minimumBurst);
				printf("Maximum Burst Time: ");
				scanf("%d",&parameters.maximumBurst);
				createFile(parameters);
				break;
			case 2:
				printf("******Read File of Processes****************\n");
				readFile();	
				break;
			default:
				break;
		}
	}

}

void createFile(struct ProcessParameter parameters){
	FILE *fPtr;
	fPtr = fopen("processRequest.dat","wb");
	srand(time(NULL));
	
	if(fPtr==NULL){
		printf("File Couldn't be created\n");
	} else {
		//File created. We can start writing.
		struct Process processData;
		for(int i=0;i<parameters.numberOfProcesses;i++){
			processData.id = i+1;
			processData.cpuBurst = (rand()%parameters.maximumBurst) + parameters.minimumBurst;
			for(int j=0;j<5;j++){
				processData.name[j] = (char)(rand()%26) + 'a';
			} 
			fwrite(&processData,sizeof(struct Process),1,fPtr);
		}

		fclose(fPtr);
		printf("File Created!\n");
	}
}

void readFile(){
	FILE *fPtr;
	fPtr = fopen("processRequest.dat","rb");
	if(fPtr==NULL){
		printf("File Couldn't be opened\n");
	} else {
		printf("%-6s%-16s%-16s\n","Id","CPU Burst","Name");

		//Read all records from file (Until EOF)
		while(!feof(fPtr)){
			struct Process readedProcess = {0,0,""};
			if((fread(&readedProcess,sizeof(struct Process),1,fPtr))!=0){
				printf("%-6d%-16d%-16s\n",readedProcess.id,readedProcess.cpuBurst,readedProcess.name);
			}
		}
		fclose(fPtr);
	}
}

int menu(){
	printf("\t\tCreation of Processes\n");
	printf("1.Create File of Processes\n");
	printf("2.Read File of Processes\n");
	printf("3.Exit\n");
	printf(".-");
	int option;
	scanf("%d",&option);
	return option;
}

