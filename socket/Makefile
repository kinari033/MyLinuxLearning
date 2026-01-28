KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88


all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o server server.c 
	$(CROSS_COMPILE)gcc -o client client.c 



clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f server
	rm -f client
	
install:
	adb push server client /root/kinari/codes/socket
