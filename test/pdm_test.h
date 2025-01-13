#ifndef _PDM_TEST_H_
#define _PDM_TEST_H_

#define PDM_TEST_PROGRAM_NAME	"pdm_test"

typedef int (*main_func_t)(int argc, char *argv[]);
typedef void (*comment_func_t)(void);

struct pdm_test_unit{
	const char *name;
	comment_func_t comment_func;
	main_func_t main_func;
};


static inline void pdm_test_clear_screen(void)
{
	printf("\033[H\033[J");
}

extern struct pdm_test_unit pdm_test_unit_show_help;
extern struct pdm_test_unit pdm_test_unit_switch;


#endif /* _PDM_TEST_H_ */
