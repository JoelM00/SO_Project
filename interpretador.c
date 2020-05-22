#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>


#define MAX 256

int interpreter(char *line, int n) {
	int status, i = 0, j;
	char *cmd, *args[MAX][MAX],ncomandos = 0;

	char *token = strtok(line, " ");


	if (!token) return 0;

	cmd = token;

	if(!strcmp("exit", cmd)) return 0;

	while(token) {

		if (!strcmp("|", token)) {
			args[ncomandos][i] = NULL;
			i = 0;
			ncomandos++;
		} else {

			args[ncomandos][i++] = token;

		}

		token = strtok(NULL, " ");
	}

	args[ncomandos++][i] = NULL;

	if (!strcmp("cd", cmd)) {
		chdir(args[0][1]);

	} else {

		int fds[ncomandos-1][2];

		for(i = 0; i < ncomandos - 1; i++)
			pipe(fds[i]);

		for(i = 0; i < ncomandos; i++) {

			if (fork() == 0) {

				if (i == 0) {
					
					dup2(fds[i][1], 1);
				} else {
					if (i == ncomandos - 1) {
				
						dup2(fds[i-1][0], 0);
					} else {
				
						dup2(fds[i-1][0], 0);  
						dup2(fds[i][1], 1);
					}
				}

				for(j = 0; j < ncomandos - 1; j++) {

					close(fds[j][0]);
					close(fds[j][1]);
				}
						
				execvp(args[i][0], args[i]);
			}
		}

		for(j = 0; j < ncomandos - 1; j++) {

			close(fds[j][0]);
			close(fds[j][1]);
		}

		while(waitpid(-1, NULL, 0) != -1);
	}

	waitpid(-1, &status, 0);

	return n;
}


int main() {

	char buffer[MAX];
	int n;

	do {

		write(1, "argus$ ",7);
		n = read(0, buffer, MAX);

		buffer[n-1] = '\0';

		if (buffer[n-2] == '&') {

			buffer[n-2] = '\0';

			if(!fork()) n = interpreter(buffer, n);

		} else n = interpreter(buffer, n);


	} while (n>0);

	return 0;
}
