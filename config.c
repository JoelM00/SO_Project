#include <unistd.h>

#include "config.h"

Config create_conf(int cmd, int option) {

	Config c;

	c.cmd = cmd;
	c.option = option;

	return c;
}

void send_conf(int fd, Config conf) {

	write(fd, &conf, sizeof(Config));
}