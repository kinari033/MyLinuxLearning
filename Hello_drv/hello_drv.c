#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>

static int major = 0;
static char kernel_buf[1024];
static struct class *hello_class;

#define MIN(a,b) (a<b?a:b)

static ssize_t hello_drv_read (){

}



static struct file_operations hello_drv = {
    /* data */
    .owner  = THIS_MODULE;
    .read   = hello_read;
    .open   = hello_open;
    .release= hello_close;
    .write  = hello_write;
};


static int hello_init(void){
    printk("%s %s line %d",__FILE__, __FUNCTION__, __LINE__);

    int major = register_chrdev(0,"hello",&hello_drv);

    hello_class = class_create(THIS_MODULE,"hello");
    int err = PTR_ERR(hello_class);
    if (IS_ERR(hello_class)){
        unregister_chrdev(major,"hello");
        return -1;

    }

    device_create(hello_class,NULL,MKDEV(major,0),NULL,"hello");
    return 0;

}

static int hello_exit(void){
    printk("%s %s line %d",__FILE__,__FUNCTION__,__LINE__);
    device_destory(hello_class,MKDEV(major,0));
    class_destory(hello_class);
    unregister_chrdev(major,0);
    return 0;

}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");