##Linux错误码

-ENOMEM	Error No Memory	kzalloc         分配内存失败时使用。
-EINVAL	Error Invalid Argument	        用户传参非法（比如设置了不支持的模式）。
-EBUSY	Error Device or Resource Busy	资源被占用（比如 GPIO 已经被别的驱动申请了）。
-ENODEV	Error No Such Device	        探测（probe）过程中发现硬件不存在。
-EFAULT	Error Bad Fault	                用户空间和内核空间拷贝数据（copy_to_user）失败。
-EPERM	Error Operation Not Permitted	权限不足，例如某驱动不需要调用read/write 

##关于堆栈(Heap vs Stack)
生存周期（Lifetime）：
如果你在 probe 里定义一个局部变量 struct my_mx1508_drv_desc desc;，它存在于 **栈** 中。当 probe 函数运行结束，这块内存就会被系统回收。
但驱动需要这个结构体贯穿整个生命周期（直到 remove 被调用）。所以必须用 **devm_kzalloc** 在 **堆** 中开辟一块永久空间 。
其他指针是否也要分配？：
原则：指针本身只是一个 4 或 8 字节的地址。如果你想让指针指向一个具体的实体结构，且这个实体在函数结束后还要用，就必须分配内存。
如果指针是指向内核已经存在的对象（比如 pdev->dev），你只需要通过赋值获取地址，不需要再次分配

p_mx1508_desc = devm_kzalloc(&pdev->dev, sizeof(struct my_mx1508_drv_desc), GFP_KERNEL);
if (!p_mx1508_desc){
    printk("Failed to allocate memory \n");
    return -ENOMEM;
}
else{
    printk("memory allcated at : %p\n", p_mx1508_desc);
}

全局变量在内核驱动开发中的潜在风险（比如不支持多设备、重入性差等）。

Linux 内核中一个非常精妙的设计：container_of 宏 和 private_data 机制。

核心原理：如何找回“丢失”的指针？
private_data（私有数据盒子）：struct file 结构体里有一个指针成员叫 private_data。它像是一个随身携带的“便签本”。**我们在 open 函数里把设备结构体的地址写上去**，之后的 ioctl 就可以直接读出来。

container_of（反向寻址神器）：这是 Linux 内核的灵魂宏。它能**通过结构体内某个成员的地址，计算出整个结构体的起始地址**。

cdev（字符设备结构体）：为了配合 container_of，**不再用简单的 register_chrdev**，而是将 **struct cdev** 嵌入到我们的自定义结构体中


struct mx1508_dev {
    struct gpio_desc *ina;    // INA 引脚描述符
    struct gpio_desc *inb;    // INB 引脚描述符
    struct cdev cdev;         // 内嵌字符设备结构体
    struct class *class;      // 设备类
    dev_t devid;             // 设备号（包含主号和次号）
};

static int mx1508_open(struct inode *inode, struct file *file)
{
    struct mx1508_dev *dev;

    // 作用：通过 cdev 成员地址反推整个结构体地址。语法：container_of(成员指针, 结构体类型, 成员名)
    dev = container_of(inode->i_cdev, struct mx1508_dev, cdev);
    
    // 作用：将找回的地址存入 file 的“便签本”。语法：赋值给指针变量
    file->private_data = dev;

    return 0;
}

static int mx1508_probe(struct platform_device *pdev)
{
    struct mx1508_dev *dev;
    int ret;

    // 1. 分配私有数据空间
    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev) return -ENOMEM;

    // 作用：将私有数据存入平台设备中。语法：platform_set_drvdata 方便 remove 函数取回
    platform_set_drvdata(pdev, dev);

    // 2. 获取 GPIO（添加了 Con ID，更符合标准）
    dev->ina = devm_gpiod_get(&pdev->dev, "ina", GPIOD_OUT_LOW);
    dev->inb = devm_gpiod_get(&pdev->dev, "inb", GPIOD_OUT_LOW);
    if (IS_ERR(dev->ina) || IS_ERR(dev->inb)) return -EINVAL;

    // 3. 注册字符设备（替代旧的 register_chrdev）
    ret = alloc_chrdev_region(&dev->devid, 0, 1, "mx1508");
    if (ret < 0) return ret;

    cdev_init(&dev->cdev, &mx1508_fops);
    cdev_add(&dev->cdev, dev->devid, 1);

    // 4. 创建设备节点 (/dev/mx1508_0)
    dev->class = class_create(THIS_MODULE, "mx1508_class");
    device_create(dev->class, NULL, dev->devid, NULL, "mx1508_0");

    return 0;
}

static int mx1508_remove(struct platform_device *pdev)
{
    // 作用：从平台设备中取回之前存入的堆地址。语法：platform_get_drvdata
    struct mx1508_dev *dev = platform_get_drvdata(pdev);

    device_destroy(dev->class, dev->devid);
    class_destroy(dev->class);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->devid, 1);

    return 0;
}




Linux GPIO子系统
devm_gpiod_get 不需要gpiod_put释放
gpiod_set_value
gpiod_get_value
gpiod_direction_output/gpiod_direction_input

Linux PWM子系统

