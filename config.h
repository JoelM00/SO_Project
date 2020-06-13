#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_INAC_TIME 1
#define CONFIG_EXEC_TIME 2
#define CONFIG_EXEC 3
#define CONFIG_LIST 4
#define CONFIG_KILL 5
#define CONFIG_HIST 6
#define CONFIG_HELP 7

typedef struct _CONFIG_ {

	int cmd;
	int option;

} Config;

Config create_conf(int, int);
void send_conf(int, Config);

#endif