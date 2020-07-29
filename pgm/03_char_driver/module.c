#include <linux/module.h>

/* add header files to support character devices */
#include <linux/cdev.h>
#include <linux/fs.h>

#include<linux/time.h>
#include<linux/device.h>

/* define major number */
#define MAJOR_NUM	202
#define MINOR_NUM	0

#define MINOR_START 0
#define MINOR_COUNT 1

#define MYDEVNAME "my_cdev"
#define CLASS_NAME "test_class"

static struct cdev my_dev;
static struct timeval start_time;
static struct class *test_class;

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
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

/* declare a file_operations structure */
static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static int __init hello_init(void)
{
	int ret;
	struct device *my_cdev;

	dev_t dev = MKDEV(MAJOR_NUM, MINOR_NUM);

	do_gettimeofday(&start_time);

	pr_info("Hello world init int_param %d\n", int_param);

	/* static allocation of device numbers */
	ret = register_chrdev_region(dev, MINOR_COUNT, MYDEVNAME);
	if (ret < 0){
		pr_err("Unable to register  major number %d\n", MAJOR_NUM);
		return ret;
	} else {
		pr_info("Registered major number %d\n", ret);
	}

#if 0
	dev_t dev_f;
	/* dynamic allocation of device numbers*/
	ret = alloc_chardev_region (&dev_f, MINOR_START, MINOR_COUNT, MYDEVNAME);
	if(ret < 0) {
		pr_err("unable to alloc char dev region\n");
		return ret;
	} else {
		pr_info("alloc is successful\n");
	}
#endif

	/* Initialize the cdev structure and add it to the kernel space.
	 * For each minor device, we have to do cdev_init and cdev_add.
	 */
	cdev_init(&my_dev, &my_dev_fops);

	/* int cdev_add(struct cdev *p, dev_t dev, unsigned count) */
	ret= cdev_add(&my_dev, dev, 1);
	if (ret < 0){
		unregister_chrdev_region(dev, 1);
		pr_err("Unable to add cdev\n");
		return ret;
	} else {
		pr_info("cdev_add successfull %d\n", ret);
	}

	/* Register the device class
	 * Creates a class for your devices, and is visible in /sys/class/.
	 * This entry is necessary so that devtmpfs can crearte a device node
	 * under /dev. Drivers will have a class name and device name under /sys
	 * for each created device.
	 */
	test_class = class_create(THIS_MODULE, CLASS_NAME);

	/* create a device node named MYDEVNAME associated to dev */
	my_cdev = device_create(test_class, NULL, dev, NULL, MYDEVNAME);

	return 0;
}

static void __exit hello_exit(void)
{
	struct timeval end_time;

	device_destroy(test_class, MKDEV(MAJOR_NUM, MINOR_NUM));

	class_destroy(test_class);

	/* for each minor device we have to call cdev_del */
	cdev_del(&my_dev);

	unregister_chrdev_region(MKDEV(MAJOR_NUM, MINOR_NUM), 1);

	do_gettimeofday(&end_time);

	pr_info("unloading module after %ld seconds\n", 
			end_time.tv_sec - start_time.tv_sec);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BV");
MODULE_DESCRIPTION("Practice");



