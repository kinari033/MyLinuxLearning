#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/uaccess.h>

static int major = 0;
static struct class *led_drv_class;
int g_ledcnt = 0;

struct led_desc {
    int pin;
    int minor;

};
static struct led_desc led_desc[18];


static int led_drv_open(struct inode *node, struct file *file){
    int minor = iminor(node);

    printk("%s %s in line %d \n", __FILE__, __FUNCTION__, __LINE__);
    printk("init led pin 0x%x as output\n", led_desc[minor].pin);
    return 0;

}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset){
    char status;
    struct inode *inode = file_inode(file);
    int minor = iminor(inode);
    int err;
    err = copy_from_user(&status, buf, 1);

    printk("set pin 0x%x as %d \n",led_desc[minor].pin, status);
    return 1;
}

static struct file_operations led_drv_fopr ={
    .owner = THIS_MODULE,
    .open = led_drv_open,
    .write = led_drv_write,
};



static int led_drv_probe(struct platform_device *pdev){
    printk("%s %s in line %d\n", __FILE__, __FUNCTION__ , __LINE__);
    struct resource *res;
    int i = 0;
    res = platform_get_resource(pdev, IORESOURCE_IRQ, i++);

    if(!res) return -EINVAL;

    // record pin

    int minor;
    minor = g_ledcnt;
    led_desc[minor].pin = res -> start;
    led_desc[minor].minor = minor;

    device_create(led_drv_class, NULL, MKDEV(major, minor), NULL, "led%d", minor);  // /dev/led0 ...
    platform_set_drvdata(pdev, &led_desc[minor]);
    g_ledcnt++;
    
    return 0;
}


//platform remove function 
static int led_drv_remove(struct platform_device *pdev){

    struct led_desc *led = platform_get_drvdata(pdev);
    device_destroy (led_drv_class, MKDEV(major, led->minor));

    return 0;
}

static const struct platform_device_id led_id_table[] = {
    {"led", 1},
    {"led_2", 2},
    {"led_3", 3},
    { },
};

static struct platform_driver led_drv_plt = {
    .probe = led_drv_probe,
    .remove = led_drv_remove,
	.driver = {
		.name = "led",
	},
    .id_table = led_id_table,
};

static int __init led_drv_init(void){

    printk("%s %s in line %d\n", __FILE__, __FUNCTION__ , __LINE__);
    major = register_chrdev(major, "led_drv", &led_drv_fopr);
    led_drv_class = class_create(THIS_MODULE, "led_drv_class");

    // assitasnce
    int err;
    err = PTR_ERR(led_drv_class);
    if (!err){
        printk("%s %s in line %d\n", __FILE__, __FUNCTION__ , __LINE__);
        
        unregister_chrdev(major, "led_drv");
        return -1;
    }

    err = platform_driver_register(&led_drv_plt);

    return 0;
}

static void __exit led_drv_exit(void){


}


module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
