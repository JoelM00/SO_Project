#ifndef TAREFA_H
#define TAREFA_H

#define MAX_COMANDOS 10
#define MAX_COMANDO_SIZE 255

typedef enum { WAITING, RUNNING, TERMINATED, MAX_INATIVIDADE, MAX_EXECUCAO } Estado;

typedef struct _TAREFA_ {
	
	int id; // ID da tarefa

	char comandos[MAX_COMANDOS][MAX_COMANDO_SIZE]; // Comandos a executar
	int ncomandos; // Número de comandos a executar.

	int fds[MAX_COMANDOS - 1][2];
	pid_t pids[MAX_COMANDOS];
	int terminated_pids;

	int ttl;

	Estado estado;

} Tarefa;

Tarefa createTarefa(char *);
void showTarefa(Tarefa, char*);

char *** createExecArray(Tarefa);
void showExecArray(char***, int);

char* toString (int);

#endif