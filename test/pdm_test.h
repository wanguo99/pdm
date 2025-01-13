#ifndef _PDM_TEST_H_
#define _PDM_TEST_H_


struct command_t{
    const char *name;
    int (*func)(int argc, char *argv[]);
};


int cmd_help(int argc, char *argv[]);

#endif /* _PDM_TEST_H_ */
