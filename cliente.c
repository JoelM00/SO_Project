#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include "tarefa.h"

#define MAX 1024

int main(int argc, char* argv[]) 
{
	int i;

	/*if (argc == 0) {
		//Modo Shell
	} else {

		for (i = 1; i < argc) {

			// Executar uma tarefa
			if (!strcmp(argv[i], "-e")) {
				


			}



		}


	}*/

	int fd = open("./myfifo",O_WRONLY);
	char buffer[MAX];
	int n;

	Tarefa t1 = createTarefa(argv[1]);



	/*printf("Pronto para escrever!\n");

	while ((n = read(0,buffer,MAX)) > 0)  {

		write(fd,buffer,n);
	}*/

	write(fd, &t1, sizeof(Tarefa));

	close(fd);

	return 0;
}