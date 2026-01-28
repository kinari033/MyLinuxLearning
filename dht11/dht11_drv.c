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

int ret;
int err;

struct my_dht11_dev {
    int major;
    struct class *dht11_drv_class;
    struct gpio_desc *dht11_gpio_desc;    
    int iminor;
    int pin;
};
struct my_dht11_dev *pt_myDht11_devs;

static int dht11_wait_for_level(int target_level, int timeout_us){

    while(timeout_us--){
        if(gpiod_get_value(pt_myDht11_devs->dht11_gpio_desc) == target_level){
            return 0;
        }
        udelay(1);
    }
    return -1;
}

static int dht11_byte_get(void){
    /*is recevice 8bit data from dht11*/
    int i;
    unsigned char data = 0;
    for (i = 0; i < 8 ; i++){

        if(dht11_wait_for_level(1,100)){
            printk("data wait for high pin timeout! \n");
            return -1;
        }
        udelay(40);
        data <<=1;//先移位，再存数据

        if(gpiod_get_value(pt_myDht11_devs->dht11_gpio_desc) == 1){
            data |= 1; //延时后高电平 置1 不置就为0
        }

        if(dht11_wait_for_level(0,100) != 0){
            printk("data wait for low pin err! \n");
            return -1;
        }        
    }
    return data;
}

ssize_t dht11_read(struct file *file, char __user *buf, size_t size, loff_t *offset){
    unsigned char data[5];
    int i;
    int val;
    int err;
    unsigned long irq_flags;

    /*dht11: start signal*/
    err = gpiod_direction_output(pt_myDht11_devs->dht11_gpio_desc, 0);
    if(err != 0){
        printk("Failed to set gpiod direction as output \n");
        return err;
    }
    else{
        mdelay(20);
        printk("send start signl to ght11 success! \n");
    }
    /*dht11: reset low 18ms -> high 30us*/
    gpiod_set_value(pt_myDht11_devs->dht11_gpio_desc, 1);
    udelay(40);
    err = gpiod_direction_input(pt_myDht11_devs->dht11_gpio_desc);
    if(err != 0){
        printk("Failed to set gpiod direction as input \n");
        return err;
    }
    // else{
    //     printk("start to recevice data from dht11! \n");
    // }    

    local_irq_save(irq_flags);//保存当前中断状态并关闭中断,防止中断打断数据交互

    /*wait to response if low 80us -> high 80us*/

    if(dht11_wait_for_level(0,1000) != 0){
        local_irq_restore(irq_flags);
        printk("DHT11 reset failed (Wait Low)\n");
        return -EIO;
    }

    if(dht11_wait_for_level(1,1000) !=0){
        local_irq_restore(irq_flags);
        printk("DHT11 reset failed (Wait High)\n");
        return -EIO;
    }

    if(dht11_wait_for_level(0,1000) !=0){
        local_irq_restore(irq_flags);
        return -EIO;
    }
    for(i = 0;i < 5; i++){
        val = dht11_byte_get();
        if(val < 0){
            local_irq_restore(irq_flags);
            printk("dht11 read byte %d failed \n", i);
            return -EIO;
        }
        data[i] = (unsigned char)val;
    }

    local_irq_restore(irq_flags);//restore ir

    if(data[4] != data[0] + data[1] + data[2] + data[3]){
        printk("dht11 chech sum err! \n");
        return -EIO;
    }
    else{
        err = copy_to_user(buf, (const void *)&data, sizeof(data));
        if(err != 0){
        printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);
        printk("can not copy to user buf\n");
        return -EFAULT;        
        }
    }

    return sizeof(data);
}
ssize_t dht11_write(struct file *file, const char __user *buf, size_t size, loff_t *offset){

    return -EPERM; //返回"操作不允许"错误码
}

int dht11_open(struct inode *node, struct file *file){
    /*init gpio4 19 direction*/
    int err;
    // int minor = iminor(node);

    err = gpiod_direction_output(pt_myDht11_devs->dht11_gpio_desc, 1);//这里已经设为输出并拉高电平？
    if(err != 0){
        printk("can not init gpio as output \n");
        return err;
    }
    else{
        printk("Init gpio 4 19 as output \n");//start to send data to dht11
        printk("init dht11 success!");
    }

    return 0;
}

const struct file_operations dht11_drv_fops = {
    .owner = THIS_MODULE,
    .open = dht11_open,
    .write = dht11_write,
    .read = dht11_read,


};

static int dht11_pdrv_probe(struct platform_device *pdev){
    printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);

    pt_myDht11_devs = devm_kzalloc(&pdev->dev, sizeof(struct my_dht11_dev), GFP_KERNEL);
    if (!pt_myDht11_devs){
        printk("Failed to allocate memory \n");
        return -ENOMEM;
    }

    printk("memory allcated at : %p\n", pt_myDht11_devs);
   /*default status is out LOW*/
    pt_myDht11_devs->dht11_gpio_desc = devm_gpiod_get(&pdev -> dev, NULL, GPIOD_OUT_LOW);
    if(IS_ERR(pt_myDht11_devs->dht11_gpio_desc)){
        printk("Failed to get GPIO descriptor \n");
        return PTR_ERR(pt_myDht11_devs->dht11_gpio_desc);
    }
    else printk("Get GPIO success! \n");


    pt_myDht11_devs->major = register_chrdev(0, "dht11_drv", &dht11_drv_fops);

    pt_myDht11_devs->dht11_drv_class = class_create(THIS_MODULE , "dht11_drv_class");
    err = PTR_ERR(pt_myDht11_devs->dht11_drv_class);

    if (IS_ERR(pt_myDht11_devs->dht11_drv_class)){
        printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(pt_myDht11_devs->major, "dht11_drv");
        return -1;
    }    

    device_create(pt_myDht11_devs->dht11_drv_class, NULL, MKDEV(pt_myDht11_devs->major, 0), NULL, "my_dht11_drv");
    platform_set_drvdata(pdev, pt_myDht11_devs);

    return 0;
}

static int dht11_pdrv_remove(struct platform_device *pdev){
    printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);
    // gpiod_put(pt_myDht11_devs->dht11_gpio_desc);
    device_destroy(pt_myDht11_devs->dht11_drv_class, MKDEV(pt_myDht11_devs->major, 0));
    class_destroy(pt_myDht11_devs->dht11_drv_class);
    unregister_chrdev(pt_myDht11_devs->major, "dht11_drv");

    return 0;
}

static const struct platform_device_id dht11_id_table[] = {
    {"dht11_1", 1},
    {"dht11_2", 2},
    {"dht11_3", 3},
    {},
};

static const struct of_device_id dts_devices_ids[] = {
    {.compatible = "test,my_dht11"},
    {},
};

static struct platform_driver dht11_platform_drv = {
	.probe = dht11_pdrv_probe,
	.remove = dht11_pdrv_remove,
    .driver = {
		.name = "dht11_drv",
        .of_match_table = dts_devices_ids,
	},
    .id_table = dht11_id_table,
};

static int __init dht11_drv_init(void){
    printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);
    ret = platform_driver_register(&dht11_platform_drv);
    if(ret < 0){
        printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);
        printk("can not register dht11 platform drv \n");
    }
    return 0;
}

static void __exit dht11_drv_exit(void){
    printk("%s,%s,%d\n",__FILE__, __FUNCTION__, __LINE__);
    platform_driver_unregister(&dht11_platform_drv);

}

module_init(dht11_drv_init);
module_exit(dht11_drv_exit);
MODULE_LICENSE("GPL");