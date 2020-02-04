#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/moduleparam.h>		/* for module_param */
#include<linux/sched.h>			/* for current */

#include<linux/cdev.h>			/* cdev */
#include<linux/types.h>			/* dev_t */
#include<linux/kdev_t.h>		/* MKDEV */
#include<linux/fs.h>			/* chardev_region */
#include<linux/slab.h>			/* kmalloc/kfree */
#include<linux/uaccess.h>		/* copy_to/from_user */

#include"my_module.h"

/* how to pass module params?
 * sudo insmod my_module.ko 
 * scull_major=10
 * char_ptr="meenakshi"
 * int_arr=100,101,102,103 
 */

/* globals */
int 	scull_major;
int	scull_quantum = SCULL_QUANTUM;
int	scull_qset = SCULL_QSET;
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

		PDEBUG("Call back function called...\n");
		PDEBUG("New value of cb_value = %d\n", cb_value);

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


/* Global dev structure
 */
scull_dev sdev;

/* Function: 
 * scull_trim is also used in the module cleanup function to return memory used
 * by scull to the system.
 */
int scull_trim(scull_dev *dev)
{
	scull_dev *next, *dptr;
	int qset = dev->qset;	/* "dev" is not-null */
	int i;

	for (dptr = dev; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				if (dptr->data[i]) 
					kfree(dptr->data[i]);

			kfree(dptr->data);
			dptr->data = NULL;
		}

		next = dptr->next;
		if(dptr != dev)
			kfree(dptr);
	}

	dev->size = 0;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->data = NULL;
	
	return 0;
}

/* Function: 
 * scull_open 
 */
int scull_open(struct inode *inode, struct file *filp)
{
	scull_dev *dev;			/* device information */
	
	dev = container_of(inode->i_cdev, scull_dev, cdev);
	filp->private_data = dev;		/* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		scull_trim(dev); 		/* ignore errors */
	}
	
	return 0;	/* success */
}

/* Function: 
 * scull_release
 */
int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* Function:
 * Follow the list 
 */
scull_dev *scull_follow(scull_dev *dev, int n)
{
    while (n--) {
        if (!dev->next) {
            dev->next = kmalloc(sizeof(scull_dev), GFP_KERNEL);
            memset(dev->next, 0, sizeof(scull_dev));
        }
        dev = dev->next;
        continue;
    }
    return dev;
}

/* Function: 
 * scull_read 
 */
ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
loff_t *f_pos)
{
	scull_dev *dev = filp->private_data;
	scull_dev *dptr;

	/* the first listitem */
	int quantum = dev->quantum;
	int qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*f_pos >= dev->size)
		goto out;

	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; 
	q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = scull_follow(dev, item);

	if (!dptr->data)
		goto out; /* don't fill holes */

	if(!dptr->data[s_pos])
		goto out;

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;
	
	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

out:
	up(&dev->sem);

	return retval;	
}


/* Function: 
 * scull_write
 */
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
loff_t *f_pos)
{
	scull_dev *dev = filp->private_data;
	scull_dev *dptr;

	int quantum = dev->quantum;
	int qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; 
	q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = scull_follow(dev, item);

	if (dptr == NULL)
		goto out;

	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;

		memset(dptr->data, 0, qset * sizeof(char *));
	}

	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}

	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

	/* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

out:
	up(&dev->sem);

	return retval;
}	

/* Function:
 * scull_ioctl
 *  old ioctl signature
 * int scull_ioctl(struct inode *inode, struct file *filp,
 *                 unsigned int cmd, unsigned long arg)
 */
long scull_ioctl(struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	return ret;
}

/* Function: 
 * scull_llseek
 */
loff_t  scull_llseek(struct file *filp, loff_t off, int whence)
{
	return (loff_t)0;
}

/* style: c tagged structure initialization syntax */
// SCULL_QUANTUM and SCULL_QSET in scull.h: compile time
// scull_quantum and scull_qset at modile load time
// or by changing both the current and default values using ioctl at runtime
struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.llseek = scull_llseek,
	.read = scull_read,
	.write = scull_write,
	.unlocked_ioctl = scull_ioctl,
	.open = scull_open,
	.release = scull_release,
};

/* Function: 
 * scull_setup_cdev
 */
static void scull_setup_cdev(scull_dev *dev, int index)
{
	int err, devno = MKDEV(scull_major, MINOR_START + index);

	cdev_init(&dev->cdev, &scull_fops);

	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	
	/* adds cdev to the kernel/system. brings cdev to live */
	err = cdev_add (&dev->cdev, devno, 1);

	/* Fail gracefully if need be */
	if (err)
		PDEBUG("Error %d adding scull%d", err, index);
}

static int __init my_module_init(void)
{
	int i;

	int ret = 0;

	dev_t first;

	PDEBUG("scull_major		= %d\n", scull_major);
	PDEBUG("cb_value 		= %d\n", cb_value);
	PDEBUG("char_ptr		= %s\n", char_ptr);

	for (i = 0; i < (sizeof int_arr / sizeof (int)); i++) {
		PDEBUG("int_arr[%d] = %d\n", i, int_arr[i]);
	}

	PDEBUG("%s %s\n", __FUNCTION__, current->comm);


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
		PDEBUG("reg fail %d\n", ret);
	} else {
		PDEBUG("scull major %d\n", scull_major);
	}

	scull_setup_cdev(&sdev, 0);

	/* returning negative value fails to insmod */
	return ret;
}

static void __exit my_module_exit(void)
{
	dev_t first = MKDEV (scull_major, MINOR_START);

	PDEBUG("%s %s\n", __FUNCTION__, current->comm);

	/* removed char device from kernel/system*/
	cdev_del(&sdev.cdev);

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

