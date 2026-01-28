KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88


all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o pthread pthread.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f pthread pthread1

	
install:
	adb push server /root/kinari/codes/thread