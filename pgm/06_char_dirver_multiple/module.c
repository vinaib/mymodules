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
#define MINOR_COUNT 4

#define MYDEVNAME "MyCdev"
#define CLASS_NAME "MyClass"

#define BUFSIZE 512

// pr_fmt is defined in printk.h. if you do this again here, compiler throws
// redefine warning. To fix this warning first you undef the macro and define it
#undef pr_fmt
#define pr_fmt(fmt) "%s " fmt,__func__

#ifdef OLDER_KERNEL
static struct timeval start_time;
#endif



char devBuffer1[BUFSIZE];
char devBuffer2[BUFSIZE];
char devBuffer3[BUFSIZE];
char devBuffer4[BUFSIZE];

struct devPriv {
	char *buffer;     	// pointer to devBufferx
	unsigned size;	  	   // size of device Buffer
	const char *serial; 	// unique serial number
	int perm;		      // permission
	struct cdev cdev;	   // cdev 
	int MinorNumber;     // minor number
   struct device *dev;
};

struct drvPriv {
	int MajorNumber;
	dev_t DeviceNumber;
	int32_t TotalDevices;
	struct devPriv Devices[MINOR_COUNT];
	struct class *Class;
};

#define RDONLY 0x1
#define WRONLY 0x10
#define RDWR   0x11

struct drvPriv drvData = 
{
	.TotalDevices = MINOR_COUNT,
	.Devices = {
		[0] = {devBuffer1, BUFSIZE, "device1", RDONLY},
		[1] = {devBuffer2, BUFSIZE, "device2", WRONLY},
		[2] = {devBuffer3, BUFSIZE, "device3", RDWR},
		[3] = {devBuffer4, BUFSIZE, "device4", RDWR}
	}
};

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

static int checkPermission(int devPerm, int accessMode);

static int checkPermission(int devPerm, int accessMode)
{
   if(devPerm == RDWR) 
      return 0;
   if((devPerm == RDONLY) && ((accessMode & FMODE_READ) && !(accessMode &
FMODE_WRITE)))
      return 0;
   if((devPerm == WRONLY) && ((accessMode & FMODE_WRITE) && !(accessMode &
FMODE_READ)))
      return 0;
   else 
      return -EPERM;
}

/*
 * inode->i_rdev holds deviceNumber. Can be used to identify
 * which minor device user is trying to access.
 */
static int my_dev_open(struct inode *inode, struct file *file)
{
   int ret = -1;
   int Minor = MINOR(inode->i_rdev);

   /* find out on which device file open was attempted by the user space */
   struct devPriv *pDevPriv;

	pr_info("is called for minor number %d.\n", Minor);

   /* by using inode->i_cdev we can fetch the device private data structure */
   /* get device private data */
   pDevPriv = container_of(inode->i_cdev, struct devPriv, cdev);

   /* inode is only available in my_dev_open and my_dev_release methods. In
    * other file operation methods inode is not available. In other methods too
    * we need access to device private data. TO achieve this, we need to store
    * pDevPriv in file->private_data. By storing this information in other fops
    * method we can access device private data structure.*/
   file->private_data = pDevPriv;

   /* check permission */
   ret = checkPermission(pDevPriv->perm, file->f_mode);

   (!ret)?pr_info("open successfull\n"):pr_info("open_unscuccessfull\n");

	return ret;
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
   struct devPriv *devData = (struct devPriv*)file->private_data;
   int maxSize = devData->size;

   loff_t temp;
   pr_info("is called current file position is %lld\n", file->f_pos);

   switch(whence)
   {
   case SEEK_CUR:
      temp = file->f_pos + offset;   
      if(temp > maxSize || temp < 0)
         return -EINVAL;
      file->f_pos = temp; 
      break;
   case SEEK_SET: 
      if(offset > maxSize || offset < 0)
         return -EINVAL;
      file->f_pos = offset; 
      break;
   case SEEK_END: 
      temp = file->f_pos + offset;   
      if(temp > maxSize || temp < 0)
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
   struct devPriv *devData = (struct devPriv*)file->private_data;
   int maxSize = devData->size;

   pr_info("is called count %ld fpos %lld\n", count, *fPos);


   /* adjust the count */
   if((*fPos + count) > maxSize)
   {
      count = maxSize - *fPos;
   }

   /* copy data to user */
   #if 1
   if(copy_to_user(buff, &devData->buffer[*fPos], count))
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
   struct devPriv *devData = (struct devPriv*)file->private_data;
   int maxSize = devData->size;

   pr_info("is called\n");

   /* adjust the count */
   if((*fPos + count) > maxSize)
   {
      count = maxSize - *fPos;
   }

   if(!count)
   {
      return -ENOMEM;
   }

   /* copy data from user */
   if(copy_from_user(&devData->buffer[*fPos], buff, count))
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
   int minorDevLoopCount = 0;

#ifdef OLDER_KERNEL
	do_gettimeofday(&start_time);
#endif

	pr_info("Hello world init int_param %d\n", int_param);

	/* dynamic allocation of device numbers*/
   /* MYDEVNAME is not the device name which gets created in /dev directory.
    * This name is name of this range.
    */
	ret = alloc_chrdev_region (&drvData.DeviceNumber, MINOR_START, MINOR_COUNT, MYDEVNAME);
	if(ret < 0) {
		pr_err("unable to alloc char dev region\n");
      goto EXIT_1;
	} else {
		pr_info("alloc is successful major(%d) Baseminor(%d) minorMaxCount(%d)\n", 
         MAJOR(drvData.DeviceNumber),
         MINOR(drvData.DeviceNumber),
         MINOR_COUNT);
	}

   drvData.MajorNumber = MAJOR(drvData.DeviceNumber);

	drvData.Class = class_create(THIS_MODULE, CLASS_NAME);
   if(IS_ERR(drvData.Class))
   {
      /* PTR_ERR() converts pointer to error code(int)
       * ERR_PTR() converts error code(int) to pointer
       *
       */
      pr_err("Class creation failed\n");
      ret = PTR_ERR(drvData.Class);
      goto UNREG_CHARDEV;
   }

	/* Initialize the cdev structure
    * Initializes cdev->ops(MyCdev) with given fops(MyCdevFops)
    */
   for(minorDevLoopCount = 0; minorDevLoopCount < MINOR_COUNT; minorDevLoopCount++)
   {
   	cdev_init(&drvData.Devices[minorDevLoopCount].cdev, &MyCdevFops);
      drvData.Devices[minorDevLoopCount].cdev.owner = THIS_MODULE;

      ret = cdev_add( &drvData.Devices[minorDevLoopCount].cdev,
                      MKDEV(drvData.MajorNumber,minorDevLoopCount), 
                      1);
      if (ret < 0){
         pr_err("Unable to add cdev\n");
         goto CDEV_DEL;
      } else {
         pr_info("cdev_add successfull %d\n", ret);
         drvData.Devices[minorDevLoopCount].MinorNumber= minorDevLoopCount;
      }

      drvData.Devices[minorDevLoopCount].dev = 
            device_create( drvData.Class, 
                           NULL, 
                           MKDEV(drvData.MajorNumber,minorDevLoopCount), 
                           NULL, 
                           "MyCdev-%d", 
                           minorDevLoopCount);
      if(IS_ERR(drvData.Devices[minorDevLoopCount].dev))
      {
         pr_err("device creation failed\n");
         ret = PTR_ERR(drvData.Devices[minorDevLoopCount].dev);
         goto CLASS_DEL;
      }
   }

   ret = 0;
	goto EXIT_1;

CDEV_DEL:
CLASS_DEL:
   for(;minorDevLoopCount >= 0; minorDevLoopCount--)
   {
      device_destroy(drvData.Class, 
                     MKDEV(drvData.MajorNumber,minorDevLoopCount));
      cdev_del(&drvData.Devices[minorDevLoopCount].cdev);
   }
   class_destroy(drvData.Class);

UNREG_CHARDEV:
   unregister_chrdev_region(drvData.DeviceNumber, MINOR_COUNT);

   pr_err("Module insertion failed\n");

EXIT_1:
   return ret;
}

static void __exit hello_exit(void)
{
   int minorDevLoopCount = 0;
#ifdef OLDER_KERNEL
	struct timeval end_time;
#endif

	/* for each minor device we have to call cdev_del */
   for(minorDevLoopCount = 0; minorDevLoopCount < MINOR_COUNT; minorDevLoopCount++)
   {
	   device_destroy(drvData.Class, MKDEV(drvData.MajorNumber, minorDevLoopCount));
   	cdev_del(&drvData.Devices[minorDevLoopCount].cdev);
   }

	class_destroy(drvData.Class);

	unregister_chrdev_region(drvData.DeviceNumber, MINOR_COUNT);

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



