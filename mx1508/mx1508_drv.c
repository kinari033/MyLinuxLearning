#include <linux/module.h>
#include <linux/dlm_device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>

#include <asm/io.h>
#include <linux/delay.h>

#define MX1508_MAGIC 'M'
#define MOTOR_STOP    _IOW(MX1508_MAGIC, 0, int) // 待机
#define MOTOR_FORWARD _IOW(MX1508_MAGIC, 1, int) // 正转
#define MOTOR_BACK    _IOW(MX1508_MAGIC, 2, int) // 反转
#define MOTOR_BRAKE   _IOW(MX1508_MAGIC, 3, int) // 刹车
#define MOTOR_PWM_A   _IOW(MX1508_MAGIC, 4, int) // PWM_A
#define MOTOR_PWM_B   _IOW(MX1508_MAGIC, 5, int) // PWM_B


int err;

struct my_mx1508_drv_desc
{
    int major;
    struct class *mx1508_class;
    struct gpio_desc *gpio_desc_INA;
    struct gpio_desc *gpio_desc_INB;
};

struct my_mx1508_drv_desc *p_mx1508_desc;



static int mx1508_open(struct inode *node, struct file *file){

    return -EPERM; //返回"操作不允许"错误码
}
static ssize_t mx1508_read(struct file *file, char __user *buf, size_t size, loff_t *offset){

    return -EPERM; //返回"操作不允许"错误码
}
static ssize_t mx1508_write(struct file *file, const char __user *buf, size_t size, loff_t *offset){

    return -EPERM; //返回"操作不允许"错误码
}


long mx1508_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    switch (cmd){
        case MOTOR_FORWARD:
            // 正转：INA=H, INB=L [cite: 203, 519]
            gpiod_set_value(p_mx1508_desc->gpio_desc_INA, 1);
            gpiod_set_value(p_mx1508_desc->gpio_desc_INB, 0);
            break;
        case MOTOR_BACK:
            // 反转：INA=L, INB=H [cite: 203, 520]
            gpiod_set_value(p_mx1508_desc->gpio_desc_INA, 0);
            gpiod_set_value(p_mx1508_desc->gpio_desc_INB, 1);
            break;
        case MOTOR_BRAKE:
            // 刹车：INA=H, INB=H [cite: 203, 521]
            gpiod_set_value(p_mx1508_desc->gpio_desc_INA, 1);
            gpiod_set_value(p_mx1508_desc->gpio_desc_INB, 1);
            break;
        case MOTOR_PWM_A:
            // PWM A：INA=H, INB=H 
            /*当输入信号 INAx 为 PWM 信号，INBx=0 或者 INAx=0，INBx 为 PWM 信号时，马达的转动速度将受 PWM
            信号占空比的控制。*/

            break;  
        case MOTOR_PWM_B:
            // PWM A：INA=H, INB=H 
            /*当输入信号 INAx 为 PWM 信号，INBx=0 或者 INAx=0，INBx 为 PWM 信号时，马达的转动速度将受 PWM
            信号占空比的控制。*/
            
            break;                   
        case MOTOR_STOP:
        default:
            // 待机：INA=L, INB=L [cite: 203, 517]
            gpiod_set_value(p_mx1508_desc->gpio_desc_INA, 0);
            gpiod_set_value(p_mx1508_desc->gpio_desc_INB, 0);
            break;
    }
    return 0;


}

const struct file_operations mx1508_fopr = {
    .owner  = THIS_MODULE, 
    .unlocked_ioctl  = mx1508_ioctl, 
    .open   = mx1508_open,   /*无操作*/
    .write  = mx1508_write, /*无操作*/
    .read   = mx1508_read, /*无操作*/

};

int mx1508_drv_probe(struct platform_device *pdev){
    printk("%s, %s, in %d: \n", __FILE__, __FUNCTION__, __LINE__);
    p_mx1508_desc = devm_kzalloc(&pdev->dev, sizeof(struct my_mx1508_drv_desc), GFP_KERNEL);
    if (!p_mx1508_desc){
        printk("Failed to allocate memory \n");
        return -ENOMEM;
    }
    else{
        printk("memory allcated at : %p\n", p_mx1508_desc);
    }

    /*init gpio INA INB as out low*/
    p_mx1508_desc->gpio_desc_INA = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if(IS_ERR(p_mx1508_desc->gpio_desc_INA)){
        printk("Failed to get gpio_desc_INA \n");
        return PTR_ERR(p_mx1508_desc->gpio_desc_INA);
    }
    else{
        printk("get gpio_desc_INA successed!\n");
    }

    p_mx1508_desc->gpio_desc_INB = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if(IS_ERR(p_mx1508_desc->gpio_desc_INB)){
        printk("Failed to get gpio_desc_INA \n");
        return PTR_ERR(p_mx1508_desc->gpio_desc_INB);
    }
    else{
        printk("get gpio_desc_INB successed!\n");
    }

    p_mx1508_desc->major = register_chrdev(0, "my_mx1508_drv", &mx1508_fopr);
    p_mx1508_desc->mx1508_class = class_create(THIS_MODULE,"my_mx1508_class");
    err = PTR_ERR(p_mx1508_desc->mx1508_class);
    if(IS_ERR(p_mx1508_desc->mx1508_class)){
        unregister_chrdev(p_mx1508_desc->major, "my_mx1508_drv");
        return -1;
    }

    device_create(p_mx1508_desc->mx1508_class, NULL, MKDEV(p_mx1508_desc->major, 0), NULL, "my_mx1508_drv");

    return 0;
}
int mx1508_drv_remove(struct platform_device *pdev){
    printk("%s, %s, in %d: \n", __FILE__, __FUNCTION__, __LINE__);
    device_destroy(p_mx1508_desc->mx1508_class, MKDEV(p_mx1508_desc->major, 0));
    
    class_destroy(p_mx1508_desc->mx1508_class);

    unregister_chrdev(p_mx1508_desc->major, "my_mx1508_drv");;
    return 0;
}


struct platform_device_id mx1508_drv_id_tables[] = {
    {"mx1508_drv_1", 1},
    {"mx1508_drv_2", 2},
    {"mx1508_drv_3", 3},
};

const struct of_device_id mx1508_drv_ids[] = {
    {.compatible = "test,my_mx1508_drv"},
    {},    

};

struct platform_driver mx1508_drv_pdrv = {

    .probe = mx1508_drv_probe,
    .remove = mx1508_drv_remove,
    .driver = {
		.name = "mx1508_drv",
        .of_match_table = mx1508_drv_ids,
	},
    .id_table = mx1508_drv_id_tables,
};

static int __init mx1508_drv_init(void){
    
    printk("%s, %s, in %d: \n", __FILE__, __FUNCTION__, __LINE__);
    err = platform_driver_register(&mx1508_drv_pdrv);
    if(err < 0){
        printk("Failed to register mx1508 platform driver \n");
        return err;
    }
    return 0;
}

static void __exit mx1508_drv_exit(void){
    printk("%s, %s, in %d: \n", __FILE__, __FUNCTION__, __LINE__);
    platform_driver_unregister(&mx1508_drv_pdrv);
}

module_init(mx1508_drv_init);
module_exit(mx1508_drv_exit);
MODULE_LICENSE("GPL");

