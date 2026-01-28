
#include <linux/module.h>
#include <linux/dlm_device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/kernel.h>


#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/dlm_device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/errno.h>
#include <asm/io.h>


#include <linux/platform_device.h>
#include <linux/of_device.h>

// int g_ledpins[100];
// struct led_operation *p_led_opr;
// void register_led_opr(struct led_operation *opr){
//     p_led_opr = opr;
// }
// EXPORT_SYMBOL(g_ledpins);
// EXPORT_SYMBOL(register_led_opr);


/*led fd*/
#include <linux/module.h>
#include <linux/dlm_device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/errno.h>
#include <asm/io.h>


#include <linux/platform_device.h>
#include <linux/of_device.h>
struct led_desc {
    int pin;
    int minor;
};

static int major = 0;
static struct class *led_drv_class;
int g_ledcnt = 0;

static struct led_desc leds_desc[18];

struct gpio_desc *led_gpio;

/*char device operation*/
static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size , loff_t *offset){
    int err; 
    char status;
    struct inode *inode = file_inode(file);

    int minor = iminor(inode);

    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
    err = copy_from_user(&status, buf, 1);
    // p_led_opr -> ctl(minor, status);
    gpiod_set_value(led_gpio, status);
    printk("set led pin 0x%x as %d \n", leds_desc[minor].pin, status);

    return 1;

}

static int led_drv_open(struct inode *node , struct file *file){

    // int err;
    int minor = iminor(node);
    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);

    // p_led_opr -> init(minor);
    gpiod_direction_output(led_gpio, 0);
    printk("init led pin 0x%x as output", leds_desc[minor].pin);
    
    return 0;
}


static struct file_operations led_drv_fopr =
{
    .owner = THIS_MODULE,
    .open = led_drv_open, 
    .write = led_drv_write,
    
};


/*platform driver */

static int led_drv_probe(struct platform_device *pdev){
    int err;

    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);

    led_gpio = gpiod_get(&pdev->dev, "led", 0);
    if(IS_ERR(led_gpio)){
        dev_err(&pdev->dev, "Failed to get GPIO for led\n");
        return PTR_ERR(led_gpio);

    } 
    else{
        printk("get led_drv descriptor \n");
    }

    major = register_chrdev(major, "led_drv", &led_drv_fopr);
    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
    led_drv_class = class_create(THIS_MODULE, "led_drv_class");
    err = PTR_ERR(led_drv_class);

    if (IS_ERR(led_drv_class)){
        printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, "led_drv");
        return -1;
    }

    device_create(led_drv_class, NULL, MKDEV(major, 0), NULL, "led%d", 0);

    return 0 ;
}

static int led_drv_remove(struct platform_device *pdev){

    device_destroy(led_drv_class, MKDEV(major, 0));
    class_destroy(led_drv_class);
    unregister_chrdev(major, "led_drv");
    gpiod_put(led_gpio);
    return 0;

}


static const struct platform_device_id led_id_table[] = {
    {"led", 1},
    {"led2", 2},
    {"led3", 3},
    {},
};

static const struct of_device_id dts_devices_ids[] = {
    {.compatible = "test,led"},
    {},
};

static struct platform_driver led_drv = {
    .probe = led_drv_probe,
    .remove = led_drv_remove,
	.driver = {
		.name = "led_drv",
        .of_match_table = dts_devices_ids,
	},
    .id_table = led_id_table,
};


static int __init led_init(void){
    int err;

    err = platform_driver_register(&led_drv);
    if(err < 0){
        printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);
        printk("can not register led_drv platform drv \n");
    }    

    return 0;
}

static void __exit led_exit(void){
    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
    platform_driver_unregister(&led_drv);
    class_destroy(led_drv_class);
    unregister_chrdev(major,"led_drv");
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");