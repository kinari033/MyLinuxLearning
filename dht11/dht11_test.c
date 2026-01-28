
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv){
    int err;
    int fd = open("/dev/my_dht11_drv", O_RDWR);
    unsigned char data[5];

    printf("DHT11 Driver Test App Started...\n");
    sleep(1); // 等待传感器稳定
    while(1){
        err = read(fd, data , 5);
        if(err < 0){
            printf("cannot read data \n");
            printf("");
            return -1;
        }
        else if(err == 5){
            printf("Humidity: %d.%d %%  |  Temperature: %d.%d C  |  CheckSum: %d\n", 
                   data[0], data[1], data[2], data[3], data[4]);
        }
        sleep(2);
    }
    close(fd);
    return 0;
}