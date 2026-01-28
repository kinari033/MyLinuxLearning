KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88


all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o send_recv send_recv.c 
	$(CROSS_COMPILE)gcc -o gps gps.c 


clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f send_recv 
	rm -f gps 


	
install:
	adb push send_recv /root/kinari/codes/uart