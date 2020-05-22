#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "servidor.h"
#include "tarefa.h"


static Tarefa tarefas[MAX_TAREFAS];
static int ntarefas = 0;

int main() {

	Tarefa t;
	int n;

	mkfifo("./myfifo", 0600);

	int fd = open("./myfifo",O_RDONLY);

	//int fdOut = open("logs.txt",O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);

	while(1) {

		while ((n = read(fd, &t,sizeof(Tarefa))) > 0) {

			t.id = ntarefas;
			tarefas[ntarefas++] = t;

			executarTarefa(t);

			showTarefas();
		}



	}
	

	//printf("EOF -> recebido!\n");

	//write(fdOut,"\n#########\n",13);

	//close(fdOut);
	close(fd);


}


void showTarefas()
{
	int i;


	printf("\n-----------------------------\n");
	printf("Lista de tarefas:\n\n");

	for (i = 0; i < ntarefas && i < MAX_TAREFAS; i++) {

		showTarefa(tarefas[i]);
	}

	printf("\n-----------------------------\n");

}


void executarTarefa(Tarefa t)
{
	int i, j;
	pid_t filho;
	char ***argv = createExecArray(t);

	// Iniciar os pipes necessários.
	for(i = 0; i < t.ncomandos - 1; i++)
		pipe(t.fds[i]);


	// Criar um processo filho para cada comando a executar.
	for(i = 0; i < t.ncomandos; i++) {

		if ((t.pids[i] = fork()) == 0) {

			// Configurar os inputs/outputs de cada processo
			if (i == 0) {
				// Primeiro processo.
				dup2(t.fds[i][1], 1);
			} else {
				if (i == t.ncomandos - 1) {
					//  Último processo.
					dup2(t.fds[i-1][0], 0);
				} else {
					// Processos intermédios.
					dup2(t.fds[i-1][0], 0);  
					dup2(t.fds[i][1], 1);
				}
			}

			// Fechar em cada processo os pipes.
			for(j = 0; j < t.ncomandos - 1; j++) {

				close(t.fds[j][0]);
				close(t.fds[j][1]);
			}
			
			// Executar o comando.
			execvp(argv[i][0], argv[i]);
		} 
	}

	// Processo pai.


	// Fechar os pipes.
	for(j = 0; j < t.ncomandos - 1; j++) {

		close(t.fds[j][0]);
		close(t.fds[j][1]);
	}

}

