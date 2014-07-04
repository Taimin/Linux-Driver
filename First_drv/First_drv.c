#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
//#include <linux/kdev_t.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
//#include <asm/arch/regs-gpio.h>
//#include <asm/hardware.h>

static int major;
static struct class * firstdrv_class;
static struct device *firstdrv_class_devs;

static volatile unsigned long *gpbcon = NULL;
static volatile unsigned long *gpbdat = NULL;

int first_drv_open (struct inode * inode, struct file * file)
{
	//printk("first_drv_open\n");
	/*configure GPB5678 into output mode*/
	*gpbcon &= ~((0x3<<(8*2))|(0x3<<(7*2))|(0x3<<(5*2))|(0x3<<(6*2)));
	*gpbcon |= ((0x1<<(8*2))|(0x1<<(7*2))|(0x1<<(5*2))|(0x1<<(6*2)));	
	return 0;
}

static ssize_t first_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int val;
	
	copy_from_user(&val, buf, count);//transfer data from user space to kernel

	if(val == 1)
	{
		//open the led
		
		*gpbdat &= ~((1<<5)|(1<<6)|(1<<7)|(1<<8));
	}
	else
	{
		//close the led
		*gpbdat |= ((1<<5)|(1<<6)|(1<<7)|(1<<8));
	}
	//copy_to_user()//from kernel to user space
	//printk("first_drv_write\n");
	
	return 0;
}



static struct file_operations first_drv_fops = {
	.owner = THIS_MODULE,
	.open = first_drv_open,
	.write = first_drv_write,
};

int first_drv_init(void)
{
	major = register_chrdev(0, "first_drv", &first_drv_fops);

	firstdrv_class = class_create(THIS_MODULE, "firstdrv");
	if(IS_ERR(firstdrv_class))
		return PTR_ERR(firstdrv_class);

	firstdrv_class_devs = device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz");
	if(unlikely(IS_ERR(firstdrv_class_devs)))
		return PTR_ERR(firstdrv_class_devs);

	gpbcon = (volatile unsigned long *)ioremap(0x56000010,16);
	gpbdat = gpbcon + 1;
	
	return 0;
}

void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv");
	device_unregister(firstdrv_class_devs);
	class_destroy(firstdrv_class);
	iounmap(gpbcon);
}


module_init(first_drv_init);
module_exit(first_drv_exit);

MODULE_LICENSE("GPL");

