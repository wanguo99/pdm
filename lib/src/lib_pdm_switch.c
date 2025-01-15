#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int PDM_API_switch_set_state(int state)
{
	printf("%s\n", __func__);
	return 0;
}
