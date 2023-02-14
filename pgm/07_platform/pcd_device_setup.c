#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

static void pcd_release(struct device *dev);

/* device private data */
struct pcdev_platform_data pcdevPData[4] =
{
 [0] = {.size = 512, .perm=RDWR, .serial_number = "PCDEV1111"},
 [1] = {.size = 1024, .perm=RDWR, .serial_number = "PCDEV2222"},
 [2] = {.size = 128, .perm=RDONLY, .serial_number = "PCDEV3333"},
 [3] = {.size = 32, .perm=WRONLY, .serial_number = "PCDEV4444"}
};

// 1. create two platform devices
// store platform private data
// platform device must initialize platform device release function. this
// release function is part of struct device.
struct platform_device pcDev1 =
{
   .name = "pcd_dev",
   .id = 0,              // used for indexing purpose
   // platform device private data
   .dev = { 
      .platform_data = &pcdevPData[0],
      .release = pcd_release
   }
};

struct platform_device pcDev2 =
{
   .name = "pcd_dev",
   .id = 1,
   // platform device private data
   .dev = { 
      .platform_data = &pcdevPData[1],
      .release = pcd_release
   }
};

struct platform_device pcDev3 =
{
   .name = "pcd_dev",
   .id = 2,
   // platform device private data
   .dev = { 
      .platform_data = &pcdevPData[2],
      .release = pcd_release
   }
};

struct platform_device pcDev4 =
{
   .name = "pcd_dev",
   .id = 3,
   // platform device private data
   .dev = { 
      .platform_data = &pcdevPData[3],
      .release = pcd_release
   }
};


/* For platform_add_devices
*/
struct platform_device *platform_pcdevs[] = 
{ &pcDev1, &pcDev2, &pcDev3, &pcDev4
};

/* Called when device module is removed.
 * Called for every platform device registration.
 * In this case, it is called twice as we are adding
 * two platforms devices
 */
static void pcd_release(struct device *dev)
{
   pr_info("device released\n");
}

/* On module load this is called.*/
static int __init pcdev_platform_init(void)
{
   // register platform device
   // On registering platform_device_register, platform devices are created
   // under "/sys/devices/platform" "pcd_dev.0, pcd_dev.1".

   #if 0 
   platform_device_register(&pcDev1);
   platform_device_register(&pcDev2);
   platform_device_register(&pcDev3);
   platform_device_register(&pcDev4);
   #endif

   /* or another way of adding platform device is platform_add_devices() 
    */
   platform_add_devices(platform_pcdevs, ARRAY_SIZE(platform_pcdevs));

   pr_info("Device setup module loaded\n");

   return 0;        
}

/* On module exit: 
 * First pcd_release is called and then pcdev_platform_exit is called.
 *
 */
static void __exit pcdev_platform_exit(void)
{
   platform_device_unregister(&pcDev1);
   platform_device_unregister(&pcDev2);

   pr_info("Device setup module unloaded\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module which registers platform devices");
