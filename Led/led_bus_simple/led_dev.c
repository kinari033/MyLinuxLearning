#include <linux/module.h>
#include <linux/device.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/fs.h>



struct resource resources[] = {
    {
        .start = (3<<8)|(1),
        .flags = IORESOURCE_IRQ,
    },

};

static void led_dev_release(struct device *dev){

    printk("%s%s in line%d \n", __FILE__, __FUNCTION__, __LINE__);
}

static struct platform_device led_dev_plt = {

    .name = "led",
    .num_resources = ARRAY_SIZE(resources),
    .dev = {
        .release = led_dev_release,
    },
};


static int __init led_dev_init(void){
    int err;
    err = platform_device_register(&led_dev_plt);
    return err;


}

static void __exit led_dev_exit(void){

    printk("%s%s in line %d \n", __FILE__, __FUNCTION__, __LINE__);
    platform_device_unregister(&led_dev_plt);


}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");