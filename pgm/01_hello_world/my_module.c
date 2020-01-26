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
 * int_value=10 
 * char_ptr="meenakshi"
 * int_arr=100,101,102,103 
 */

/* globals */
int 	int_value;
int	int_arr[4];
char 	*char_ptr;
int 	cb_value = 0;

/* module params */
module_param(int_value, int, S_IRUSR|S_IWUSR);                    
module_param(char_ptr, charp, S_IRUSR|S_IWUSR);                  

module_param_array(int_arr, int, NULL, S_IRUSR|S_IWUSR);

MODULE_PARM_DESC(int_value, "this is int variable");
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

static int __init my_module_init(void)
{
	int i;

	int ret = 0;

	dev_t first = MKDEV (MAJOR_NUM, MINOR_START);

	pr_alert("int_value		= %d\n", int_value);
	pr_alert("cb_value 		= %d\n", cb_value);
	pr_alert("char_ptr		= %s\n", char_ptr);

	for (i = 0; i < (sizeof int_arr / sizeof (int)); i++) {
		pr_alert("int_arr[%d] = %d\n", i, int_arr[i]);
	}

	pr_alert("%s %s\n", __FUNCTION__, current->comm);


	ret = register_chrdev_region (
			first,
			MINOR_COUNT,
			DEV_NAME );
	if(ret < 0) {
		pr_alert("reg fail %d\n", ret);
	}

	/* returning negative value fails to insmod */
	return ret;
}

static void __exit my_module_exit(void)
{
	dev_t first = MKDEV (MAJOR_NUM, MINOR_START);

	pr_alert("%s %s\n", __FUNCTION__, current->comm);

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

