#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/moduleparam.h>		/* for module_param */
#include<linux/sched.h>			/* for current */

#include<linux/types.h>			/* dev_t */
#include<linux/kdev_t.h>		/* MKDEV */
#include<linux/fs.h>			/* chardev_region */

/* how to pass module params?
 * sudo insmod my_module.ko 
 * scull_major=10
 * char_ptr="meenakshi"
 * int_arr=100,101,102,103 
 */

/* globals */
int 	scull_major;
int	int_arr[4];
char 	*char_ptr;
int 	cb_value = 0;

/* module params */
module_param(scull_major, int, S_IRUSR|S_IWUSR);
module_param(char_ptr, charp, S_IRUSR|S_IWUSR);

module_param_array(int_arr, int, NULL, S_IRUSR|S_IWUSR);

MODULE_PARM_DESC(scull_major, "major number");
MODULE_PARM_DESC(char_ptr, "this is char pointer variable");
MODULE_PARM_DESC(int_arr, "this is integer arry");
 
int notify_param(const char *val, const struct kernel_param *kp)
{ 
	/* Use helper for write variable */
	int res = param_set_int(val, kp);

	if(res == 0) {

		pr_alert("Call back function called...\n");
		pr_alert("New value of cb_value = %d\n", cb_value);

		return 0;
	}

        return -1;
}
 
const struct kernel_param_ops my_param_ops =
{
	/* Use our setter */
	.set = &notify_param,

	/* standard getter */
	.get = &param_get_int,
};
 
module_param_cb(cb_value, &my_param_ops, &cb_value, S_IRUSR|S_IWUSR );
 
/* __init and __exit are actually kernel macros
 * defined in include/linux/init.h
 *    	#define __init__section(.init.text)
 * 	#define __exit__section(.exit.text)
 */

#define MAJOR_NUM 	511
#define MINOR_START 	0
#define MINOR_LAST 	3
#define MINOR_COUNT	4
#define DEV_NAME	"scull_char"

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data; 	/* Pointer to first quantum set */
	int quantum; 			/* the current quantum size */
	int qset;			/* the current array size */
	unsigned long size;		/* amount of data stored here */
	unsigned int access_key; 	/* used by sculluid and scullpriv */
	struct semaphore sem;		/* mutual exclusion semaphore */
	struct cdev cdev; 		/* Char device structure */
};

struct scull_dev sdev;

int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;			/* device information */
	
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;		/* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) = = O_WRONLY) {
		scull_trim(dev); 		/* ignore errors */
	}
	
	return 0;	/* success */
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* style: c tagged structure initialization syntax */
struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.llseek = scull_llseek,
	.read = scull_read,
	.write = scull_write,
	.ioctl = scull_ioctl,
	.open = scull_open,
	.release = scull_release,
};

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err, devno = MKDEV(scull_major, MINOR_START + index);

	cdev_init(&dev->cdev, &scull_fops);

	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	
	/* adds cdev to the kernel/system. brings cdev to live */
	err = cdev_add (&dev->cdev, devno, 1);

	/* Fail gracefully if need be */
	if (err)
		pr_alert("Error %d adding scull%d", err, index);
}

static int __init my_module_init(void)
{
	int i;

	int ret = 0;

	dev_t first;

	pr_alert("scull_major		= %d\n", scull_major);
	pr_alert("cb_value 		= %d\n", cb_value);
	pr_alert("char_ptr		= %s\n", char_ptr);

	for (i = 0; i < (sizeof int_arr / sizeof (int)); i++) {
		pr_alert("int_arr[%d] = %d\n", i, int_arr[i]);
	}

	pr_alert("%s %s\n", __FUNCTION__, current->comm);


	if(scull_major) {
		first = MKDEV (scull_major, MINOR_START);

		ret = register_chrdev_region (
			first,
			MINOR_COUNT,
			DEV_NAME );
	} else {
		ret = alloc_chrdev_region (
				&first,
				MINOR_START,
				MINOR_COUNT,
				DEV_NAME);

		scull_major = MAJOR(first);
	}

	if(ret < 0) {
		pr_alert("reg fail %d\n", ret);
	} else {
		pr_alert("scull major %d\n", scull_major);
	}

	scull_setup_cdev(&sdev, 0);

	/* returning negative value fails to insmod */
	return ret;
}

static void __exit my_module_exit(void)
{
	dev_t first = MKDEV (scull_major, MINOR_START);

	pr_alert("%s %s\n", __FUNCTION__, current->comm);

	/* removed char device from kernel/system*/
	cdev_del(&sdev->cdev);

	unregister_chrdev_region (
			first,
			MINOR_COUNT);

}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("vinai");
MODULE_DESCRIPTION("hello world");
MODULE_VERSION("0.1");

