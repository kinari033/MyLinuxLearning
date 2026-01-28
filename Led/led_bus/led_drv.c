#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <asm/io.h>

#include "led_opr.h"

static int major = 0;
static struct class *led_class;
struct led_operation *p_led_opr;

#define MIN(a,b) (a < b ? a : b)

void led_class_create_device(int minor){
    device_create(led_class, NULL, MKDEV(major,minor), NULL, "led_pin%d", minor);

}
void led_class_destroy_device(int minor){
    device_destroy(led_class, MKDEV(major, minor));
    
}
void register_led_operation(struct led_operation *opr){
    p_led_opr = opr; 

}

EXPORT_SYMBOL(led_class_create_device);
EXPORT_SYMBOL(led_class_destroy_device);
EXPORT_SYMBOL(register_led_operation);


static ssize_t led_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset){
    printk("%s %s line %d \n", __FILE__, __FUNCTION__, __LINE__);
    return 0;


}
static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset){
    int err;
    char status;
    struct inode *inode = file_inode(file);
    int minor = iminor(inode);
    printk("%s %s line %d \n", __FILE__, __FUNCTION__, __LINE__);
    
    err = copy_from_user(&status, buf, 1);
    p_led_opr ->  ctl(minor, status );
    return 1;//sizeof(status)


}


static int led_drv_open(struct inode *node, struct file *file){
    int minor = iminor(node);

    printk("%s %s line %d \n", __FILE__, __FUNCTION__, __LINE__);

    p_led_opr->init(minor);

    return 0;


}
static int led_drv_close(struct inode *node, struct file *file){
    printk("%s %s line %d \n", __FILE__, __FUNCTION__, __LINE__);
    return 0;

}

static struct file_operations led_drv_fopr = {
    .owner  = THIS_MODULE,
    .open   = led_drv_open,
    .write  = led_drv_write,
    .read   = led_drv_read,
    .release= led_drv_close, 
};

static int __init led_drv_init(void){
    int err; 
    printk("%s %s line %d \n", __FILE__, __FUNCTION__, __LINE__);
    major = register_chrdev(major, "led_drv", &led_drv_fopr);/* /dev/led */

    led_class = class_create(THIS_MODULE, "led_class"); //  sys/led_class
    
    err = PTR_ERR(led_class);

    if(IS_ERR(led_class)){

        printk("%s %s in line %d \n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, "led_drv");
        return -1;

    }
    return 0 ;
}

static void __exit led_drv_exit(void){

    printk("%s %s in line %d \n", __FILE__, __FUNCTION__, __LINE__);
    class_destroy(led_class);
    unregister_chrdev(major , "led_drv");
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");




