#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "pdm_test.h"
#include "pdm_switch_ioctl.h"

#define PDM_SWITCH_TEST_CDEV_FILE_PREFIX	"/dev/pdm_client/pdm_switch."

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

static int pdm_test_switch_open_device(int index)
{
	char cdev_file[256];
	int fd;

	snprintf(cdev_file, sizeof(cdev_file), "%s%d", PDM_SWITCH_TEST_CDEV_FILE_PREFIX, index);
	printf("cdev_file: %s\n", cdev_file);

	fd = open(cdev_file, O_RDWR);
	if (fd < 0) {
		perror("Failed to open device.\n");
		return -1;
	}
	return fd;
}

static void pdm_test_switch_close_device(int fd)
{
	close(fd);
}

static int pdm_test_switch_main(int argc, char *argv[])
{
	int status = 0;
	int state;
	int index;
	int fd = -1;

	if (argc < 2) {
		pdm_test_switch_comment();
		return -1;
	}

	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		pdm_test_switch_comment();
		return 0;
	}

	if (argc == 4 && !strcmp(argv[1], "-s")) {
		index = atoi(argv[2]);
		state = atoi(argv[3]);
		if (state != 0 && state != 1) {
			printf("Invalid state: %d\n", state);
			return -1;
		}
		fd = pdm_test_switch_open_device(index);
			if (fd < 0) {
			return -1;
		}
		status = pdm_test_switch_set_state(fd, state);
	}
	else if (argc == 3 && !strcmp(argv[1], "-g")) {
		index = atoi(argv[2]);
		fd = pdm_test_switch_open_device(index);
		if (fd < 0) {
			return -1;
		}
		status = pdm_test_switch_get_state(fd, &state);
		if (!status) {
			printf("Current state is %s\n", state ? "ON" : "OFF");
		} else {
			printf("get_state failed\n");
		}
	}
	else {
		pdm_test_switch_comment();
		status = -1;
	}

	if (fd >= 0) {
		pdm_test_switch_close_device(fd);
	}

	return status;
}


struct pdm_test_unit pdm_test_unit_switch = {
	.name = "switch_test",
	.comment_func = pdm_test_switch_comment,
	.main_func = pdm_test_switch_main,
};

