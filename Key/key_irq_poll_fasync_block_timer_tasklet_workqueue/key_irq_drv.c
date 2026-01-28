#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/irqreturn.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/uaccess.h>

#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

#include <linux/poll.h>
#include <linux/fcntl.h>

#include <linux/timer.h>

#include <linux/workqueue.h>
#include <asm/current.h>

struct gpio_key{
    int gpio;
    struct gpio_desc *gpiod;
    int flag;
    int irq;
    struct timer_list key_timer;// every gpio_key has own timer
    struct tasklet_struct key_tasklet;
    struct work_struct key_work;
};
static struct gpio_key *gpio_key_P;


//main chrdev 
int major = 0;
static struct class *key_irq_class;


static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);

struct fasync_struct *fasync_key;

/////////////////////*ring buffer start*///////////////////////////////
#define BUF_LEN 128
static int g_key[BUF_LEN];
static int r,w;
#define NEXT_POS(x) ((x+1)%BUF_LEN)

static int is_buf_empty(void){
    return (r==w);
}
static int is_buf_full(void){
    return (r==NEXT_POS(w));
}
static void put_key(int key){
    if (!is_buf_full()){
        g_key[w] = key;
        w = NEXT_POS(w);

    }
}

static int get_key(void){
    int key = 0;
    if (!is_buf_empty()){
        key = g_key[r];
        r = NEXT_POS(r);
    }
    return key;
}


#define CONTAINED_OF 
/////////////////////*ring buffer end*///////////////////////////////

/////////////////////*soft timer start*///////////////////////////////

static void key_timer_expire(unsigned long data){
    // static int cnt = 0;
    int gpio_value; 

    struct gpio_key *gpio_key = data;
    gpio_value = gpiod_get_value(gpio_key->gpiod);
    put_key(gpio_value);

    printk("### key_timer_expire key %d, %d \n",gpio_value, gpio_key->gpio);
    put_key(gpio_value);
    //
    wake_up_interruptible(&gpio_key_wait);

    //send signal
    kill_fasync(&fasync_key, SIGIO, POLL_IN);


}
/////////////////////*soft timer end*///////////////////////////////

/*char device operation*/
// static ssize_t key_irq_write(struct file *file, const char __user *buf, size_t size , loff_t *offset){
//     int err; 
//     char status;
//     struct inode *inode = file_inode(file);

//     int minor = iminor(inode);

//     printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
//     // err = copy_from_user(&status, buf, 1);
//     // // p_led_opr -> ctl(minor, status);
//     // // gpiod_set_value(led_gpio, status);
//     // // printk("set led pin 0x%x as %d \n", leds_desc[minor].pin, status);

//     return 1;

// }

// static int key_irq_open(struct inode *node , struct file *file){

//     // int err;
//     int minor = iminor(node);
//     printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);

//     // // p_led_opr -> init(minor);
//     // gpiod_direction_output(led_gpio, 0);
//     // printk("init led pin 0x%x as output", leds_desc[minor].pin);
    
//     return 0;
// }

static ssize_t key_irq_read(struct file *file, char __user *buf, size_t size, loff_t *offset){

    int err;
    int key;
    //用户使用read。调用sys_read,进入内核，把gpio_key_wait放入gpio_queue,等待事件g_key唤醒为true,然后拷贝g_key到用户空间
    printk("%s, %s, in line %d:\n",__FILE__, __FUNCTION__, __LINE__);

    if ((file->f_flags & O_NONBLOCK) && is_buf_empty()){
        return -EAGAIN;

    }

    wait_event_interruptible(gpio_key_wait, !is_buf_empty());
    key = get_key();
    err = copy_to_user(buf, &g_key, 4);
    return 4; 
}

static unsigned int key_irq_poll(struct file *fp, struct poll_table_struct *wait){
    printk("%s, %s, in line %d:\n",__FILE__, __FUNCTION__, __LINE__);

    poll_wait(fp, &gpio_key_wait, wait);
    return is_buf_empty() ? 0 : POLLIN | POLLRDNORM ;
}



static int key_irq_fasync(int fd, struct file *file, int on){
    if (fasync_helper(fd, file, on, &fasync_key) >= 0){
        return 0;
    }else{
        return -EIO;
    }
}


static struct file_operations key_irq_fopr = {
    .owner = THIS_MODULE, 
    // .open = key_irq_open,
    // .write = key_irq_write,
    .read = key_irq_read,
    .poll = key_irq_poll,
    
    .fasync = key_irq_fasync

};

/*if interrupted, call function*/
static irqreturn_t gpio_key_isr(int irq, void *dev_id){
    // // static int cnt = 0;
    // int gpio_value; 

    // struct gpio_key *gpio_key = dev_id;
    // gpio_value = gpiod_get_value(gpio_key->gpiod);
    // put_key(gpio_value);

    // printk("###gpio_key_isr key %d, %d \n",gpio_value, gpio_key->gpio);
    // put_key(gpio_value);
    // //
    // wake_up_interruptible(&gpio_key_wait);

    // //send signal
    // kill_fasync(&fasync_key, SIGIO, POLL_IN);
    struct gpio_key *gpio_key = dev_id;
    // printk("gpio_key_isr key irq happen \n");
    tasklet_schedule(&gpio_key_P->key_tasklet);
    mod_timer(&gpio_key_P->key_timer, jiffies + HZ/50);

    schedule_work(&gpio_key_P -> key_work);
    return IRQ_HANDLED;

}


////////////////////////*tasklet start*///////////////////////////

static void key_tasklet_func(unsigned long data){

    // static int cnt = 0;
    int gpio_value; 

    struct gpio_key *gpio_key = data;
    gpio_value = gpiod_get_value(gpio_key->gpiod);
    put_key(gpio_value);

    printk("### key_tasklet_func key %d, %d \n",gpio_value, gpio_key->gpio);
    put_key(gpio_value);
    //
    wake_up_interruptible(&gpio_key_wait);

    //send signal
    kill_fasync(&fasync_key, SIGIO, POLL_IN);    
}
////////////////////////*tasklet end*///////////////////////////

////////////////////////*workqueue strat*///////////////////////////

static void key_work_func(struct work_struct *work){
    int gpio_value; 
    struct gpio_key *gpio_key = container_of(work, struct gpio_key_P, work);

    gpio_value = gpiod_get_value(gpio_key->gpiod);

    printk("### key_work_func key %d, %d \n",gpio_value, gpio_key->gpio);
    printk("### key_work_func the process id is %s PID %i \n",current->comm, current->pid);

}
////////////////////////*workqueue end*///////////////////////////

static int key_irq_probe(struct platform_device *pdev){
    int err;

    /*irq func*/
    int count ; 
    struct device_node *node = pdev->dev.of_node;
    enum of_gpio_flags flag;
    int i;
    int j;

    printk("%s, %s, in line %d:\n",__FILE__, __FUNCTION__, __LINE__);
    major = register_chrdev(major, "key_irq", &key_irq_fopr);
    key_irq_class = class_create(THIS_MODULE, "key_irq_class");

    err = PTR_ERR(key_irq_class);
    if (IS_ERR(key_irq_class)){

        printk("%s, %s, in line %d:\n",__FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, "key_irq");
        return -1;
    }

    device_create(key_irq_class, NULL, MKDEV(major, 0), NULL, "key_irq");


    /*irq func*/
    count = of_gpio_count(node);

    if (!count){
        printk("%s, %s, in line %d, there is not any gpio available:\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    gpio_key_P = kzalloc(sizeof(struct gpio_key) * count, GFP_KERNEL);
    //申请注册gpio并配置
    for (i = 0;  i < count; i++){
        gpio_key_P[i].gpio = of_get_gpio_flags(node, i, &flag);

        if (gpio_key_P[i].gpio < 0){
            printk("%s, %s, in line %d, of_get_gpio_flags failed:\n",__FILE__, __FUNCTION__, __LINE__);
            return -1;

        }

        //申请GPIO资源 
        err = gpio_request(gpio_key_P[i].gpio, "key_irq_gpio");
        if (err) {
            printk("###gpio_request failed for gpio %d, err = %d\n", gpio_key_P[i].gpio, err);
            // 释放之前已申请的GPIO
            while (--i >= 0) {
                gpio_free(gpio_key_P[i].gpio);
            }
            kfree(gpio_key_P);
            return err;
        }

        gpio_key_P[i].gpiod = gpio_to_desc(gpio_key_P[i].gpio);

        //设置GPIO为输入方向（非常重要！）
        err = gpiod_direction_input(gpio_key_P[i].gpiod);
        if (err) {
            printk("###gpiod_direction_input failed for gpio %d, err = %d\n", gpio_key_P[i].gpio, err);
            // 释放GPIO
            for (j = 0; j <= i; j++) {
                gpio_free(gpio_key_P[j].gpio);
            }
            kfree(gpio_key_P);
            return err;
        }

            // gpio_key_P[i].gpiod = gpio_to_desc(gpio_key_P[i].gpio);
            gpio_key_P[i].flag = flag & OF_GPIO_ACTIVE_LOW;
            gpio_key_P[i].irq = gpio_to_irq(gpio_key_P[i].gpio);

            // init and add timer
            setup_timer(&gpio_key_P[i].key_timer, key_timer_expire, &gpio_key_P[i]);
            gpio_key_P[i].key_timer.expires = ~0;
            add_timer(&gpio_key_P[i].key_timer);

            //init tasklet
            tasklet_init(&gpio_key_P[i].key_tasklet, key_tasklet_func, &gpio_key_P[i]);

            //init woker
            INIT_WORK(&gpio_key_P[i].key_work, key_work_func);



    }


    //申请注册中断
    for(i = 0; i < count; i++){



        err = request_irq(gpio_key_P[i].irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_key", &gpio_key_P[i]);
    
    //错误检查
        if(err){
            printk("###request_irq failed for irq %d, gpio %d, err = %d\n", gpio_key_P[i].irq, gpio_key_P[i].gpio, err);
          // 释放已注册的中断
            while (--i >= 0) {
                free_irq(gpio_key_P[i].irq, &gpio_key_P[i]);
            }
            // 释放所有GPIO
            for (i = 0; i < count; i++) {
                gpio_free(gpio_key_P[i].gpio);
            }
            kfree(gpio_key_P);
            return err;
        }
        printk("###request_irq success for irq %d, gpio %d\n", gpio_key_P[i].irq, gpio_key_P[i].gpio);
    }


    return 0;
}

static int key_irq_remove(struct platform_device *pdev){

    struct device_node *node = pdev->dev.of_node;
    int count;
    int i;
    
    printk("%s, %s, in line %d:\n",__FILE__, __FUNCTION__, __LINE__);

    device_destroy(key_irq_class, MKDEV(major, 0));

    class_destroy(key_irq_class);

    unregister_chrdev(major, "key_irq");

    count = of_gpio_count(node);
    for(i = 0; i < count; i++){
        free_irq(gpio_key_P[i].irq, &gpio_key_P[i]);
        gpio_free(gpio_key_P[i].gpio);//释放gpio
        del_timer(&gpio_key_P[i].key_timer);//delet timer

        tasklet_kill(&gpio_key_P[i].key_tasklet); //remove tasklet
    }
    // free_irq(key_irq, NULL);
    kfree(gpio_key_P);

    return 0;

}
static const struct of_device_id key_irq_dts_devices_ids[] = {
    {.compatible = "test,key_irq"},
    {},
};

static struct platform_driver key_irq_pdrv = {
	.probe = key_irq_probe,
    .remove = key_irq_remove,

	.driver = {
		.name = "key_irq_drv",
        .of_match_table = key_irq_dts_devices_ids,
	},
};

static int __init key_irq_init(void){

    int ret;
    ret = platform_driver_register(&key_irq_pdrv);

    return ret;
}

static void __exit key_irq_exit(void){

    platform_driver_unregister(&key_irq_pdrv);
}
module_init(key_irq_init);
module_exit(key_irq_exit);
MODULE_LICENSE("GPL");

