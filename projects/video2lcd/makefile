KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

CFLAGS = -I../config

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o key_test key_test.c 

clean:

	
install:

transfer:
	cp  -r ../video2lcd /mnt/hgfs/share_18/projects/camera

obj-y += main.o
obj-y += convert/
obj-y += display/
obj-y += render/
obj-y += video/

