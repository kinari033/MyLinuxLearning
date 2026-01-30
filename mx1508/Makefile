KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules 
#	$(CROSS_COMPILE)gcc -o dht11_test dht11_test.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
#	rm -f dht11_test
	
install:
	adb push *.ko /root/kinari/codes/mx1508


obj-m	+= mx1508_drv_improved.o 