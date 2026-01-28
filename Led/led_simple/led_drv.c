#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/io.h>



static int major = 0;
static struct class *led_class;
//IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 address: 0x02290000 + 0x14
static volatile unsigned long *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;//pointer
//GPIO5_GDIR address: 0x020AC004
static volatile unsigned int *GPIO5_GDIR;
//GPIO5_DR address: 0x020AC000
static volatile unsigned int *GPIO5_DR;

static ssize_t led_write(struct file *file, const char __user *buf, size_t size, loff_t *offset){
    int ret;
    char val;

    ret = copy_from_user(&val, buf, 1);

    //set gpio: 1/0
    if (val){
        *GPIO5_DR &= ~(1<<3);
    }
    else{
        *GPIO5_DR |= (1<<3);
    }
    return 1;

   
}

int led_open(struct inode *node, struct file *file){
    /**
     * enable GPIO5_3
     * config GPIO5_3 as gpio
     * config GPIO5_3 as out
     * 
    */
   *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 &= ~0xf;
   *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 |= 0x5;
   *GPIO5_GDIR |= (1<<3);

    return 0;
}

static struct file_operations led_file_operations= {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,

};

//init

static int __init led_init(void){

    printk("%s, %s, line = %d",__FILE__,__FUNCTION__,__LINE__);
    //register char device
    major = register_chrdev(major, "led_drv", &led_file_operations);

    /*ioremap*/

    //phy address -> virtual address
    IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x02290000 + 0x14, 4); 

    GPIO5_GDIR = ioremap(0x020AC004, 4);

    GPIO5_DR = ioremap(0x020AC000, 4);

    //class
    led_class = class_create(THIS_MODULE, "led_class");

    //device
    device_create(led_class, NULL, MKDEV(major, 0), NULL, "led_device");

    return 0;
}

static void __exit led_exit(void){

    iounmap(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3);
    iounmap(GPIO5_GDIR);
    iounmap(GPIO5_DR);


    //reverse
    device_destroy(led_class, MKDEV(major, 0));

    class_destroy(led_class);

    unregister_chrdev(major, "led_drv");


}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");