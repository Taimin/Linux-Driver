KERN_DIR = /work/linux_kernel/GT2440/linux-2.6.38.6

all:
	make -C $(KERN_DIR) M=`pwd` modules
	
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	
obj-m += Second_drv.o