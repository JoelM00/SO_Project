#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tarefa.h"

#define MAX 100

char *** createExecArray(Tarefa t)
{
	int i, j;
	char ***args, *token, ncomandos = 0;


	// ls -la | wc
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

	while (token) {

		strcpy(nova.comandos[nova.ncomandos++], token);
		token = strtok(NULL, "|");
	}

	return nova;
}


void showTarefa(Tarefa t) 
{
	int i, j;

	printf("[%d] S=%d ", t.id, t.estado);

	for (i = 0; i < t.ncomandos; i++) {

		printf("%s [%d]", t.comandos[i], t.pids[i]);
		
		if (i < t.ncomandos - 1) printf(">");
	}

	printf("\n");
}


void showExecArray(char *** array, int n)
{
	int i, j;

	for(i = 0; i < n; i++) {

		for(j = 0; array[i][j]; j++) {
			printf("%s ", array[i][j]);
		}
	}
}

