#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "interpretador.h"
#include "config.h"
#include "tarefa.h"

#define ERROR_MAX 100
#define MAX_LINE_SIZE 1024

#define OUTPUT_FD "output"


// ---------------------------------------------- verificar se uma string é um número ---------------------------------------------- \\

int isNumeric(char *s) {

	for (int i = 0; s[i]; i++){
		if (!isdigit(s[i])) return 0;
	}

	return 1;
}

// -------------------------------------------------------- validar o input -------------------------------------------------------- \\

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

// -------------------------------------------------- listar os comandos possíveis ------------------------------------------------- \\

int help() {
	write(1,"tempo-inactividade <segs>\n",26);
	write(1,"tempo-execucao <segs>\n",22);
	write(1,"executar 'cmd1 | cmd2 ... cmdn'\n",31);
	write(1,"listar\n",7);
	write(1,"historico\n",10);
	write(1,"terminar\n",9);
	return 0;
}

// ---------------------------------------------------------- modo shell ----------------------------------------------------------- \\

int shell(int server) {

	char buffer[MAX], *token;
	int n, o;
	Tarefa t;

	char errors[2][ERROR_MAX] = {
		"      > tempo-inactividade <segundos>\n", 
		"      Insira um valor numérico maior que zero (0)\n"
	};

	int fd_output;
	int readOutput = 0; // apenas ler o output quando for inserido input
	int bytesRead;
    char outputBuffer[MAX_LINE_SIZE];

	if ((fd_output = open(OUTPUT_FD, O_RDONLY | O_CREAT)) == -1){
        perror("error opening output file");
		return -1;
	}

	while (strcmp(buffer, "exit")) {

		// Ler input

		write(1, "argus$ ",7);
		n = read(0, buffer, MAX);

		buffer[n-1] = '\0';

		token = strtok(buffer, " ");

		// tempo de inactividade
		if (!strcmp(token, "tempo-inactividade")) {

			token = strtok(NULL, " ");

			if (getParameter(token, &o, errors)) send_conf(server, create_conf(CONFIG_INAC_TIME, o));

			readOutput = 1;
		}

		// tempo de execução
		else if (!strcmp(token,"tempo-execucao")) {

			token = strtok(NULL," ");

			if (token != NULL) {
				int tempo = atoi(token);
				send_conf(server, create_conf(CONFIG_EXEC_TIME, tempo));
			} 
			else write(1,"Comando inválido!",18);

			readOutput = 1;
		}

		// executar
		else if (!strcmp(token, "executar")) {

			token = strtok(NULL, "'");

			if (token) {
				send_conf(server, create_conf(CONFIG_EXEC, 0));
				Tarefa t = createTarefa(token);
				write(server, &t, sizeof(Tarefa));
			} 
			else write(1,"Comando invalido!",18);

			readOutput = 1;
		}

		// listar
		else if (!strcmp(token, "listar")) {

			send_conf(server, create_conf(CONFIG_LIST, 0));

			readOutput = 1;
		}

		// histórico
		else if (!strcmp(token,"historico")) {

			send_conf(server, create_conf(CONFIG_HIST, 0));

			readOutput = 1;
		}

		// terminar
		else if (!strcmp(token,"terminar")) {

			token = strtok(NULL," ");

			if (token!=NULL) {
				int id = atoi(token);
				send_conf(server,create_conf(CONFIG_KILL,id));
			} 
			else write(1,"Comando invalido!",18);

			readOutput = 1;
		}

		// ajuda
		else if (!strcmp(token,"ajuda")) {

			help();
		}

		// Ler output

		while (readOutput){

			sleep(1);

			while ((bytesRead = read(fd_output, &outputBuffer, MAX_LINE_SIZE)) > 0){
        		write(1, &outputBuffer, bytesRead);
        	}

			readOutput = 0;
		}	
	}

	close(fd_output);

	return 0;
}