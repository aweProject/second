#include<linux/module.h>
#include<linux/types.h>
#include<linux/fs.h>
#include<linux/errno.h>
#include<linux/mm.h>
#include<linux/sched.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<asm/io.h>
#include<asm/system.h>
#include<asm/uaccess.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/jiffies.h>
#include<linux/time.h>

#define SECOND_MAJOR 248

static int second_major = SECOND_MAJOR;

struct second_dev  {
	struct cdev cdev;
	atomic_t counter;
	struct timer_list s_timer;
};

struct second_dev *second_devp;

static void second_timer_handle(unsigned long arg)
{
	mod_timer(&second_devp->s_timer, jiffies + HZ);
	atomic_inc(&second_devp->counter);
	
	printk(KERN_NOTICE "current jiffies is %ld\n",jiffies);
}

int second_open(struct inode *inode , struct file *filp)
{
	init_timer(&second_devp->s_timer);
	second_devp->s_timer.function = &second_timer_handle;
	
	second_devp->s_timer.expires = jiffies + HZ;
	
	add_timer(&second_devp->s_timer);
	
	atomic_set(&second_devp->counter , 0);

	return 0;
}

int second_release(struct inode *inode , struct file *filp)
{
	del_timer(&second_devp->s_timer);

	return 0;
}

static ssize_t second_read(struct file *filp ,char __user *buf ,size_t count, loff_t *ppos)
{
	int counter ;
	
	counter = atomic_read(second_devp);
	
	if(put_user(counter, (int*) buf))
		return -EFAULT;
	else 
		return sizeof(unsigned int );
}

static const struct file_operations second_fops = {
	.owner = THIS_MODULE,
	.open = second_open,
	.release = second_release,
	.read = second_read,
};
	
static void second_setup_cdev(struct second_dev *dev , int index)
{
	int err, devno = MKDEV(second_major ,index);
	
	cdev_init(&dev->cdev , &second_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno ,1);
	if(err)
		printk(KERN_NOTICE "Error %d adding LED %d",err, index);
}

int __init second_init(void)
{
	int ret;
	dev_t devno = MKDEV(second_major , 0);
	
	if(second_major)
		ret = register_chrdev_region(devno,1,"second");
	else {
		ret = alloc_chrdev_region(&devno, 0 ,1 ,"second");
	}
	
	if(ret < 0)
		return ret;
	
	second_devp = (struct second_dev *) kmalloc(sizeof(struct second_dev), GFP_KERNEL);

	if(!second_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}
	
	memset(second_devp , 0 , sizeof(struct second_dev));
	
	second_setup_cdev(second_devp, 0);
	
	printk(KERN_INFO "driver loading %s",__func__);
	return 0;

fail_malloc:
	unregister_chrdev_region(devno,1);
	return ret;
}

void __exit second_exit(void)
{
	cdev_del(&second_devp->cdev);
	kfree(second_devp);
	
	unregister_chrdev_region(MKDEV(second_major,0),1);
	printk(KERN_INFO "driver exting %s",__func__);
}

MODULE_LICENSE("Dual BSD/GPL");
module_param(second_major,int, S_IRUGO);

module_init(second_init);
module_exit(second_exit);
