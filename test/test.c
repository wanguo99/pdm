#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_CMD _IOR('M', 0, int)  // 示例命令，读取一个整数

int main(int argc, char *argv[]) {
    int fd;
    int result;

    // 检查命令行参数
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <device_path>\n", argv[0]);
        return 1;
    }

    const char *device_path = argv[1];

    // 打开设备文件
    fd = open(device_path, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return 1;
    }

    // 调用 ioctl
    if (ioctl(fd, IOCTL_CMD, &result) < 0) {
        perror("ioctl call failed");
        close(fd);
        return 1;
    }

    // 打印结果
    printf("ioctl result: %d\n", result);

    // 关闭设备文件
    close(fd);

    return 0;
}
