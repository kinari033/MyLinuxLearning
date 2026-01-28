#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <stdlib.h>

#define IOCTL_AT24C02_READ 100
#define IOCTL_AT24C02_WRITE 101



/**
 * at24c02 /dev/at24c02_drv r 10
 * at24c02 /dev/at24c02_drv w 10 a
 * 
*/
int main (int argc, char **argv){
    int fd ; 
    int buf[2];
    int addr;
    int data;

    if ((argc != 4) && (argc != 5)){
        printf("Usage: %s <dev> r <addr> \n", argv[0]);
        printf("Usage: %s <dev> w <addr> <val> \n", argv[0]);
        return -1;

    }

    fd = open(argv[1], O_RDWR);

    if (fd < 0 ){
        printf("cannot open file,err(%d): %s \n", errno, strerror(errno));
        return -1;
    }
    // read();
    if(argv[2][0] == 'r'){

        buf[0] = strtoul(argv[3],NULL,0);
        ioctl(fd, IOCTL_AT24C02_READ, buf);
        printf("Read addr 0x%x, get data 0x%x\n",buf[0], buf[1]);
    }
    else{
        buf[0] = strtoul(argv[3],NULL,0);
        buf[1] = strtoul(argv[4],NULL,0);
        ioctl(fd, IOCTL_AT24C02_WRITE, buf);

        printf("Read addr 0x%x, get data 0x%x\n",buf[0], buf[1]);

    }
    
    close(fd);
    return 0;
}
