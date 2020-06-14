# Compiler
CC=gcc

# Flags for compiler
CC_FLAGS = -Wall -ansi

# Compilation and linking
all: argusd argus

argusd: argusd.o tarefa.o config.o
	$(CC) $(CFLAGS) argusd.o tarefa.o config.o -o argusd

argus: argus.o tarefa.o interpretador.o config.o
	$(CC) $(CFLAGS) argus.o tarefa.o interpretador.o config.o -o argus

argusd.o: argusd.c argusd.h
	$(CC) $(CFLAGS) -c argusd.c

argus.o: argus.c
	$(CC) $(CFLAGS) -c argus.c

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