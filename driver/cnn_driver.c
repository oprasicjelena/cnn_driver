#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/types.h> //dev_t type for defining major and minor nums
#include <linux/kdev_t.h> //majro minor macros
#include <linux/device.h> //class_create & device_create function for creating nodes

#include <linux/fs.h>

#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>

#include <linux/io.h> //iowrite ioread
#include <linux/slab.h>//kmalloc kfree
#include <linux/platform_device.h>//platform driver
#include <linux/of.h>//of_match_table
#include <linux/ioport.h>//ioremap

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;


static int __init cnn_driver_init(void)
{
	int ret = 0;
	
	ret = alloc_chrdev_region(&my_dev_id, 0, 1, "cnn_driver");
	if(ret)
	{
		printk(KERN_ERR "Failed to register char device\n");
		return ret;
	}

	my_class = class_create(THIS_MODULE, "cnn_driver_class");
	if(NULL == my_class)
	{
		printk(KERN_ERR "Failed to create class\n");
		goto fail_0;
	}

	my_device = device_create(my_class, NULL, my_dev_id, NULL, "cnn_driver");
	if(NULL == my_device)
	{
		printk(KERN_ERR "Failed to create device\n");
		goto fail_1;
	}
	
	printk(KERN_INFO "Device created.\n");
	printk(KERN_INFO "Hello from cnn driver!\n");

	fail_1:
		class_destroy(my_class);
	fail_0:
		unregister_chrdev_region(my_dev_id,1);

	return -1;
}

static void __exit cnn_driver_exit(void)
{
	device_destroy(my_class, my_dev_id);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1);
	printk("Goodbye from cnn driver\n");
}


module_init(cnn_driver_init);
module_exit(cnn_driver_exit);





