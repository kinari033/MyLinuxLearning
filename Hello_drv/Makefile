

KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o hello_test hello_drv_simple_test.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f hello_test

# 参考内核源码drivers/char/ipmi/Makefile
# 要想把a.c, b.c编译成ab.ko, 可以这样指定:
# ab-y := a.o b.o
# obj-m += ab.o

# leddrv.c board_100ask_imx6ull.c 编译成 100ask_led.ko -> driver files
# 100ask_led-y := leddrv.o board_100ask_imx6ull.o
obj-m	+= hello_drv_simple.o


# KVERSION = 4.18.0-15-generic
# KERN_DIR =  /lib/modules/$(KVERSION)/build

# all:
# 	make -C $(KERN_DIR) M=$(CURDIR) modules

# clean:
# 	make -C $(KERN_DIR) M=$(CURDIR) modules clean
# 	rm -rf modules.order

# obj-m	+= hello_drv_simple.o
