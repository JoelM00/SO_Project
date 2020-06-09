#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

#include "servidor.h"
#include "tarefa.h"
#include "config.h"

static int tempo_inacticidade = 10;
static int tempo_execucao = 2;


static Tarefa tarefas[MAX_TAREFAS];
static int ntarefas = 0;


// Handler do tempo de execução

void timeout_handler (int signum) {

	int i, j;

	//showTarefas();


	for (i = 0; i < ntarefas; i++) {

		if ( tarefas[i].estado != TERMINATED && tarefas[i].ttl <= 0) {

			for(j = 0; j < tarefas[i].ncomandos; j++) {

				if (tarefas[i].pids[j] > 0) {

					printf("A matar %d\n", tarefas[i].pids[j]);
					kill(tarefas[i].pids[j], SIGKILL);
				}
			}

			tarefas[i].estado = TERMINATED;
			printf("TERMINOU TAREFA %d %d\n", i, tarefas[i].estado);

		} else {

			tarefas[i].ttl--;
		}
	}


	alarm(1);


}

void sigchld_handler(int signum) {

	int i, j;
	pid_t   pid;
 	int     stat;

 	pid = wait(&stat);
 	printf("child %d terminated %d %d\n", pid, WIFEXITED(stat),WEXITSTATUS(stat));


 	for (i = 0; i < ntarefas; i++) {

 		for( j = 0; j < tarefas[i].ncomandos; j++) {

 			if (pid == tarefas[i].pids[j])
 				tarefas[i].terminated_pids++;
 		}

 		if (tarefas[i].terminated_pids >= tarefas[i].ncomandos)
 			tarefas[i].estado = TERMINATED;

 	}

    
    return;
}


int main() {

	Tarefa t;
	Config conf;
	int n, i, j;

	mkfifo("./myfifo", 0600);

	int fd = open("./myfifo",O_RDONLY);


	signal(SIGCHLD, sigchld_handler);
	signal(SIGALRM, timeout_handler);
	alarm(1);

	while(1) {

		while ((n = read(fd, &conf,sizeof(Config))) > 0) {

			//printf(">%d %d\n", conf.cmd, conf.option);

			if (conf.cmd == CONFIG_EXEC) {

				if ((n = read(fd, &t,sizeof(Tarefa))) > 0) {

					t.id = ntarefas;
					printf("WAIT\n");
					t.estado = WAITING;
					t.ttl = tempo_execucao;
					tarefas[ntarefas++] = t;

				
					executarTarefa(&tarefas[ntarefas-1]);
					

				} else {

					write(1,"Tarefa nao recebida",20);
				}

			}

			if (conf.cmd == CONFIG_INAC_TIME) {

				tempo_inacticidade = conf.option;



			}

			if (conf.cmd == CONFIG_EXEC_TIME) {

				tempo_execucao = conf.option;

			}

			if (conf.cmd == CONFIG_LIST) {

				showTarefasEmExecucao();

				//showTarefas();
			
			}


			if (conf.cmd == CONFIG_KILL) {

				for (i = 0; i < ntarefas; i++) {

					if (tarefas[i].id == conf.option) {

						for (j = 0; j < tarefas[i].ncomandos; j++) {

							if (tarefas[i].pids[j]>-1) {
								kill(tarefas[i].pids[j], SIGKILL);
							}
						}

						tarefas[i].estado = TERMINATED;
					}
				} 
			}

			if (conf.cmd == CONFIG_HIST) {

				showTarefasTerminadas();
			
			}
		}
	}

	close(fd);
}


void showTarefas()
{
	int i;


	printf("\n--------------------------------------\n");
	printf("Lista de tarefas:\n\n");

	for (i = 0; i < ntarefas && i < MAX_TAREFAS; i++) {

		showTarefa(tarefas[i]);

	}

	printf("\n--------------------------------------\n");

}


void showTarefasTerminadas()
{
	int i;


	printf("\n--------------------------------------\n");
	printf("Lista de tarefas (terminadas):\n\n");

	for (i = 0; i < ntarefas && i < MAX_TAREFAS && tarefas[i].estado==TERMINATED; i++) {

		//if (tarefas[i].estado == RUNNING)
			showTarefa(tarefas[i]);
	}

	printf("\n--------------------------------------\n");

}



void showTarefasEmExecucao()
{
	int i;


	printf("\n--------------------------------------\n");
	printf("Lista de tarefas (em execução):\n\n");

	for (i = 0; i < ntarefas && i < MAX_TAREFAS && tarefas[i].estado==1; i++) {

		//if (tarefas[i].estado == RUNNING)
			showTarefa(tarefas[i]);
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

	printf("RUN\n");
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

			sleep(50);

    		execvp(argv[i][0], argv[i]); // Executar o comando.
		}
	}

	// Processo pai.
	// Fechar os pipes.
	for (int i = 0; i < t->ncomandos - 1; i++) {
		close(t->fds[i][0]);
		close(t->fds[i][1]);
	}

	//printf("TERM\n");
	//t->estado = TERMINATED;
}