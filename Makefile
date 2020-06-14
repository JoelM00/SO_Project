# Compiler
CC=gcc

# Flags for compiler
CC_FLAGS = -Wall -ansi

# Compilation and linking
all: argusd argus

argusd: servidor.o tarefa.o config.o
	$(CC) $(CFLAGS) servidor.o tarefa.o config.o -o argusd

argus: cliente.o tarefa.o interpretador.o config.o
	$(CC) $(CFLAGS) cliente.o tarefa.o interpretador.o config.o -o argus

servidor.o: servidor.c servidor.h
	$(CC) $(CFLAGS) -c servidor.c

cliente.o: cliente.c
	$(CC) $(CFLAGS) -c cliente.c

tarefa.o: tarefa.c tarefa.h
	$(CC) $(CFLAGS) -c tarefa.c

interpretador.o: interpretador.c interpretador.h
	$(CC) $(CFLAGS) -c interpretador.c

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

clean:
	rm -f argusd
	rm -f argus
	rm -f myfifo
	rm -f terminadas
	rm -f output
	rm *.o