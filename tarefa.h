#ifndef TAREFA_H
#define TAREFA_H

#define MAX_COMANDOS 10
#define MAX_COMANDO_SIZE 255

typedef struct _TAREFA_ {
	int id; // ID da tarefa

	char comandos[MAX_COMANDOS][MAX_COMANDO_SIZE]; // Comandos a executar
	int ncomandos; // Número de comandos a executar.

	int fds[MAX_COMANDOS - 1][2];
	pid_t pids[MAX_COMANDOS];




} Tarefa;

Tarefa createTarefa(char *);
void showTarefa(Tarefa);

char *** createExecArray(Tarefa);
void showExecArray(char***, int);


#endif


