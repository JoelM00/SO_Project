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
static int tempo_execucao = 20;


static Tarefa tarefas[MAX_TAREFAS];
static int ntarefas = 0;


Tarefa *save;

void rotinaSigAlarm(int signum) 
{
	printf("HELLO (%d)\n", save->id);
}

int main() {

	Tarefa t;
	Config conf;
	int n, i, j;

	mkfifo("./myfifo", 0600);




	int fd = open("./myfifo",O_RDONLY);


	while(1) {

		while ((n = read(fd, &conf,sizeof(Config))) > 0) {

			printf(">%d %d\n", conf.cmd, conf.option);

			if (conf.cmd == CONFIG_EXEC) {

				if ((n = read(fd, &t,sizeof(Tarefa))) > 0) {

					t.id = ntarefas;
					t.estado = WAITING;
					tarefas[ntarefas++] = t;

				
					executarTarefa(&tarefas[ntarefas-1]);
					

				} else {

					write(1,"Tarefa nao recebida",20);
				}

			}

			if (conf.cmd == CONFIG_LIST) {

				showTarefasEmExecucao();

				//showTarefas();
			
			}

			if (conf.cmd == CONFIG_INAC_TIME) {

				tempo_inacticidade = conf.option;



			}

			if (conf.cmd == CONFIG_EXEC_TIME) {

				tempo_execucao = conf.option;



			}


			if (conf.cmd == CONFIG_LIST) {


			}


			if (conf.cmd == CONFIG_KILL) {

				for (i = 0; i < ntarefas; i++) {

					if (tarefas[i].id == conf.option) {

						for (j = 0; j < tarefas[i].ncomandos; j++) {

							kill(tarefas[i].pids[j], SIGQUIT);

						}


					}
				} 


			}

			if (conf.cmd == CONFIG_HIST) {


			}

			if (conf.cmd == CONFIG_HELP) {



			}
		}
	}

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



void showTarefasEmExecucao()
{
	int i;


	printf("\n-----------------------------\n");
	printf("Lista de tarefas (em execução):\n\n");

	for (i = 0; i < ntarefas && i < MAX_TAREFAS; i++) {

		//if (tarefas[i].estado == RUNNING)
			showTarefa(tarefas[i]);
	}

	printf("\n-----------------------------\n");

}



void executarTarefa(Tarefa *t)
{
	int i, j;
	pid_t filho;
	char ***argv = createExecArray(*t);

	signal(SIGALRM, rotinaSigAlarm);

	t->estado = RUNNING;

	// Iniciar os pipes necessários.
	for(i = 0; i < t->ncomandos - 1; i++)
		pipe(t->fds[i]);


	// Criar um processo filho para cada comando a executar.
	for(i = 0; i < t->ncomandos; i++) {

		save = t;


		if ((t->pids[i] = fork()) == 0) {

			// Configurar os inputs/outputs de cada processo
			if (i == 0) {
				// Primeiro processo.
				dup2(t->fds[i][1], 1);
			} else {
				if (i == t->ncomandos - 1) {
					//  Último processo.
					dup2(t->fds[i-1][0], 0);
				} else {
					// Processos intermédios.
					dup2(t->fds[i-1][0], 0);  
					dup2(t->fds[i][1], 1);
				}
			}

			// Fechar em cada processo os pipes.
			for(j = 0; j < t->ncomandos - 1; j++) {

				close(t->fds[j][0]);
				close(t->fds[j][1]);
			}

			
			
			
			// Executar o comando.
			if (i == t->ncomandos - 1 && fork()==0) execvp(argv[i][0], argv[i]);

			alarm(3);
			printf("OKKO!!!");

			wait(NULL);
			printf("OK!!!");

			t->estado = TERMINATED;

		} else {
			printf("PID: %d\n", t->pids[i]);
		}
	}

	// Processo pai.
	// Fechar os pipes.
	for(j = 0; j < t->ncomandos - 1; j++) {

		close(t->fds[j][0]);
		close(t->fds[j][1]);
	}



}

