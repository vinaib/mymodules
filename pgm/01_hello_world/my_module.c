#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/moduleparam.h>

/* how to pass module params?
 * sudo insmod my_module.ko 
 * int_value=10 
 * char_ptr="meenakshi"
 * int_arr=100,101,102,103 
 */

/* globals */
int 	int_value;
int		int_arr[4];
char 	*char_ptr;
int 	cb_value = 0;

/* module params */
module_param(int_value, int, S_IRUSR|S_IWUSR);                    
module_param(char_ptr, charp, S_IRUSR|S_IWUSR);                  

module_param_array(int_arr, int, NULL, S_IRUSR|S_IWUSR);  
 
int notify_param(const char *val, const struct kernel_param *kp)
{ 
	/* Use helper for write variable */
	int res = param_set_int(val, kp);

	if(res == 0) {

		printk(KERN_INFO "Call back function called...\n");
		printk(KERN_INFO "New value of cb_value = %d\n", cb_value);

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
 
module_param_cb(cb_value, &my_param_ops, &cb_value, S_IRUGO|S_IWUSR );
 
/* __init and __exit are actually kernel macros
 * defined in include/linux/init.h
 *    	#define __init__section(.init.text)
 * 	#define __exit__section(.exit.text)
 */
static int __init my_module_init(void)
{
	int i;

	printk(KERN_INFO "int_value		= %d\n", int_value);
	printk(KERN_INFO "cb_value 		= %d\n", cb_value);
	printk(KERN_INFO "char_ptr		= %s\n", char_ptr);

	for (i = 0; i < (sizeof int_arr / sizeof (int)); i++) {
		printk(KERN_INFO "int_arr[%d] = %d\n", i, int_arr[i]);
	}

	printk(KERN_INFO "%s\n", __FUNCTION__);

	return 0;
}

static void __exit my_module_exit(void)
{
	printk(KERN_INFO "%s\n", __FUNCTION__);
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("vinai");
MODULE_DESCRIPTION("hello world");
MODULE_VERSION("0.1");

