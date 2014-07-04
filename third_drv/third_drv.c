#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/io.h>

static int major = 0;
static struct class *thirddrv_class = NULL;
static struct device *thirddrv_dev = NULL;

static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;

static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

static volatile  int ev_press = 0;



struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

/*
press: 0x01, 0x02, 0x03, 0x04
release: 0x81, 0x82, 0x83, 0x84
*/

static unsigned char key_val;

static struct pin_desc pins_desc[4] = {
	{S3C2410_GPF(0), 0x01},
	{S3C2410_GPF(2), 0x02},
	{S3C2410_GPG(3), 0x03},
	{S3C2410_GPG(11), 0x04},
};

/*
make sure the value of the button
*/

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;

	
//read the values of pins
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{
		/*released*/
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/*pressed*/
		key_val =  pindesc->key_val;
		
	}

	ev_press = 1;
	wake_up_interruptible(&button_waitq);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int third_drv_open(struct inode *inode, struct file *file)
{
	/*configure GPF0, 2 into input pin*/
	request_irq(IRQ_EINT0, buttons_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "K4", &pins_desc[0]);
	request_irq(IRQ_EINT2, buttons_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "K3", &pins_desc[1]);

	/*configure GPG3, 11 into input pin*/
	request_irq(IRQ_EINT11, buttons_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "K2", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "K1", &pins_desc[3]);

	return 0;
}

ssize_t third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{

	if(size != 1)
	{
		return -EINVAL;
	}

	/*if no button is pressed, rest*/
	wait_event_interruptible(button_waitq, ev_press);

	/*if the button is pressed, running*/

	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

static int third_drv_release(struct inode * inode, struct file * file)
{
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	return 0;
}

static struct file_operations third_drv_fops = {
	.owner = THIS_MODULE,
	.open = third_drv_open,
	.read = third_drv_read,
	.release = third_drv_release,
};

static int third_drv_init(void)
{
	major = register_chrdev(0, "third_drv", &third_drv_fops);
	thirddrv_class = class_create(THIS_MODULE, "third_drv");
	thirddrv_dev = device_create(thirddrv_class, NULL, MKDEV(major, 0), NULL, "yang");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");
	device_unregister(thirddrv_dev);
	class_destroy(thirddrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
}

module_init(third_drv_init);
module_exit(third_drv_exit);

MODULE_LICENSE("GPL");
