#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define MIN(a, b) (a < b ? a:b) 
static int major = 0;
static int kernel_val = 123;
static struct class *class_for_hello;

static ssize_t hello_read (struct file *file, char __user *buf, size_t size, loff_t *offset){
    // int err;
    int copyTo_bytes;
    printk("%s, %s, line%d \n", __FILE__, __FUNCTION__, __LINE__);
    copyTo_bytes = copy_to_user(buf, &kernel_val, 4);
    if (copyTo_bytes != 0){
        printk(KERN_ERR"copy to user failed \n");
        return -EFAULT;
    }
    return 4;
}

static ssize_t hello_write (struct file *file, const char __user *buf, size_t size , loff_t *offset){
    int copyForm_byres;
    printk("%s, %s , line%d \n", __FILE__, __FUNCTION__, __LINE__);
    copyForm_byres = copy_from_user(&kernel_val , buf, 4);
    if (copyForm_byres != 0){
        printk(KERN_ERR"copy to user failed \n");
        return -EFAULT;
    }
    return 4;
}

static struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .read = hello_read,
    .write = hello_write,
};

int __init hello_init(void){
    printk("hello drv init");
    major = register_chrdev(0, "hello_drv", &hello_fops);
    class_for_hello = class_create(THIS_MODULE,"hello_class");//  /sys/class/hello_class/
    device_create(class_for_hello, NULL, MKDEV(major, 0), NULL, "hello_device");
    return 0;
}

void __exit hello_exit(void){
    printk("hello drv exit\n");

    device_destroy(class_for_hello, MKDEV(major, 0));

    class_destroy(class_for_hello);

    unregister_chrdev(major, "hello_drv");
    

}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");