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


// Handler do tempo de execução

void timeout_handler (int signum) {

	int i, j;

	for (i = 0; i < ntarefas; i++) {

		if ( tarefas[i].estado != TERMINATED && tarefas[i].ttl <= 0) {

			for (j = 0; j < tarefas[i].ncomandos; j++) {

				if (tarefas[i].pids[j] > 0) {

					printf("A matar %d\n", tarefas[i].pids[j]);
					kill(tarefas[i].pids[j], SIGKILL);
				}
			}

			tarefas[i].estado = MAX_EXECUCAO;
			printf("TERMINOU TAREFA %d %d\n", i, tarefas[i].estado);

		} 
		else tarefas[i].ttl--;
	}

	alarm(1);
}


void sigchld_handler (int signum) {

	int i, j;
	pid_t pid;
 	int stat;

 	pid = wait(&stat);

 	if (WIFEXITED(stat)) printf("Filho %d terminou normalmente! %d\n", pid,WEXITSTATUS(stat));
	else printf("Filho %d foi morto! %d\n", pid,WEXITSTATUS(stat));

 	for (i = 0; i < ntarefas; i++) {

 		for (j = 0; j < tarefas[i].ncomandos; j++) {

 			if (pid == tarefas[i].pids[j]) tarefas[i].terminated_pids++;
 		}

 		if (tarefas[i].terminated_pids >= tarefas[i].ncomandos) tarefas[i].estado = TERMINATED;
 	}

	return;
}


int main() {

	Tarefa t;
	Config conf;
	int n, i, j;

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

					if (ntarefas < MAX_TAREFAS){

						t.id = idTarefa;
						idTarefa++;
						t.estado = WAITING;
						t.ttl = tempo_execucao;
						tarefas[ntarefas++] = t;

						executarTarefa(&tarefas[ntarefas-1]);

						if (t.estado == RUNNING) t.estado = TERMINATED;
						write(fd_terminadas, &t, sizeof(Tarefa));
					}
					else write(1, "Limite de tarefas atingido\n", 28);
				} 
				else write(1,"Tarefa não recebida",20);
			}

			// definir o tempo máximo (segundos) de inactividade de comunicação num pipe anónimo

			if (conf.cmd == CONFIG_INAC_TIME) {
				tempo_inactividade = conf.option;
				write(fd_output, "tempo de inactividade atualizado.\n", 35);
			}

			// definir o tempo máximo (segundos) de execução de uma tarefa

			if (conf.cmd == CONFIG_EXEC_TIME) {
				tempo_execucao = conf.option;
				write(fd_output, "tempo de execução atualizado.\n", 32);
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


void showTarefas () {

	printf("\n--------------------------------------\n");
	printf("Lista de tarefas:\n\n");

	for (int i = 0; i < ntarefas && i < MAX_TAREFAS; i++) {
		showTarefa(tarefas[i]);
	}

	printf("\n--------------------------------------\n");
}


int showTarefasTerminadas() {

	printf("\n--------------------------------------\n");
	printf("Lista de tarefas (terminadas):\n\n");

	int fd = open(TERMINADAS_FD, O_RDONLY, 0600);
    
	if (fd < 0){
        write(1, "error opening file\n", 20);
        return -1;
    }

	Tarefa t;
	int bytes_read;

	while ((bytes_read = read(fd, &t, sizeof(Tarefa))) > 0){
		showTarefa(t);
	}

	close(fd);

	printf("\n--------------------------------------\n");

	return 0;
}



void showTarefasEmExecucao() {

	printf("\n--------------------------------------\n");
	printf("Lista de tarefas (em execução):\n\n");

	for (int i = 0; i < ntarefas && i < MAX_TAREFAS; i++) {

		if (tarefas[i].estado == RUNNING) showTarefa(tarefas[i]);
	}

	printf("\n--------------------------------------\n");
}



// Executar uma tarefa
void executarTarefa (Tarefa *t) {

	printf("pid pai = %d\n", getpid());

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