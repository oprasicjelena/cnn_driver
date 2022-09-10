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

dev_t dev_id;
struct cdev *my_cdev;


ssize_t driver_cnn_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset); 
static ssize_t driver_cnn_write(struct file *pfile,const  char __user *buffer, size_t length, loff_t *offset);
int driver_cnn_open(struct inode *pinode, struct file *pfile);
int driver_cnn_close(struct inode *pinode, struct file *pfile);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = driver_cnn_open,
	.read = driver_cnn_read,
	.write = driver_cnn_write,
	.release = driver_cnn_close,
};

int driver_cnn_open(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully opened file\n");
	return 0;
}

int driver_cnn_close(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully closed file\n");
	return 0;
}

ssize_t driver_cnn_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	return 0;
}

static ssize_t driver_cnn_write(struct file *pfile,const  char __user *buffer, size_t length, loff_t *offset) 
{
	return 0;
}

static int __init driver_cnn_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&dev_id, 0, 1, "driver_cnn");
	if(ret)
	{
		printk(KERN_ERR "Failed to register char device\n");
		return ret;
	}
	
	my_cdev = cdev_alloc();
	my_cdev -> owner = THIS_MODULE;
	my_cdev -> ops = &my_fops;
	
	ret = cdev_add(my_cdev, dev_id, 1);
	if(ret)
	{
		unregister_chrdev_region(dev_id, 1);
		printk(KERN_ERR "Failed to add char device\n");
		return ret;
	}

	printk(KERN_INFO "Hello from cnn driver!\n");
	return 0;
}

static void __exit diver_cnn_exit(void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(dev_id, 1);
	printk(KERN_INFO"Goodbye from cnn driver\n");
}


module_init(driver_cnn_init);
module_exit(diver_cnn_exit);





