#ifndef TAREFA_H
#define TAREFA_H

#define MAX_COMANDOS 10
#define MAX_COMANDO_SIZE 255

typedef enum {WAITING, RUNNING, TERMINATED} Estado;

typedef struct _TAREFA_ {
	int id;                                        // ID da tarefa

	char comandos[MAX_COMANDOS][MAX_COMANDO_SIZE]; // Comandos a executar
	int ncomandos;                                 // Número de comandos a executar.

	int fds[MAX_COMANDOS - 1][2];                   // Número de comandos a executar.
	pid_t pids[MAX_COMANDOS];                       // Número de comandos a executar.
	int terminated_pids;                            // Número de comandos a executar.

	int ttl;

	Estado estado;

} Tarefa;

Tarefa createTarefa(char *);
void showTarefa(Tarefa);

char *** createExecArray(Tarefa);
void showExecArray(char***, int);


#endif


