#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_CMD _IOR('M', 0, int)

int main(int argc, char *argv[]) {
    int fd;
    int result;
    const char *device_path = argv[1];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <device_path>\n", argv[0]);
        return 1;
    }

    fd = open(device_path, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return 1;
    }

    if (ioctl(fd, IOCTL_CMD, &result) < 0) {
        perror("ioctl call failed");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
