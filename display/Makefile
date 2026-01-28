KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88


all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o pixel_show pixel_show.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f pixel_show
	
install:
	adb push *.ko pixel_show /root/kinari/codes/display

# obj-y += main.o
