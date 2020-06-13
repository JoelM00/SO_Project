#include <unistd.h>

#include "config.h"


// ----------------------------------------------------- criar um novo config ------------------------------------------------------ \\

Config create_conf(int cmd, int option) {

	Config c;

	c.cmd = cmd;
	c.option = option;

	return c;
}

// -------------------------------------------- enviar um config por um file descriptor -------------------------------------------- \\

void send_conf(int fd, Config conf) {

	write(fd, &conf, sizeof(Config));
}