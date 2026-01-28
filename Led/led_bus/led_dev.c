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

#include "led_res.h"

static void led_dev_release(struct device *dev){
    
}


static struct resource resources[] ={
    {
        .start  = GROUP_PIN(3,1),
        .flags   = IORESOURCE_IRQ, 
        .name   = "led_pin", 
    },
    {
        .start  = GROUP_PIN(5,8),
        .flags   = IORESOURCE_IRQ, 
        .name   = "led_pin",         
    }
};


static struct platform_device board_A_led_dev = {

    .name = "led_pin",
    .num_resources = ARRAY_SIZE(resources),
    .resource = resources,
    .dev = {
        .release = led_dev_release,
    }
};


static int __init led_dev_init(void){
    int err; 
    printk("%s %s line %d \n", __FILE__, __FUNCTION__, __LINE__);

    err = platform_device_register(&board_A_led_dev);

    return 0 ;
}

static void __exit led_dev_exit(void){

    printk("%s %s in line %d \n", __FILE__, __FUNCTION__, __LINE__);

    platform_device_unregister(&board_A_led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");




