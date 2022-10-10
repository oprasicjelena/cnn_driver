// ------------------------------------------
// LINUX DEVICE DRIVER
// by g0-2021
// ------------------------------------------

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
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

#define BUFF_SIZE 100

// ------------------------------------------
// INFO FOR OS
// ------------------------------------------
#define DRIVER_NAME "cnn"
#define DEVICE_NAME "xilcnn"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR ("g0-2021");
MODULE_DESCRIPTION("Driver Zynq PL Conv Neural Network.");
MODULE_ALIAS("custom:cnn");

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

// ------------------------------------------
// VARIABLES
// ------------------------------------------
unsigned char state = 0;
int ready = 0;
int device_fsm = 0;

// ------------------------------------------
// DRIVER AND OS FUNCTIONS
// ------------------------------------------
static int cnn_probe(struct platform_device *pdev);
static int cnn_remove(struct platform_device *pdev);
int cnn_open(struct inode *pinode, struct file *pfile);
int cnn_close(struct inode *pinode, struct file *pfile);
ssize_t cnn_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t cnn_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

static int __init cnn_init(void);
static void __exit cnn_exit(void);

// ------------------------------------------
// INFO FOR DRIVER AND OS
// ------------------------------------------
struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = cnn_open,
	.read = cnn_read,
	.write = cnn_write,
	.release = cnn_close,
};

static struct of_device_id cnn_of_match[] = {
	{ .compatible = "xlnx,ip-1.0", },
	{ .compatible = "xlnx,axi-bram-ctrl-4.1", },
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


static int cnn_probe(struct platform_device *pdev) {
	struct resource *r_mem;
	int rc = 0;

	// Get phisical register adress space from device tree
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		printk(KERN_ALERT "cnn_probe: Failed to get reg resource\n");
		return -ENODEV;
	}

	switch (device_fsm)	{
	case 0:
		// Get memory for structure cnn_info
		tp = (struct cnn_info *) kmalloc(sizeof(struct cnn_info), GFP_KERNEL);
		if (!tp) {
			printk(KERN_ALERT "cnn_probe: Could not allocate CNN device\n");
			return -ENOMEM;
		}

		// Put phisical adresses in cnn_info structure
		tp->mem_start = r_mem->start;
		tp->mem_end = r_mem->end;

		// Reserve that memory space for this driver
		if (!request_mem_region(tp->mem_start,tp->mem_end - tp->mem_start + 1,	DEVICE_NAME)) {
			printk(KERN_ALERT "cnn_probe: Could not lock memory region at %p\n",(void *)tp->mem_start);
			rc = -EBUSY;
			goto error1;
		}

		// Remap phisical to virtual adresses
		tp->base_addr = ioremap(tp->mem_start, tp->mem_end - tp->mem_start + 1);
		if (!tp->base_addr) {
			printk(KERN_ALERT "cnn_probe: Could not allocate memory\n");
			rc = -EIO;
			goto error2;
		}
		++device_fsm;
		printk(KERN_NOTICE "cnn_probe: CNN platform driver registered\n");
		return 0;//ALL OK
	error2:
		release_mem_region(tp->mem_start, tp->mem_end - tp->mem_start + 1);
		kfree(tp);
	error1:
		return rc;
		break;
	case 1:
		// Get memory for structure cnn_info
		bp = (struct cnn_info *) kmalloc(sizeof(struct cnn_info), GFP_KERNEL);
		if (!bp) {
			printk(KERN_ALERT "cnn_probe: Could not allocate CNN device\n");
			return -ENOMEM;
		}

		// Put phisical adresses in cnn_info structure
		bp->mem_start = r_mem->start;
		bp->mem_end = r_mem->end;

		// Reserve that memory space for this driver
		if (!request_mem_region(bp->mem_start,bp->mem_end - bp->mem_start + 1,	DEVICE_NAME)) {
			printk(KERN_ALERT "cnn_probe: Could not lock memory region at %p\n",(void *)bp->mem_start);
			rc = -EBUSY;
			goto error3;
		}

		// Remap phisical to virtual adresses
		bp->base_addr = ioremap(bp->mem_start, bp->mem_end - bp->mem_start + 1);
		if (!bp->base_addr) {
			printk(KERN_ALERT "cnn_probe: Could not allocate memory\n");
			rc = -EIO;
			goto error4;
		}
		++device_fsm;
		printk(KERN_NOTICE "cnn_probe: CNN platform driver registered\n");
		return 0;//ALL OK
	error4:
		release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
		kfree(bp);
	error3:
		return rc;
		break;

	default:
		printk(KERN_INFO "cnn_probe: Device FSM in illegal state. \n");
		return -1;
		break;
	}
}

static int cnn_remove(struct platform_device *pdev) {
	switch (device_fsm)	{
	case 0:
		iounmap(tp->base_addr);
    	iowrite32(0, tp->base_addr);
		release_mem_region(tp->mem_start, tp->mem_end - tp->mem_start + 1);
		kfree(tp);
		printk(KERN_WARNING "cnn_remove: CNN driver removed\n");
		return 0;
		break;
	case 1:
		iounmap(bp->base_addr);
    	iowrite32(0, bp->base_addr);
		release_mem_region(bp->mem_start, bp->mem_end - bp->mem_start + 1);
		kfree(bp);
		printk(KERN_WARNING "cnn_remove: CNN driver removed\n");
		return 0;
		break;
	default:
    	printk(KERN_INFO "cnn_remove Device FSM in illegal state. \n");
    	return -1;
		break;
	}
}

int cnn_open(struct inode *pinode, struct file *pfile) {
	printk(KERN_INFO "Succesfully opened CNN\n");
	return 0;
}
int cnn_close(struct inode *pinode, struct file *pfile) {
	printk(KERN_INFO "Succesfully closed CNN\n");
	return 0;
}


ssize_t cnn_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
	char buf[BUFF_SIZE];
  long int r[10];
  int i = 0;
  int len;

  ready = ioread32(tp->base_addr + XIL_CNN_READY_OFFSET);
  if (ready) 
  {
    printk(KERN_INFO "cnn_write: results ready\n");

    for (i = 0; i < 10; i++)
    {
      r[i] = ioread32(tp->base_addr + XIL_CNN_R_0_OFFSET + 4*i);
    }

    len = scnprintf(buf, BUFF_SIZE, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9]);

    printk("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9]);

     if (copy_to_user(buffer, buf, len))
      return -EFAULT;
    }
    else
    {
      printk(KERN_INFO "cnn_write: results not ready\n");
    }

	return 0;
}

ssize_t cnn_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {

	 char buff[BUFF_SIZE];
    int len = 0;
    int br_c, pos;
    int val;
    int minor = MINOR(pfile->f_inode->i_rdev);

    len = copy_from_user(buff, buffer, length);
    if(len) {
      return -EFAULT;
    }
    
    buff[length]='\0';
                
    switch (minor) {
    case 0:
          printk(KERN_INFO "cnn_read Succesfully wrote into CNN device /dev/xlnx,ip-1.0\n");
          iowrite32(1, tp->base_addr + XIL_CNN_START_OFFSET);
          break;
    case 1:
          printk(KERN_INFO "cnn_read Succesfully wrote into CNN device xlnx,axi-bram-ctrl-4.1.\n");
          printk(KERN_INFO "length is: %d\n", length);

          sscanf(buff, "%d %d %d", &br_c, &pos, &val);

          if(br_c < 32) {
            iowrite32(1 << br_c, tp->base_addr + XIL_CNN_WEA0_OFFSET);
          }
          else {
            iowrite32(1 << (br_c - 32), tp->base_addr + XIL_CNN_WEA1_OFFSET);
          }
          int ret = 0;
          iowrite32(val, bp->base_addr + 4*pos);
          ret = ioread32(bp->base_addr + 4*pos);

          printk("ret is %d\n", ret);  
          iowrite32(0, tp->base_addr + XIL_CNN_WEA0_OFFSET);
          iowrite32(0, tp->base_addr + XIL_CNN_WEA1_OFFSET);
          
          printk(KERN_INFO "%d %d %d", br_c, pos, val);
          break;

  default:
          printk(KERN_ERR "cnn_read Invalid minor. \n");
          break;
  }

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

	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),0), NULL, "xlnx,ip-1.0");
	if (my_device == NULL){
		printk(KERN_ERR "cnn_init: Failed to create device\n");
		goto fail_1;
	}
	printk(KERN_INFO "cnn_init: Device AXI created\n");

	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),1), NULL, "xlnx,axi-bram-ctrl-4.1");
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