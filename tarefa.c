#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tarefa.h"

#define MAX 100


// -------------------------------------------- criar o array de comandos de uma tarefa -------------------------------------------- \\

char *** createExecArray (Tarefa t){

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

// ----------------------------------------------------- criar uma nova tarefa ----------------------------------------------------- \\

Tarefa createTarefa (char * line){

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

// ------------------------------------------------ converter um inteiro para char* ------------------------------------------------ \\

char* toString (int n){

	char* buffer = (char*)malloc(MAX * sizeof(char));

	if (n == 0) {
		char c = '0';
		strncat(buffer, &c, 1);
	}
	else {

		char number[5];
		int size = 0;

		for (int x = n; x != 0; x /= 10){
			number[size] = (x % 10) + '0';
			size++;
		}

		for (int i = size - 1; i >= 0; i--){
			strncat(buffer, &(number[i]), 1);
		}
	}

	return buffer;
}