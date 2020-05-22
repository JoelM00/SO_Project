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

$(SERVER): servidor.o tarefa.o
	$(CC) -o $@ $^

$(CLIENT): cliente.o tarefa.o
	$(CC) -o $@ $^

servidor.o: servidor.c servidor.h tarefa.o 
	$(CC) -c servidor.c

cliente.o: cliente.c tarefa.o
	$(CC) -c cliente.c

tarefa.o: tarefa.c 
	gcc -c tarefa.c

clean:
	rm -rf *.o $(SERVER) $(CLIENT) logs.txt *~