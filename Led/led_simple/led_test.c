#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>



/**
 * Usage:
 *       ./led_test on
 *       ./led_test off
 * 
*/
int main(int argc, char **argv){

    if(argc != 3){
        printf("Usage: %s <dev> <on> or <off>\n", argv[0]);
        printf("eg. :  %s /dev/led_device on/off", argv[0]);
        return -1;

    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0 ){
        printf("can not open file %s\n", argv[1]);
        printf("Err(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    else{
        printf("open file %s successfully!\n", argv[0]);
    }

    char status = 0;
    if(strcmp(argv[2], "on") == 0){
        status = 1;

    }
    else if (strcmp(argv[2], "off") == 0){
        status = 0;
    }

    write(fd, &status, 1);

    close(fd);
    return 0;
}