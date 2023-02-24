#ifndef __PLATFORM_H
#define __PLATFOMR_H

#define RDWR 0x11
#define RDONLY 0x01
#define WRONLY 0x10

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

struct pcdev_platform_data
{
   int size;
   int perm;
   const char *serial_number;
};

#endif
