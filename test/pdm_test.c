#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pdm_test.h"

void pdm_test_clear_screen() {
    printf("\033[H\033[J");
}

static struct pdm_test_command_t commands[] = {
    {"show_help", pdm_test_show_help},
    {NULL, NULL}
};

int pdm_test_show_help(int argc, char *argv[])
{
	struct pdm_test_command_t *cmd;

	(void)argc;
	(void)argv;

	printf("\n### Available commands:\n");
	for (cmd = commands; cmd->name; cmd++) {
		printf(" - %s\n", cmd->name);
	}
	printf("### \n\n");

	return 0;
}

int pdm_test_dispatch_command(int argc, char *argv[])
{
	struct pdm_test_command_t *cmd;
	if (!argv[0]) {
		fprintf(stderr, "Error: No command provided.\n");
		return -1;
	}

	for (cmd = commands; cmd->name; cmd++) {
		if (!strcmp(cmd->name, argv[0])) {
			printf("[CMD]: %s\n", cmd->name);
			return cmd->func(argc, argv);
		}
	}

	printf("Error: Unknown command '%s'. Showing help instead.\n", argv[0]);
	return pdm_test_show_help(argc, argv);
}

void pdm_test_print_title() {
	pdm_test_clear_screen();
	printf("\n====== PDM Test ======\n");
}

int main(int argc, char *argv[]) {
	pdm_test_print_title();

	if (argc > 1 && !strncmp(argv[1], "./", 2)) {
		argv[1] += 2;
	}

	if (argc <= 1 || !strcmp(argv[1], "pdm_test")) {
		return pdm_test_show_help(argc - 1, argv + 1);
	} else {
		return pdm_test_dispatch_command(argc - 1, argv + 1);
	}
}
