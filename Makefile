# Name of the project
SERVER=argusd
CLIENT=argus

# Compiler
CC=gcc

# Flags for compiler
CC_FLAGS = -Wall -ansi

#
# Compilation and linking
#
all: $(SERVER) $(CLIENT)

$(SERVER): servidor.o tarefa.o config.o
	$(CC) -o $@ $^

$(CLIENT): cliente.o tarefa.o interpretador.o config.o
	$(CC) -o $@ $^

servidor.o: servidor.c servidor.h tarefa.o config.o servidor.h
	$(CC) -c servidor.c

cliente.o: cliente.c tarefa.o interpretador.o config.o cliente.c
	$(CC) -c cliente.c

tarefa.o: tarefa.c tarefa.h
	gcc -c tarefa.c

interpretador.o: interpretador.c config.o interpretador.h
	gcc -c interpretador.c

config.o: config.c config.h
	gcc -c config.c

clean:
	rm -rf *.o $(SERVER) $(CLIENT) logs.txt *~ myfifo