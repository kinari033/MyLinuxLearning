KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o at24c02_test at24c02_test.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f at24c02_test
	
install:
	adb push *.ko at24c02_test /root/kinari/codes/I2C


obj-m	+= at24c02_drv.o 