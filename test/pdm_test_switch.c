#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pdm_test.h"

static int pdm_test_switch_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	printf("pdm_test_switch_main\n");

	return 0;
}

static void pdm_test_switch_comment(void)
{
	printf("pdm switch module test unit.\n");
}

struct pdm_test_unit pdm_test_unit_switch = {
	.name = "switch_test",
	.comment_func = pdm_test_switch_comment,
	.main_func = pdm_test_switch_main,
};

