#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "tarefa.h"

#define MAX_TAREFAS 1024

void executarTarefa(Tarefa*);
void showTarefasEmExecucao();
void showTarefasTerminadas();

#endif