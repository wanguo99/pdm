#ifndef _PDM_TEST_H_
#define _PDM_TEST_H_

#define PDM_TEST_PROGRAM_NAME	"pdm_test"

typedef int (*command_func_t)(int argc, char *argv[]);

struct pdm_test_unit{
	const char *name;
	const char *comment;
	command_func_t func;
};


static inline void pdm_test_clear_screen(void)
{
	printf("\033[H\033[J");
}

struct pdm_test_unit pdm_test_unit_show_help;


#endif /* _PDM_TEST_H_ */
