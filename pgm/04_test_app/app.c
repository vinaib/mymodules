#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

/*
 * insmod char_demo.ko int_param=5
 * busybox mknod /dev/my_char_device c 202 0
 * cat /proc/modules or lsmod
 * cat /sys/module/char_demo/
 * Assigned major numbers can be viewed by:
 * cat /proc/devices

 */

#define DEVICE "/dev/my_cdev"

int main(void)
{
	int fd;

	printf("Testing my_char_dev(202)\n");

	fd = open(DEVICE, O_RDONLY);
	if(fd < 0) {
		printf("%s open failed %d %d %s\n", DEVICE, fd, errno, strerror(errno));
	}

	if((ioctl(fd, 100, 110)) < 0) {
		printf("ioctl failed\n");
	}

	if(close(fd) < 0) {
		printf("%s close failed\n", DEVICE);
	}

	return 0;
}
