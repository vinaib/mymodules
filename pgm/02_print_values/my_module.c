#include<linux/module.h>
#include<linux/init.h>
//#include<liunx/memory.h>
#include<linux/kernel.h>

//linux/arch/arm/include/asm/pgtable.h

static int __init my_module_init(void)
{
	int ret = 0;

	pr_alert("TASK_SIZE 				0x%lx\n", TASK_SIZE);
	pr_alert("CONFIG_PAGE_OFFSET 		0x%x\n", CONFIG_PAGE_OFFSET);
	pr_alert("MODULES_VADDR		 		0x%lx\n",  MODULES_VADDR);
	//pr_alert("MODULES_END 				0x%lx\n",  MODULES_END);
	//pr_alert("PKMAP_BASE 				0x%lx\n",  PKMAP_BASE);
	pr_alert("PAGE_OFFSET 				0x%lx\n", PAGE_OFFSET);
	//pr_alert("VA_START	 				0x%lx\n", VA_START);
	//pr_alert("VMALLOC_START	 			0x%lx\n", VMALLOC_START);
	//pr_alert("VMALLOC_END	 			0x%lx\n", VMALLOC_END);
	//pr_alert("KERNEL_START	 			0x%lx\n", KERNEL_START);
	//pr_alert("KERNEL_END	 			0x%lx\n", KERNEL_END);
	pr_alert("TASK_UNMAPPED_BASE	 	0x%lx\n", TASK_UNMAPPED_BASE);
	pr_alert("VECTORS_BASE 				0x%lx\n", VECTORS_BASE);
	pr_alert("THREAD_SIZE 				0x%lx\n", THREAD_SIZE);
	//pr_alert("KERNEL_STACK 				0x%lx\n", KERNEL_STACK);
#if 0
	pr_alert("0x%lx\n", );
#endif
	/* returning negative value fails to insmod */
	return ret;
}

static void __exit my_module_exit(void)
{
	pr_alert("%s\n", __FUNCTION__);

}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vinai");
MODULE_DESCRIPTION("print values");
MODULE_VERSION("0.1");

