#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "pdm_test.h"
#include "pdm_switch_ioctl.h"

#define PDM_SWITCH_TEST_CDEV_FILE	"/dev/pdm_client/pdm_switch.0"

static int pdm_test_switch_set_state(int fd, int state)
{
    if (ioctl(fd, PDM_SWITCH_SET_STATE, &state) < 0) {
        perror("ioctl PDM_SWITCH_SET_STATE failed");
        return -1;
    }

    printf("State set to: %d\n", state);
    return 0;
}

static int pdm_test_switch_get_state(int fd, int *state)
{
    if (ioctl(fd, PDM_SWITCH_GET_STATE, state) < 0) {
        perror("ioctl PDM_SWITCH_GET_STATE failed");
        return -1;
    }

    return 0;
}


static void pdm_test_switch_comment(void)
{
    printf("PDM Switch Module Test Unit\n");
    printf("Usage: pdm_test_switch [-h] [-s <state>] [-g]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help      Show this help message and exit.\n");
    printf("  -s <state>      Set the switch state to the specified integer value.\n");
    printf("                  Example: -s 1 (to set state to 1)\n");
    printf("  -g              Get the current switch state.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  pdm_test_switch -s 1   # Set the switch state to 1\n");
    printf("  pdm_test_switch -g     # Get the current switch state\n");
}

static int pdm_test_switch_main(int argc, char *argv[])
{
	int fd;
	int status;
	int state;

	(void)argc;
	(void)argv;

	printf("pdm_test_switch_main\n");

	fd = open(PDM_SWITCH_TEST_CDEV_FILE, O_RDWR);
	if (fd < 0) {
		perror("Failed to open device file");
		return -1;
	}

	if (!strcmp(argv[1], "-s")) {
		if (argc != 3) {
			fprintf(stderr, "Usage: %s -s <state>\n", argv[0]);
			status = -1;
			goto close_fd;
		}
		state = atoi(argv[2]);
		status = pdm_test_switch_set_state(fd, state);
		if (status) {
			printf("get_state failed\n");
			goto close_fd;
		}
	}
	else if (!strcmp(argv[1], "-g")) {
		status = pdm_test_switch_get_state(fd, &state);
		if (!status) {
			printf("Current state is %s\n", state ? "ON" : "OFF");
		}
	}
	else {
		pdm_test_switch_comment();
	}

close_fd:
	close(fd);
	return status;
}

struct pdm_test_unit pdm_test_unit_switch = {
	.name = "switch_test",
	.comment_func = pdm_test_switch_comment,
	.main_func = pdm_test_switch_main,
};

