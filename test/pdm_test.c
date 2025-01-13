#include <stdio.h>
#include <string.h>

#include "pdm_test.h"

struct command_t commands[] =
{
	{"help", cmd_help},
	{NULL, NULL}
};

int cmd_help(int argc, char *argv[])
{
	struct command_t *cmd;
	(void)argc;
	(void)argv;

	printf("Available commands:\n");
	for (cmd = commands; cmd->name; cmd++) {
		printf("  %s\n", cmd->name);
	}

	return 0;
}

int dispatch_command(int argc, char *argv[])
{
	struct command_t *cmd;

	if (argc == 0) {
		cmd_help(argc, argv);
		return 0;
	}

	for (cmd = commands; cmd->name; cmd++) {
		if (strcmp(cmd->name, argv[0]) == 0) {
			return cmd->func(argc, argv);
		}
	}

	fprintf(stderr, "Error: unknown command '%s'\n", argv[0]);
	cmd_help(argc, argv);
	return -1;
}

int main(int argc, char *argv[])
{
	if (!strcmp(argv[0], "pdm_test") || !strcmp(argv[0], "./pdm_test")) {
		if (argc == 0) {
			cmd_help(argc, argv);
			return 0;
		} else {
			dispatch_command(argc - 1, argv + 1);
		}
	} else {
		dispatch_command(argc, argv);
	}
    return 0;
}
