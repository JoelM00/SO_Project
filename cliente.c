#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include "tarefa.h"
#include "interpretador.h"

//#define MAX 1024

int main(int argc, char* argv[]) 
{
	int i;

	int fd = open("./myfifo",O_WRONLY);
	char buffer[MAX];
	int n;


	/*Tarefa t1 = createTarefa(argv[1]);

	write(fd, &t1, sizeof(Tarefa));*/

	shell(fd);

	close(fd);

	return 0;
}