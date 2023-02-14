#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include<linux/slab.h>
#include "platform.h"

// pr_fmt is defined in printk.h. if you do this again here, compiler throws
// redefine warning. To fix this warning first you undef the macro and define it
#undef pr_fmt
#define pr_fmt(fmt) "%s " fmt,__func__

/* DEVICE private data */
struct pcdev_private_data
{
	struct pcdev_platform_data pdata;
   char *buffer;
	dev_t device_num;
	struct cdev cdev;
};

/* Driver private data */
struct pcdrv_private_data
{
   int total_devices;
	dev_t device_num_base;
	struct class *class_pcd;
	struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data;

int check_permission(int dev_perm, int acc_mode)
{
   if(dev_perm == RDWR)
      return 0;
   if((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode &
FMODE_WRITE)))
      return 0;
   if((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode &
FMODE_READ)))
      return 0;
   else 
      return -EPERM;
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
   return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
   return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
   return -ENOMEM;
}

int pcd_open(struct inode *inode, struct file *filp)
{
   return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
   pr_info("pcd release was successful\n");
   return 0;
}

struct file_operations pcd_fops = 
{
   .open = pcd_open,
   .release = pcd_release,
   .read = pcd_read,
   .write = pcd_write,
   .llseek = pcd_lseek,
   .owner = THIS_MODULE
};

/* Called when match platform device is found.
 * While driver is loading, if platform core does not find 
 * the device what this driver is looking for (specified in 
 * platform_driver structure "name" field) then probe is not 
 * called. Probe will be called by platform core only when 
 * device is already listed under platform core.
 */
int pcd_platform_driver_probe(struct platform_device *pdev)
{
   int ret = 0;

   struct pcdev_private_data *dev_data;

   struct pcdev_platform_data *pdata;

   pr_info("A device is detected\n");
   /*1. Get the platform data.
    * The platform data can also be fetched by helper function
    * void *dev_get_platdata(struct device *dev)
    */
   //pdata = pdev->dev.platform_data;
   pdata = (struct pcdev_platform_data*)dev_get_platdata(&pdev->dev);
   if(!pdata) {
      pr_info("No platform data\n");
      ret = -EINVAL;
      goto out;
   }

   /*2. Dynamically allocate memory for the device private data*/
   dev_data = devm_kzalloc(&pdev->dev, sizeof(struct pcdev_private_data), GFP_KERNEL);
   if(!dev_data) {
      pr_info("Cannot allocate memory\n");
      ret = -ENOMEM;
      goto out;
   }

   /* save the dev_data pointer in platfrom device structure */
   //pdev->dev.driver_data = dev_data;
   dev_set_drvdata(&pdev->dev, dev_data);

   dev_data->pdata.size = pdata->size;
   dev_data->pdata.perm = pdata->perm;
   dev_data->pdata.serial_number = pdata->serial_number;

   pr_info("Device Serial Number %s\n", dev_data->pdata.serial_number);
   pr_info("Device Size %d\n", dev_data->pdata.size);
   pr_info("Device permission %d\n", dev_data->pdata.perm);

   /*3. Dynamically allocate memory for device buffer using size information
    * from the platform data
    */
    dev_data->buffer = devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
    if(!dev_data->buffer){
      pr_info("Cannot allocate memory\n");
      ret = -ENOMEM;
      goto dev_data_free;
    }

   /*4. Get the device number */
   dev_data->device_num = pcdrv_data.device_num_base + pdev->id;

   /*5. Do cdev init and cdev add */
   cdev_init(&dev_data->cdev, &pcd_fops);

   dev_data->cdev.owner = THIS_MODULE;
   ret = cdev_add(&dev_data->cdev, dev_data->device_num, 1);
   if(ret < 0) {
      pr_err("cdev_add failed\n");
      goto buffer_free;
   }

   /*6. create device file for detected platform device */
   pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL,
   dev_data->device_num, NULL, "pcdev-%d",pdev->id);
   if(IS_ERR(pcdrv_data.device_pcd)) {
      pr_err("device creation failed\n");
      ret = PTR_ERR(pcdrv_data.device_pcd);
      goto cdev_del;
   }

   pr_info("Device probe succesfull\n");
   pcdrv_data.total_devices++;

   return 0;

   /*7. error handling */
cdev_del:
   cdev_del(&dev_data->cdev);

buffer_free:
   devm_kfree(&pdev->dev, dev_data->buffer);

dev_data_free:
   devm_kfree(&pdev->dev, dev_data);

out:
   pr_info("Device probe failed\n");
   return ret;
}

/* Gets called when device is removed from system.
 * Or gets called when this driver is removed from the system.
 * For each device probe, the corresponding device remove is called
 * to perform the device cleanup.
 */
int pcd_platform_driver_remove(struct platform_device *pdev)
{
   struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);

   /*1. remove device that was created with device_Create() */
   device_destroy(pcdrv_data.class_pcd, dev_data->device_num);

   /*2. remove cdev entry from the system */
   cdev_del(&dev_data->cdev);

   /*3. free the memory held by the device 
   * Following are not required as we used device managed
   * resource API's to allocate device buffer.
   * kfree(dev_data->buffer);
   * kfree(dev_data);
   */
   

   pr_info("A device is removed\n");
   pcdrv_data.total_devices--;

   return 0;
}

struct platform_driver pcd_platform_driver =
{
   .probe = pcd_platform_driver_probe,
   .remove = pcd_platform_driver_remove,
   .driver = 
   {
      .name = "pcd_dev"   //name should match with device
   }
};

#define MAX_DEVICES 10

/* On module load, create cdev region, class create and
 * register platform driver.
 */
static int __init pcd_platform_driver_init(void)
{
   int ret;

   /* 1. dynamically allocate a device number for MAX_DEVICES */
   ret = alloc_chrdev_region(&pcdrv_data.device_num_base ,0, MAX_DEVICES, "pcdevs");
   if(ret < 0) {
      pr_err("Alloc chardev region failed\n");
      return ret;
   }

   /* 2. create device class under /sys/class */
   pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
   if(IS_ERR(pcdrv_data.class_pcd)) {
      pr_err("Class creation failed\n");
      ret = PTR_ERR(pcdrv_data.class_pcd);
      goto unreg_chrdev;
   }

   /* 3. register a platform driver */
   platform_driver_register(&pcd_platform_driver);
   pr_info("pcd platform driver registered\n");
   return 0;

unreg_chrdev:
   unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);

   return ret;
}

/* On module unload, unregister platform driver.
 * 
 */
static void __exit pcd_platform_driver_exit(void)
{
   /*1. unregister the platform driver */
   platform_driver_unregister(&pcd_platform_driver);
   pr_info("pcd platform driver unregistered\n");

   /*2. class destroy */
   class_destroy(pcdrv_data.class_pcd);

   /*3. unregister device numbers for MAX_DEVICES*/
   unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BB");
MODULE_DESCRIPTION("example platform driver");
