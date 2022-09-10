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

// ------------------------------------------
// REGISTER CONSTANTS
// ------------------------------------------
#define XIL_CNN_START_OFFSET 	0x00
#define XIL_CNN_WEA0_OFFSET 	0x04
#define XIL_CNN_WEA1_OFFSET 	0x08
#define XIL_CNN_READY_OFFSET 	0x0c
#define XIL_CNN_R_0_OFFSET 		0x10
#define XIL_CNN_R_1_OFFSET 		0x14
#define XIL_CNN_R_2_OFFSET 		0x18
#define XIL_CNN_R_3_OFFSET 		0x1c
#define XIL_CNN_R_4_OFFSET 		0x20
#define XIL_CNN_R_5_OFFSET 		0x24
#define XIL_CNN_R_6_OFFSET 		0x28
#define XIL_CNN_R_7_OFFSET 		0x2c
#define XIL_CNN_R_8_OFFSET 		0x30
#define XIL_CNN_R_9_OFFSET 		0x34

#define DRIVER_NAME "cnn"
#define DEVICE_NAME "xilcnn"

MODULE_LICENSE("Dual BSD/GPL");

struct cnn_info {
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
static struct cnn_info *tp = NULL;
static struct cnn_info *bp = NULL;

ssize_t cnn_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset); 
static ssize_t cnn_write(struct file *pfile,const  char __user *buffer, size_t length, loff_t *offset);
int cnn_open(struct inode *pinode, struct file *pfile);
int cnn_close(struct inode *pinode, struct file *pfile);

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = cnn_open,
	.read = cnn_read,
	.write = cnn_write,
	.release = cnn_close,
};

static struct of_device_id cnn_of_match[] = {
	{ .compatible = "xlnx,cnn", },
	{ .compatible = "xlnx,bram", },
	{ /* end of list */ },
};

static struct platform_driver cnn_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= cnn_of_match,
	},
	.probe		= cnn_probe,
	.remove		= cnn_remove,
};

MODULE_DEVICE_TABLE(of, cnn_of_match);

int cnn_open(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully opened file\n");
	return 0;
}

int cnn_close(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully closed file\n");
	return 0;
}

ssize_t cnn_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	return 0;
}

static ssize_t cnn_write(struct file *pfile,const  char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int len = 0;
	int br_c, pos;
	int val;
	int minor = MINOR(pfile->f_inode->i_rdev);
	
	len = copy_from_user(buff, buffer, length);
	if(len) {
		return -EFAULT;
	}


	printk(KERN_INFO "cnn_read Succesfully wrote into CNN device 1.\n");

	sscanf(buffer, "%d %d %d", br_c, pos, val);

	if(br_c < 32) {
		iowrite32(1 << br_c, tp->base_addr + XIL_CNN_WEA0_OFFSET);
	}
	else {
		iowrite32(1 << (br_c - 32), tp->base_addr + XIL_CNN_WEA1_OFFSET);
	}

	iowrite32(val, bp->base_addr + 4*pos);

	iowrite32(0, tp->base_addr + XIL_CNN_WEA0_OFFSET);
	iowrite32(0, tp->base_addr + XIL_CNN_WEA1_OFFSET);


	return length;
}

static int __init cnn_init(void) {
	int ret = 0;

	ret = alloc_chrdev_region(&my_dev_id, 0, 1, DRIVER_NAME);
	if (ret){
		printk(KERN_ERR "cnn_init: Failed to register char device\n");
		return ret;
	}
	printk(KERN_INFO "cnn_init: Char device region allocated\n");

	my_class = class_create(THIS_MODULE, "cnn_class");
	if (my_class == NULL){
		printk(KERN_ERR "cnn_init: Failed to create class\n");
		goto fail_0;
	}
	printk(KERN_INFO "cnn_init: Class created\n");

	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),0), NULL, "xlnx,cnn");
	if (my_device == NULL){
		printk(KERN_ERR "cnn_init: Failed to create device\n");
		goto fail_1;
	}
	printk(KERN_INFO "cnn_init: Device AXI created\n");

	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),1), NULL, "xlnx,bram");
	if (my_device == NULL){
		printk(KERN_ERR "cnn_init: Failed to create device\n");
		goto fail_2;
	}
	printk(KERN_INFO "cnn_init: Device BRAM created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 2);
	if (ret)
	{
		printk(KERN_ERR "cnn_init: Failed to add cdev\n");
		goto fail_3;
	}
	printk(KERN_INFO "cnn_init: Cdev added\n");
	printk(KERN_NOTICE "cnn_init: Hello world\n");

	return platform_driver_register(&cnn_driver);

fail_3:
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
fail_2:
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
fail_1:
	class_destroy(my_class);
fail_0:
	unregister_chrdev_region(my_dev_id, 1);
	return -1;
}

// ------------------------------------------
// EXIT
// ------------------------------------------
static void __exit cnn_exit(void) {
	platform_driver_unregister(&cnn_driver);
	cdev_del(my_cdev);
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1);
	printk(KERN_INFO "cnn_exit: Goodbye, cruel world\n");
}

module_init(cnn_init);
module_exit(cnn_exit);
