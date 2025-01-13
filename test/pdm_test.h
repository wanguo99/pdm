#ifndef _PDM_TEST_H_
#define _PDM_TEST_H_

typedef int (*command_func_t)(int argc, char *argv[]);

struct pdm_test_command_t{
	const char *name;
	command_func_t func;
};

int pdm_test_show_help(int argc, char *argv[]);

#endif /* _PDM_TEST_H_ */
