#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#include "interpretador.h"
#include "config.h"
#include "tarefa.h"

#define ERROR_MAX 100

int isNumeric(char *s) {

	int i;

	for (i = 0; s[i]; i++)
		if (!isdigit(s[i]))
			return 0;

	return 1;
}

int getParameter(char *token, int *parameter, char errors[][ERROR_MAX]) {

	if (token != NULL) {

		if (!isNumeric(token) || ((*parameter = atoi(token)) <= 0)) {

			write(1, "Erro: Parâmetro inválido.\n", 28);
			write(1, errors[1], strlen(errors[1]) + 6);
			return 0;
		}

	} else {

		write(1, "Erro: Falta o parâmetro de tempo.\n", 35);
		write(1, errors[0], strlen(errors[0]) + 6);
		return 0;
	}

	return 1;
}

int help() {
	write(1,"tempo-inactividade <segs>\n",26);
	write(1,"tempo-execucao <segs>\n",22);
	write(1,"executar 'cmd1 | cmd2 ... cmdn'\n",31);
	write(1,"listar\n",7);
	write(1,"historico\n",10);
	write(1,"terminar\n",9);
}



int shell(int server) {

	char buffer[MAX], *token;
	int n, o;
	Tarefa t;

	char errors[2][ERROR_MAX] = {
		"      > tempo-inactividade <segundos>\n", 
		"      Insira um valor numérico maior que zero (0)\n"
	};

	do {

		write(1, "argus$ ",7);
		n = read(0, buffer, MAX);

		buffer[n-1] = '\0';

		token = strtok(buffer, " ");

		if (!strcmp(token, "tempo-inactividade")) {

			token = strtok(NULL, " ");

			if (getParameter(token, &o, errors)) {

				send_conf(server, create_conf(CONFIG_INAC_TIME, o));

			}
		}

		if (!strcmp(token,"tempo-execucao")) {

			token = strtok(NULL," ");

			if (token!=NULL) {

				int tempo = atoi(token);

				send_conf(server, create_conf(CONFIG_EXEC_TIME, tempo));

			} else {

				write(1,"Comando invalido!",18);
			}
		}


		if (!strcmp(token, "executar")) {

			token = strtok(NULL, "'");

			if (token) {

				send_conf(server, create_conf(CONFIG_EXEC, 0));

				Tarefa t = createTarefa(token);
				write(server, &t, sizeof(Tarefa));

			} else {

				write(1,"Comando invalido!",18);

			}
		}

		if (!strcmp(token, "listar")) {

			send_conf(server, create_conf(CONFIG_LIST, 0));

		}

		if (!strcmp(token,"historico")) {

			send_conf(server, create_conf(CONFIG_HIST, 0));

		}

		if (!strcmp(token,"terminar")) {

			token = strtok(NULL," ");

			if (token!=NULL) {

				int id = atoi(token);

				send_conf(server,create_conf(CONFIG_KILL,0));

			} else {

				write(1,"Comando invalido!",18);

			}
		}

		if (!strcmp(token,"ajuda")) {

			help();
		}

	} while (strcmp(buffer, "exit"));

	return 0;
}
