KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88


all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o input_fasync input_fasync.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f input_fasync
	
install:
	adb push input_fasync /root/kinari/codes/input_sys

# obj-y += main.o