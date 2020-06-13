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


// -------------------------------------------------------------- main ------------------------------------------------------------- \\

int main(int argc, char* argv[]) {

	int i;

	int fd = open("./myfifo", O_WRONLY);
	char buffer[MAX];
	int n;

	if (argc > 1) {

		if (strcmp(argv[1], "-i") == 0) {
			int option = atoi(argv[2]);
			if (option > 0) send_conf(fd, create_conf(CONFIG_INAC_TIME, option));
			else write(1, "valor inválido!\n", 17);
		}

		else if (strcmp(argv[1], "-m") == 0) {
			int option = atoi(argv[2]);
			if (option > 0) send_conf(fd, create_conf(CONFIG_EXEC_TIME, option));
			else write(1, "valor inválido!\n", 17);
		}

		else if (strcmp(argv[1], "-e") == 0) {
			send_conf(fd, create_conf(CONFIG_EXEC, 0));
			Tarefa t = createTarefa(argv[2]);
			write(fd, &t, sizeof(Tarefa));
		}

		else if (strcmp(argv[1], "-l") == 0) {
			send_conf(fd, create_conf(CONFIG_LIST, 0));
			return 0;
		}

		else if (strcmp(argv[1], "-t") == 0) {
			int id = atoi(argv[2]);
			send_conf(fd, create_conf(CONFIG_KILL, id));
			return 0;
		}

		else if (strcmp(argv[1], "-r") == 0) {
			send_conf(fd, create_conf(CONFIG_HIST, 0));
			return 0;
		}

		else if (strcmp(argv[1], "-h") == 0) {
			help();
			return 0;
		}

	} 
	else shell(fd);

	close(fd);

	return 0;
}