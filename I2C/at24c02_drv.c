#include <linux/module.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/err.h>
// #include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#include <linux/i2c.h>
#include <asm/uaccess.h>


#define IOCTL_AT24C02_READ 100
#define IOCTL_AT24C02_WRITE 101

static int major = 0;
static struct class *at24c02_drv_class;

static struct i2c_client *at24c02_client;//platform_device 


static int at24c02_drv_open(struct inode *node , struct file *file){
    printk("%s, %s, %d \n",__FILE__, __FUNCTION__, __LINE__);

    return 0;
}

static ssize_t at24c02_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset){
    printk("%s, %s, %d \n",__FILE__, __FUNCTION__, __LINE__);

    return 4; 
}

static ssize_t at24c02_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset){
    printk("%s, %s, %d \n",__FILE__, __FUNCTION__, __LINE__);


    return 0;
}
static long at24c02_drv_ioctl (struct file *file, unsigned int cmd, unsigned long arg){
    unsigned char addr;
    unsigned char data;

    unsigned int ker_buf[2];
    struct i2c_msg msgs[2]; 

    unsigned int *user_buf = (unsigned int *)arg;

    unsigned char byte_buf[2]; 

    copy_from_user(ker_buf, user_buf, 8);
    addr = ker_buf[0];

    switch (cmd)
    {
        case IOCTL_AT24C02_READ:
        {    /* read */
            msgs[0].addr = at24c02_client->addr;
            msgs[0].flags = 0; /*write*/
            msgs[0].len = 1; /*write len */
            msgs[0].buf = &addr; /*write at addr*/

            msgs[1].addr = at24c02_client->addr;
            msgs[1].flags = I2C_M_RD; /*read*/
            msgs[1].len = 1; /*read len */
            msgs[1].buf = &data; /*read at addr*/

            i2c_transfer(at24c02_client->adapter, msgs, 2);

            ker_buf[1] = data;
            copy_to_user(user_buf, ker_buf, 8);
            break;
        }
        case IOCTL_AT24C02_WRITE:
        {    /* write */
            byte_buf[0] = addr;
            byte_buf[1] = ker_buf[1];

            msgs[0].addr = at24c02_client->addr;
            msgs[0].flags = 0; /*write*/
            msgs[0].len = 2; /*write len */
            msgs[0].buf = byte_buf; /*write at addr*/

            i2c_transfer(at24c02_client->adapter, msgs, 1);
            //delay
            msleep(20);
            mdelay(20);

            
            break;
        }  

    }
    return 0;
}


static struct file_operations at24c02_drv_fopr = {
    .owner              = THIS_MODULE,
    .open               = at24c02_drv_open,
    .write              = at24c02_drv_write,
    .read               = at24c02_drv_read,
	.unlocked_ioctl     = at24c02_drv_ioctl,

};

//i2c_client  = platform_device *pdev
static int at24c02_drv_probe(struct i2c_client *i2c_client, const struct i2c_device_id *i2c_device_id){


    int err; 
    printk("%s, %s, %d \n",__FILE__, __FUNCTION__, __LINE__);

    major = register_chrdev(major, "at24c02_drv", &at24c02_drv_fopr);
    at24c02_drv_class = class_create(THIS_MODULE, "at24c02_drv_class");
    err = PTR_ERR(at24c02_drv_class);
    if(IS_ERR(at24c02_drv_class)){
        unregister_chrdev(major, "at24c02_drv");
        printk("%s, %s, %d \n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    device_create(at24c02_drv_class, NULL, MKDEV(major, 0), NULL, "at24c02_drv");


    at24c02_client = i2c_client;//record this struct 
    // i2c_transfer();

    return 0;
}

static int at24c02_drv_remove(struct i2c_client *i2c_client){
    printk("%s, %s, %d \n",__FILE__, __FUNCTION__, __LINE__);

    device_destroy(at24c02_drv_class, MKDEV(major, 0));

    class_destroy(at24c02_drv_class);

    unregister_chrdev(major, "at24c02_drv");

    return 0;

}

static const struct of_device_id iat24c02_i2c_match_tables[] = {
    {.compatible = "test,at24c02"},
    {},
}; 

static const struct i2c_device_id iat24c02_i2c_ids[] = {
    {"111"},
    {},

};

static struct i2c_driver at24c02_drv = {
    .probe = at24c02_drv_probe,
    .remove = at24c02_drv_remove,
    .driver = {
        .name = "at24c02_drv",
        .owner = THIS_MODULE,
        .of_match_table = iat24c02_i2c_match_tables,
    },
    .id_table = iat24c02_i2c_ids,
};

static int __init at24c02_drv_init(void){
    int err;
    err = i2c_add_driver(&at24c02_drv);
    return err;
}

static void __exit at24c02_drv_exit(void){
    i2c_del_driver(&at24c02_drv);
}

module_init(at24c02_drv_init);
module_exit(at24c02_drv_exit);
MODULE_LICENSE("GPL");





