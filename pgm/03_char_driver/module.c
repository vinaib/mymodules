#include <linux/module.h>

/* add header files to support character devices */
#include <linux/cdev.h>
#include <linux/fs.h>

#include<linux/time.h>
#include<linux/device.h>

#include<linux/uaccess.h>

/* define major number */
#define MAJOR_NUM	510
#define MINOR_NUM	0

#define MINOR_START 0
#define MINOR_COUNT 1

#define MYDEVNAME "MyCdev"
#define CLASS_NAME "MyClass"

#define BUFSIZE 512

// pr_fmt is defined in printk.h. if you do this again here, compiler throws
// redefine warning. To fix this warning first you undef the macro and define it
#undef pr_fmt
#define pr_fmt(fmt) "%s " fmt,__func__

static struct cdev MyCdev;
int MajorNumber;
int MinorNumber;

#ifdef OLDER_KERNEL
static struct timeval start_time;
#endif

static struct class *test_class;

char devBuffer[BUFSIZE];

/* command to pass module parameter
 *  insmod char_demo.ko int_param=5 
 */
static int int_param = 10;

/*
 * module_param(name, type, perm)
 * perm specifies the permissions of the corresponding
 * file in sysfs
 */
module_param(int_param, int, S_IRUGO);

static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("is called.\n");
	return 0;
}

static int my_dev_release(struct inode *inode, struct file *file)
{
	pr_info("is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static loff_t my_dev_lseek(struct file *file, loff_t offset, int whence)
{
   loff_t temp;
   pr_info("is called current file position is %lld\n", file->f_pos);

   switch(whence)
   {
   case SEEK_CUR:
      temp = file->f_pos + offset;   
      if(temp > BUFSIZE || temp < 0)
         return -EINVAL;
      file->f_pos = temp; 
      break;
   case SEEK_SET: 
      if(offset > BUFSIZE || offset < 0)
         return -EINVAL;
      file->f_pos = offset; 
      break;
   case SEEK_END: 
      temp = file->f_pos + offset;   
      if(temp > BUFSIZE || temp < 0)
         return -EINVAL;
      file->f_pos = temp; 
      break;
   default: 
      return -EINVAL;
   }

   pr_info("updated file position is %lld\n", file->f_pos);

   return file->f_pos;
}

/* 
 * fPos : file pointer position. starts with zero. Indicates next file position
 * to read or write.
 * count: indicates number of bytes to be read. count always should be validated
 * with max size supported by the implementation
 * buff : pointer to user mode buffer
 *
 */
static ssize_t my_dev_read(struct file *file, char __user *buff, size_t count, loff_t *fPos)
{
   pr_info("is called count %ld fpos %lld\n", count, *fPos);

   /* adjust the count */
   if((*fPos + count) > BUFSIZE)
   {
      count = BUFSIZE - *fPos;
   }

   /* copy data to user */
   #if 1
   if(copy_to_user(buff, &devBuffer[*fPos], count))
   {
      /* copy to user failed to copy count bytes, due to access permissions */
      return -EFAULT;
   }
   #else
   memcpy(buff, &devBuffer[*fPos], count);
   #endif


   /* adjust current file position fPos */
   *fPos += count;

   pr_info("number of bytes successfully read is %ld current file position %lld\n", count, *fPos);

   return count;
}

static ssize_t my_dev_write(struct file *file, const char __user *buff, size_t count, loff_t *fPos)
{
   pr_info("is called\n");

   /* adjust the count */
   if((*fPos + count) > BUFSIZE)
   {
      count = BUFSIZE - *fPos;
   }

   if(!count)
   {
      return -ENOMEM;
   }

   /* copy data from user */
   if(copy_from_user(&devBuffer[*fPos], buff, count))
   {
      /* copy from user failed to copy count bytes, due to access permissions */
      return -EFAULT;
   }

   /* adjust current file position fPos */
   *fPos += count;

   pr_info("number of bytes successfully written %ld current file position %lld\n", count, *fPos);

   return count;
}

/* declare a file_operations structure */
static const struct file_operations MyCdevFops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.release = my_dev_release,
	.unlocked_ioctl = my_dev_ioctl,
   .read = my_dev_read,
   .write = my_dev_write,
   .llseek = my_dev_lseek,
};

static int __init hello_init(void)
{
	int ret;
	struct device *myDev;

	dev_t deviceNumber = MKDEV(MAJOR_NUM, MINOR_NUM);
#ifdef OLDER_KERNEL
	do_gettimeofday(&start_time);
#endif

	pr_info("Hello world init int_param %d\n", int_param);

#if defined(DYNAMIC_DEVICE_CREATION)
	/* dynamic allocation of device numbers*/
   /* MYDEVNAME is not the device name which gets created in /dev directory.
    * This name is name of this range.
    */
	ret = alloc_chrdev_region (&deviceNumber, MINOR_START, MINOR_COUNT, MYDEVNAME);
	if(ret < 0) {
		pr_err("unable to alloc char dev region\n");
      goto EXIT_1;
	} else {
		pr_info("alloc is successful major(%d) minor(%d)\n", MAJOR(deviceNumber),
MINOR(deviceNumber));
	}
#else
	/* static allocation of device numbers */
	ret = register_chrdev_region(deviceNumber, MINOR_COUNT, MYDEVNAME);
	if (ret < 0){
		pr_err("Unable to register  major number %d\n", MAJOR_NUM);
      goto EXIT_1;
	} else {
		pr_info("Registered major number %d Major:Minor %d:%d\n", ret,
MAJOR(deviceNumber), MINOR(deviceNumber));
	}
#endif

   MajorNumber = MAJOR(deviceNumber);
   MinorNumber = MINOR(deviceNumber);

   // one more way of initializing MyCdevFops structure
   // MyCdevFops.open = my_dev_open;

	/* Initialize the cdev structure
    * Initializes cdev->ops(MyCdev) with given fops(MyCdevFops)
    */
	cdev_init(&MyCdev, &MyCdevFops);
   MyCdev.owner = THIS_MODULE;

   /* add it to the kernel space.
	 * For each minor device, we have to do cdev_init and cdev_add.
	 * int cdev_add(struct cdev *p, dev_t dev, unsigned count) 
    * register a device with VFS
    */
	ret = cdev_add(&MyCdev, deviceNumber, 1);
	if (ret < 0){
		pr_err("Unable to add cdev\n");
      goto UNREG_CHARDEV;
	} else {
		pr_info("cdev_add successfull %d\n", ret);
	}

	/* creates the directory in sysfs "/sys/class/<your class name>"
	 * Creates a class for your devices, and is visible in /sys/class/.
	 * This entry is necessary so that devtmpfs can create a device node
	 * under /dev. Drivers will have a class name and device name under /sys
	 * for each created device.
    * Path: /sys/class/MyClass/
    *       /sys/class/MyClass/MyCdev [files: dev, power, susbsystem, uevent]
    * cat /sys/class/MyClass/MyCdev/uevent 
    * MAJOR=507
    * MINOR=0
    * DEVNAME=MyCdev
    *
    * cat /sys/class/MyClass/MyCdev/dev 
    * 507:0
    * 
    * This creates a sysfs entry /sys/class/MyClass. Failure to call this there
    * will be no entry under /sys/class.
	 */
	test_class = class_create(THIS_MODULE, CLASS_NAME);
   if(IS_ERR(test_class))
   {
      /* PTR_ERR() converts pointer to error code(int)
       * ERR_PTR() converts error code(int) to pointer
       *
       */
      pr_err("Class creation failed\n");
      ret = PTR_ERR(test_class);
      goto CDEV_DEL;
   }

	/* device_create api exports the information regarding device such as 
    * 1) device file name
    * 2) major number
    * 3) minor number, to sysfs.
    *
    * udev looks for a file called "dev" in the /sys/class tree of sysfs. to 
    * determine what the major and minor number is assigned to a specific device
    *
    * this api creates a sub directory under /sys/class/<your class name>/ with
    * your device name.
    *
    * create a device node named MYDEVNAME associated to dev.
    * populate the sysfs with device information.
    * path: /dev/MyCdev
    * 
    * This create a device file under /dev/MyCdev. Failure to call this there
    * will be no entry under /dev nor under /sys/class/MyClass/
    */
	myDev = device_create(test_class, NULL, deviceNumber, NULL, MYDEVNAME);
   if(IS_ERR(myDev))
   {
      pr_err("device creation failed\n");
      ret = PTR_ERR(myDev);
      goto CLASS_DEL;
   }

   ret = 0;
	goto EXIT_1;

CLASS_DEL:
     class_destroy(test_class);

CDEV_DEL:
     cdev_del(&MyCdev);

UNREG_CHARDEV:
		unregister_chrdev_region(deviceNumber, 1);

      pr_err("Module insertion failed\n");

EXIT_1:
		return ret;
}

static void __exit hello_exit(void)
{
#ifdef OLDER_KERNEL
	struct timeval end_time;
#endif

	device_destroy(test_class, MKDEV(MajorNumber, MinorNumber));

	class_destroy(test_class);

	/* for each minor device we have to call cdev_del */
	cdev_del(&MyCdev);

	unregister_chrdev_region(MKDEV(MajorNumber, MinorNumber), 1);

#ifdef OLDER_KERNEL
	do_gettimeofday(&end_time);
#endif

	pr_info("unloading module after\n"); 
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BV");
MODULE_DESCRIPTION("Practice");



