#include<linux/module.h>
#include<linux/init.h>
//#include<liunx/memory.h>
#include<linux/kernel.h>

#include<linux/sched/signal.h>
#include<linux/sched.h>

/* swapper_pg_dir is macro defined in head.S, but not exported by kernel */
extern int swapper_pg_dir;

//linux/arch/arm/include/asm/pgtable.h

char *parse_task_state(const long state) 
{
	switch(state) 
	{
		case TASK_RUNNING: return "TASK_RUNNING";
		case TASK_INTERRUPTIBLE: return "TASK_INTERRUPTIBLE";
		case TASK_UNINTERRUPTIBLE: return "TASK_UNINTERRUPTIBLE";
		case __TASK_STOPPED: return "__TASK_STOPPED";
	    case __TASK_TRACED: return "__TASK_TRACED";
		case TASK_PARKED: return "TASK_PARKED";
		case TASK_DEAD: return "TASK_DEAD";
		case TASK_WAKEKILL: return "TASK_WAKEKILL";
		case TASK_WAKING: return "TASK_WAKING";
		case TASK_NOLOAD: return "TASK_NOLOAD";
		case TASK_NEW: return "TASK_NEW";
		case TASK_KILLABLE: return "TASK_KILLABLE";
		case TASK_STOPPED: return "TASK_STOPPED";
		case TASK_TRACED: return "TASK_TRACED";
		case TASK_IDLE: return "TASK_IDLE";
		case TASK_NORMAL: return "TASK_NORMAL";
		default: return "TODO!";
	}
}

static int __init my_module_init(void)
{
	int ret = 0;

	struct task_struct *task;
	int counter = 0;

	register unsigned long csp asm("sp");
	register unsigned long clr asm("lr");
	register unsigned long cr1 asm("r1");

	pr_alert("TASK_SIZE 				0x%lx\n", TASK_SIZE);
	pr_alert("CONFIG_PAGE_OFFSET 		0x%x\n", CONFIG_PAGE_OFFSET);
	pr_alert("MODULES_VADDR		 		0x%lx\n", MODULES_VADDR);
	pr_alert("PAGE_OFFSET 				0x%lx\n", PAGE_OFFSET);
	pr_alert("TASK_UNMAPPED_BASE	 	0x%lx\n", TASK_UNMAPPED_BASE);
	pr_alert("VECTORS_BASE 				0x%lx\n", VECTORS_BASE);
	pr_alert("THREAD_SIZE 				0x%lx\n", THREAD_SIZE);
	pr_alert("swapper_pg_dir			0x%lx\n", swapper_pg_dir);

	//pr_alert("MODULES_END 				0x%lx\n",  MODULES_END);
	//pr_alert("PKMAP_BASE 				0x%lx\n",  PKMAP_BASE);
	//pr_alert("VA_START	 				0x%lx\n", VA_START);
	//pr_alert("VMALLOC_START	 			0x%lx\n", VMALLOC_START);
	//pr_alert("VMALLOC_END	 			0x%lx\n", VMALLOC_END);
	//pr_alert("KERNEL_START	 			0x%lx\n", KERNEL_START);
	//pr_alert("KERNEL_END	 			0x%lx\n", KERNEL_END);
	//pr_alert("KERNEL_STACK 				0x%lx\n", KERNEL_STACK);
#if 0
	pr_alert("0x%lx\n", );
#endif
	pr_alert("Current process list ......\n");

	for_each_process(task) {
		pr_alert("%s :[%d]: %s:[%ld] \n", task->comm, task->pid,
				parse_task_state(task->state),  task->state);
		counter++;
	}

	pr_alert("%d processes present in system\n", counter);

	pr_alert("stack pointer %p\n", (void *)csp);
	pr_alert("LR %p\n", (void *)clr);
	pr_alert("R1 %lx\n", cr1);
	pr_alert("thread info->task %p\n", ((struct thread_info *)csp)->task);
	pr_alert("current address %p\n", current);

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

