#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tarefa.h"

#define MAX 100

char *** createExecArray(Tarefa t)
{
	int i, j;
	char ***args, *token, ncomandos = 0;

	args = (char***) malloc(sizeof(char**) * t.ncomandos);

	for (i = 0; i < t.ncomandos; i++) {
		args[i] = (char**) malloc(sizeof(char*) * MAX);

		for (j = 0; j < MAX; j++) {

			args[i][j] = (char*) malloc(sizeof(char) * MAX);
		}
	}

	for (i = 0; i < t.ncomandos; i++) {

			token = strtok(t.comandos[i], " ");
			j = 0;

			while (token) {

				args[i][j++] = token;
				token = strtok(NULL, " ");
			}

			args[i][j] = NULL;
	}

	return args;
}

Tarefa createTarefa(char * line)
{
	Tarefa nova;
	char *token = strtok(line,"|");

	nova.id = -1;
	nova.ncomandos = 0;
	nova.terminated_pids = 0;

	while (token) {

		strcpy(nova.comandos[nova.ncomandos++], token);
		token = strtok(NULL, "|");
	}

	return nova;
}


void showTarefa(Tarefa t) {

	//write(1, "#", 2);
	int x = t.id;
	printf("%s\n", "#");
	printf("%d\n", x);
	//write(1, &x, sizeof(x));
	//write(1, "\n", 2);

	char* buffer = (char*)malloc(MAX * sizeof(char));
	
	if (t.estado == WAITING) strcat(buffer, " em espera: ");
	else if (t.estado == RUNNING) strcat(buffer, " em execução: ");
	else if (t.estado == TERMINATED) strcat(buffer, " concluída: ");
	else if (t.estado == MAX_INATIVIDADE) strcat(buffer, " max inactividade: ");
	else if (t.estado == MAX_EXECUCAO) strcat(buffer, " max execução: ");

	for (int i = 0; i < t.ncomandos - 1; i++) {
		strcat(buffer, t.comandos[i]);
		strcat(buffer, "|");
	}
	strcat(buffer, t.comandos[t.ncomandos-1]);
	strcat(buffer, "\n");
	//write(1, &buffer, strlen(buffer) + 1);
	printf("%s\n", buffer);

	free(buffer);
}


void showExecArray(char *** array, int n) {
	int i, j;

	for(i = 0; i < n; i++) {

		for(j = 0; array[i][j]; j++) {
			printf("%s ", array[i][j]);
		}
	}
}

