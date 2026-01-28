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
#include "led_res.h"
#include "led_drv.h"

static int g_ledpins[100];
static int g_ledcnt = 0;

static int board_demo_led_init(int which){
    printk("init gpio: group %d , pin %d\n", GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    switch(GROUP(g_ledpins[which])){
        case 0 :
        {
            printk("init pin of group 0\n");
            break;
        }

        case 1 :
        {
            printk("init pin of group 1\n");
            break;
        }
        case 2 :
        {
            printk("init pin of group 2\n");
            break;
        }
        case 3 :
        {
            printk("init pin of group 3\n");
            break;
        }

    }
    return 0;
}

static int board_demo_led_ctl(int which, char status){
    printk("set led %s: group %d, pin %d \n",status ? "on":"off", GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    switch(GROUP(g_ledpins[which])){
        case 0 :
        {
            printk("set pin of group 0\n");
            break;
        }

        case 1 :
        {
            printk("set pin of group 1\n");
            break;
        }
        case 2 :
        {
            printk("set pin of group 2\n");
            break;
        }
        case 3 :
        {
            printk("set pin of group 3\n");
            break;
        }

    }   
    return 0;
}

struct led_operation board_demo_led_opr ={
    .init = board_demo_led_init,
    .ctl  = board_demo_led_ctl,
};

struct led_operation *get_board_led_opr(void){
    return &board_demo_led_opr;
}

//if match -> probe func
static int chip_demo_gpio_probe(struct platform_device *pdev){
    struct resource *res;
    int i = 0;
    while(1){
        res = platform_get_resource(pdev, IORESOURCE_IRQ , i++);

        if(!res)
            break;

        g_ledpins[g_ledcnt] = res->start ; 
        
        g_ledcnt++;
    }

    return 0;
}

static int chip_demo_gpio_remove(struct platform_device *pdev){

    struct resource *res;
    int i = 0;

    while (1)
    {
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
        if (!res){
            break;
        }

        led_class_destroy_device(i);
        i++;
        g_ledcnt--;
    }

    return 0;
    
}


static struct platform_driver chip_demo_gpio_driver = {
    .probe      = chip_demo_gpio_probe,
    .remove     = chip_demo_gpio_remove,
    .driver     = {
        .name   = "led_pin",
    },
};

static int __init chip_demo_gpio_drv_init(void){

    int err;

    err = platform_driver_register(&chip_demo_gpio_driver);
    register_led_operation(&board_demo_led_opr);

    return 0 ;
}

static void __exit chip_demo_gpio_drv_exit(void){
    platform_driver_unregister(&chip_demo_gpio_driver);
}

module_init(chip_demo_gpio_drv_init);
module_exit(chip_demo_gpio_drv_exit);
MODULE_LICENSE("GPL");

