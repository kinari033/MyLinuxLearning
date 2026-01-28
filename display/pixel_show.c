

#include <linux/fb.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>



static struct fb_var_screeninfo var; /*current var*/
static unsigned char *fb_base;
static unsigned int line_width;
static unsigned int pixel_width;

static int screen_size;


void lcd_put_pixel(int x, int y, unsigned int color){

    unsigned char *pen_8 = fb_base + y * line_width + x * pixel_width; 
    unsigned short *pen_16;
    unsigned int *pen_32;

    unsigned int red,green,blue;
    pen_16 = (unsigned short *)pen_8;
    pen_32 = (unsigned int *)pen_16;

    switch(var.bits_per_pixel){
        case 8 :{
            *pen_8 = color;
            break;
        }

        case 16 : {//color = 
            //RGB 565
            red = (color >> 16) & 0xff;
            green = (color >> 8) & 0xff;
            blue = (color >> 0) & 0xff;

            color = ((red >> 3) << 11) | ((green >> 2) <<5 ) | ((blue >> 3));
            *pen_16 = color;
            break;
        }

        case 32: {
            *pen_32 = color;
            break;
        }

        default : {
            printf("can not support %d bpp\n", var.bits_per_pixel);
            break;
        }

    }

}


int main(int argc, char **argv){

    int i;
    int fb;
    int err;
    fb = open("/dev/fb0", O_RDWR);
    if(fb < 0){
        printf("can not open /dev/fb0 \n");
        return -1;
    }
    err = ioctl(fb, FBIOGET_VSCREENINFO, &var);
    if(err < 0){
        printf("can not get var screen info \n");
        return -1;

    }

    line_width = var.xres * var.bits_per_pixel / 8;
    pixel_width = var.bits_per_pixel / 8;
    screen_size = var.xres * var.yres * var.bits_per_pixel / 8;
    /*mmap return (void *) ptr*/
    fb_base = (unsigned char*)mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);

    if(fb_base == (unsigned char *)-1){

        printf("cannot mmap \n");
        return -1;
    }

    //clean screen as white
    memset(fb_base , 0xff, screen_size);

    for(i = 0; i < 100; i++){
        lcd_put_pixel(var.xres/2 + i , var.yres/2 + i, 0xFF0000);

    }
    munmap(fb_base, screen_size);


    close(fb);
    return 0;

}