#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>

static int major = 0;
static struct class *seconddrv_class = NULL;
static struct device *seconddrv_dev = NULL;

static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;

static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;


static int second_drv_open(struct inode *inode, struct file *file)
{
	/*configure GPF0, 2 into input pin*/
	*gpfcon &= ~((0x3<<(0*2)) | (0x3<<(2*2)));
	
	/*configure GPG3, 11 into input pin*/
	*gpgcon &= ~((0x3<<(3*2)) | (0x3<<(11*2)));
	
	return 0;
}

static ssize_t second_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/*return the voltage of four pins*/
	unsigned char key_vals[4];
	int regval;

	if(size != sizeof(key_vals))
	{
		return -EINVAL;
	}
	
	/*read GPF0, 2*/
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;

	/*read GPG3, 11*/
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;

	copy_to_user(buf, key_vals, sizeof(key_vals));

	
	return sizeof(key_vals);
}

static struct file_operations second_drv_fops = {
	.owner = THIS_MODULE,
	.open = second_drv_open,
	.read = second_drv_read,
};

static int second_drv_init(void)
{
	major = register_chrdev(0, "second_drv", &second_drv_fops);
	seconddrv_class = class_create(THIS_MODULE, "second_drv");
	seconddrv_dev = device_create(seconddrv_class, NULL, MKDEV(major, 0), NULL, "yang");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

static void second_drv_exit(void)
{
	unregister_chrdev(major, "second_drv");
	device_unregister(seconddrv_dev);
	class_destroy(seconddrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
}

module_init(second_drv_init);
module_exit(second_drv_exit);

MODULE_LICENSE("GPL");

