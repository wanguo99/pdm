#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pdm_test.h"

static struct pdm_test_unit *pdm_test_units[] = {
    &pdm_test_unit_show_help,
    NULL
};


static struct pdm_test_unit *pdm_test_match_unit(struct pdm_test_unit **units,
const char *name)
{
    if (!units || !name) {
        return NULL;
    }

    while (*units != NULL) {
        if (strcmp((*units)->name, name) == 0) {
            return *units;
        }
        units++;
    }

    return NULL;
}

static int pdm_test_show_help(int argc, char *argv[])
{
    struct pdm_test_unit **units = pdm_test_units;

    (void)argc;
    (void)argv;

    printf("\n### Available commands:\n");

    while (*units != NULL) {
        printf("------------------\n");
        printf(" - %s:\n", (*units)->name);
        printf("   %s\n", (*units)->comment);
        units++;
    }

    printf("### \n\n");

    return 0;
}

struct pdm_test_unit pdm_test_unit_show_help = {
    .name = "show_help",
    .comment = "show this message.",
    .func = pdm_test_show_help
};

static int pdm_test_dispatch_command(int argc, char *argv[])
{
    struct pdm_test_unit *unit;

    if (!argv[0]) {
        fprintf(stderr, "Error: No command provided.\n");
        return -1;
    }

    unit = pdm_test_match_unit(pdm_test_units, argv[0]);
    if (!unit) {
        printf("Error: Unknown command '%s'.\n", argv[0]);
        return pdm_test_show_help(argc, argv);
    }

    printf("[CMD]: %s\n", unit->name);
    return unit->func(argc, argv);
}

static void pdm_test_print_title()
{
    pdm_test_clear_screen();
    printf("\n====== PDM Test ======\n");
}

int main(int argc, char *argv[])
{
    pdm_test_print_title();

    if (argc > 1 && !strncmp(argv[1], "./", 2)) {
        argv[1] += 2;
    }

    if (argc <= 1 || !strcmp(argv[1], PDM_TEST_PROGRAM_NAME)) {
        return pdm_test_show_help(argc - 1, argv + 1);
    } else {
        return pdm_test_dispatch_command(argc - 1, argv + 1);
    }
}
