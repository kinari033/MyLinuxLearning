#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/**
 * Usage:
 *      ./led_test /dev/led_device on or off
*/
int main(int argc, char **argv){

    if(argc != 3){
        printf("Usage: %s, /dev/led_device on or off \n", argv[0]);
        return -1;

    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0){

        printf("can not open file,Err(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    else{
        printf("open file successfully \n");
    }
    
    char status;
    if (strcmp(argv[2], "on") == 0){
        status = 1;
    }
    else if (strcmp(argv[2], "off") == 0){
        status = 0;
    }

    int write_ret = write(fd, &status, 1);

    close(fd);
    return 0;
}

