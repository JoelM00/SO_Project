#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include "tarefa.h"
#include "interpretador.h"
#include "config.h"


int main(int argc, char* argv[]) 
{
	int i;

	int fd = open("./myfifo",O_WRONLY);
	char buffer[MAX];
	int n;

	if (argc > 1) {

		if (!strcmp(argv[1],"-i")) {


		}

		if (!strcmp(argv[1],"-m")) {


		}

		if (!strcmp(argv[1],"-e")) {

			send_conf(fd, create_conf(CONFIG_EXEC, 0));

			Tarefa t = createTarefa(argv[2]);

			write(fd, &t, sizeof(Tarefa));

		}

		if (!strcmp(argv[1],"-l")) {

			send_conf(fd, create_conf(CONFIG_LIST, 0));

			return 0;
		}

		if (!strcmp(argv[1],"-t")) {

			send_conf(fd, create_conf(CONFIG_KILL, 0));

			return 0;
		}

		if (!strcmp(argv[1],"-r")) {

			send_conf(fd, create_conf(CONFIG_HIST, 0));

			return 0;
		}

		if (!strcmp(argv[1],"-h")) {

			help();

			return 0;
		}

	} else {
		
		shell(fd);
	}

	close(fd);

	return 0;
}