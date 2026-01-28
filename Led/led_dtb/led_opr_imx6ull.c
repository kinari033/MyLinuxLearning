#include <linux/module.h>
#include <linux/device.h>
#include <asm/io.h>

#include "led_opr.h"

//IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 address: 0x02290000 + 0x14
static volatile unsigned long *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;//pointer
//GPIO5_GDIR address: 0x020AC004
static volatile unsigned int *GPIO5_GDIR;
//GPIO5_DR address: 0x020AC000
static volatile unsigned int *GPIO5_DR;

extern int g_ledpins[];


static int led0_init(int which){
    printk("DEBUG: led0_init called, which = %d, g_ledpins[%d] = 0x%x, GROUP = %d, PIN = %d \n",
                                        which, which, g_ledpins[which], GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    printk("init gpio : group %d, pin %d\n", GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
    switch(GROUP(g_ledpins[which])){

        case 0:{
            printk("init pin of group ...\n");
            break;
        }
        case 1:{
            printk("init pin of group ...\n");
            break;
        }   

        case 3:{
            printk("init pin of group 3 ...\n");
            break;
        }
        case 5:{
            IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x02290000 + 0x14, 4); 
            // if(!IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3){
            //     printk("%s: failed to map IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3\n", __FUNCTION__);
            //     return -1;
            // }

            GPIO5_GDIR = ioremap(0x020AC004, 4);

            GPIO5_DR = ioremap(0x020AC000, 4);

            *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 &= ~0xf;
            *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 |= 0x5;
            *GPIO5_GDIR |= (1<<3);
            printk("init pin of group 5 ...\n");
            break;
        }
    }

    return 0;

}

static int led_ctl(int which, char status){
    printk("DEBUG: led_ctl called, which = %d, g_ledpins[%d] = 0x%x, GROUP = %d, PIN = %d \n",
                                        which, which, g_ledpins[which], GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    printk("set led %s : group %d, pin %d\n", status?"on":"off",GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    printk("%s, %s, in line %d\n", __FILE__, __FUNCTION__, __LINE__);
    switch(GROUP(g_ledpins[which])){

        case 0:{
            printk("set pin of group 0...\n");
            break;
        }
        case 1:{
            printk("set pin of group 1 ...\n");
            break;
        }   

        case 3:{
            // if (status){
            //     *GPIO5_DR &= ~(1<<3);
            //     printk("init pin of group 3 ...\n");
            // }
            // else{
            //     *GPIO5_DR |= (1<<3);
            // }
            break;
        }
        case 5:{
            if (status){
                *GPIO5_DR &= ~(1<<3);
                printk("set pin of group 5 as ...\n");
            }
            else{
                *GPIO5_DR |= (1<<3);
            }
            break;
        }

    }

    return 0;


}

struct led_operation imx_led_opr = {
    .init = led0_init,
    .ctl = led_ctl,
};

struct led_operation *get_led_opr (void){
    return &imx_led_opr;
}


static int __init led_opr_imx6ull_init(void){
    printk("%s, %s, in line %d \n", __FILE__, __FUNCTION__, __LINE__);
    register_led_opr(&imx_led_opr);
    printk("IMX6ULL LED operation registered\n");
    return 0;

}

static void __exit led_opr_imx6ull_exit(void){
    printk("%s, %s, in line %d \n", __FILE__, __FUNCTION__, __LINE__);
    //IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 address: 0x02290000 + 0x14
    if (IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3){
        iounmap((void*)IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3);
    }
    if (GPIO5_GDIR){
        iounmap((void*)GPIO5_GDIR);
    }
    if (GPIO5_DR){
        iounmap((void*)GPIO5_DR);
    }

}

module_init(led_opr_imx6ull_init);
module_exit(led_opr_imx6ull_exit);
MODULE_LICENSE("GPL");

