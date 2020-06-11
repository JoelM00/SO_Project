#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "servidor.h"
#include "tarefa.h"
#include "config.h"

#define FIFO_FD "myfifo"
#define TERMINADAS_FD "terminadas"
#define OUTPUT_FD "output"

static int tempo_inactividade = 10;
static int tempo_execucao = 10;

static int idTarefa = 0;

static Tarefa tarefas[MAX_TAREFAS];
static int ntarefas = 0;



// Remover uma tarefa do array
void removeTerminada (int index) {

	for (int i = index; i < ntarefas; i++){
        tarefas[i] = tarefas[i+1];
	}
}


// Handler do tempo de execução
void timeout_handler (int signum) {

	int i, j;

	for (i = 0; i < ntarefas; i++) {

		if (tarefas[i].estado != TERMINATED && tarefas[i].ttl <= 0) {

			for (j = 0; j < tarefas[i].ncomandos; j++) {

				if (tarefas[i].pids[j] > 0) {
					//printf("A matar %d\n", tarefas[i].pids[j]);
					kill(tarefas[i].pids[j], SIGKILL);
				}
			}

			tarefas[i].estado = MAX_EXECUCAO;
			//printf("TERMINOU TAREFA %d %d\n", i, tarefas[i].estado);

		} 
		else tarefas[i].ttl--;
	}

	alarm(1);
}


// Handler das childs
void sigchld_handler(int signum) {

	int i, j;
	pid_t pid;
 	int stat;
	int fd_terminadas = open(TERMINADAS_FD, O_CREAT | O_APPEND | O_WRONLY, 0600);

 	pid = wait(&stat);
 	/*if (WIFEXITED(stat)) printf("Filho %d terminou normalmente! %d\n", pid,WEXITSTATUS(stat));
	else printf("Filho %d foi morto! %d\n", pid,WEXITSTATUS(stat));*/

 	for (i = 0; i < ntarefas; i++) {

 		for (j = 0; j < tarefas[i].ncomandos; j++) {

 			if (pid == tarefas[i].pids[j]) tarefas[i].terminated_pids++;
 		}

 		if (tarefas[i].terminated_pids >= tarefas[i].ncomandos){
 			if (tarefas[i].estado == RUNNING) tarefas[i].estado = TERMINATED;
			write(fd_terminadas, &(tarefas[i]), sizeof(Tarefa));
			removeTerminada(i);
			ntarefas--;
		}
 	}
    
    return;
}


// main
int main() {

	Tarefa t;
	Config conf;
	int n, i, j;

	mkfifo("./myfifo", 0600);

	mkfifo(FIFO_FD, 0600);
	int fd = open(FIFO_FD, O_RDONLY);
	int fd_terminadas = open(TERMINADAS_FD, O_CREAT | O_APPEND | O_WRONLY, 0600);
	int fd_output = open(OUTPUT_FD, O_CREAT | O_APPEND | O_WRONLY, 0600);

	signal(SIGCHLD, sigchld_handler);
	signal(SIGALRM, timeout_handler);
	alarm(1);

	while(1) {

		while ((n = read(fd, &conf,sizeof(Config))) > 0) {

			// executar uma tarefa

			if (conf.cmd == CONFIG_EXEC) {

				if ((n = read(fd, &t,sizeof(Tarefa))) > 0) {

					t.id = idTarefa;
					char* buffer = toString(idTarefa);
					write(fd_output, "nova Tarefa #", 14);
					write(fd_output, buffer, sizeof(idTarefa));
					free(buffer);
					idTarefa++;

					t.estado = WAITING;
					t.ttl = tempo_execucao;
					tarefas[ntarefas++] = t;

					executarTarefa(&tarefas[ntarefas-1]);
				} 
				else write(1, "Tarefa nao recebida", 20);
			}

			// definir o tempo máximo (segundos) de inactividade de comunicação num pipe anónimo

			if (conf.cmd == CONFIG_INAC_TIME) {
				tempo_inactividade = conf.option;
				write(fd_output, "tempo de inactividade atualizado", 33);
			}

			// definir o tempo máximo (segundos) de execução de uma tarefa

			if (conf.cmd == CONFIG_EXEC_TIME) {
				tempo_execucao = conf.option;
				write(fd_output, "tempo de execução atualizado", 30);
			}

			// listar tarefas em execução

			if (conf.cmd == CONFIG_LIST) {
				showTarefasEmExecucao();
			}

			// terminar uma tarefa em execução

			if (conf.cmd == CONFIG_KILL) {

				for (i = 0; i < ntarefas; i++) {

					if (tarefas[i].id == conf.option) {

						for (j = 0; j < tarefas[i].ncomandos; j++) {

							if (tarefas[i].pids[j]>-1) kill(tarefas[i].pids[j], SIGKILL);
						}

						tarefas[i].estado = TERMINATED;
					}
				} 
			}

			// listar registo histórico de tarefas terminadas

			if (conf.cmd == CONFIG_HIST) {
				showTarefasTerminadas();
			}
		}
	}

	close(fd);
	close(fd_terminadas);
	close(fd_output);
}


// Listar as tarefas em execução e em espera
void showTarefasEmExecucao () {

	for (int i = 0; i < ntarefas && i < MAX_TAREFAS; i++) {
		showTarefa(tarefas[i]);
	}
}


// Listar as tarefas terminadas
void showTarefasTerminadas() {

	int fd = open(TERMINADAS_FD, O_RDONLY, 0600);
    
	if (fd < 0){
        write(1, "error opening file\n", 20);
    }
	else {

		Tarefa t;
		int bytes_read;

		while ((bytes_read = read(fd, &t, sizeof(Tarefa))) > 0){
			showTarefa(t);
		}
	}

	close(fd);
}


// Executar uma tarefa
void executarTarefa (Tarefa *t) {

	//printf("pid pai = %d\n", getpid());

	char ***argv = createExecArray(*t);

	if (signal(SIGALRM, timeout_handler) < 0){
        perror("signal SIGALRM");
        exit(-1);
    }

	t->estado = RUNNING;

	// Iniciar os pipes necessários.
	for (int i = 0; i < t->ncomandos - 1; i++){
		pipe(t->fds[i]);
	}

	
	for (int i = 0; i < t->ncomandos; i++) {

		if ((t->pids[i] = fork()) == 0){

			// Configurar os inputs/outputs de cada processo

			if (i == 0) dup2(t->fds[i][1], 1); // Primeiro processo.
			
			else if (i == t->ncomandos - 1) dup2(t->fds[i-1][0], 0); //  Último processo.

			else {
				// Processos intermédios.
				dup2(t->fds[i-1][0], 0);  
				dup2(t->fds[i][1], 1);
			}

			// Fechar em cada processo os pipes.
			for (int j = 0; j < t->ncomandos - 1; j++) {
				close(t->fds[j][0]);
				close(t->fds[j][1]);
			}

    		execvp(argv[i][0], argv[i]); // Executar o comando.
		}
	}

	// Processo pai.
	// Fechar os pipes.
	for (int i = 0; i < t->ncomandos - 1; i++) {
		close(t->fds[i][0]);
		close(t->fds[i][1]);
	}
}