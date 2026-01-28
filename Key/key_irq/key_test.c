#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main (int argc, char **argv){


    int fd ;
    int val;

    if (argc != 2){
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd < 0){
        printf("cannot open file\n, err(%d): %s ",errno, strerror(errno));
        return -1;

    }

    while(1){
        read(fd, &val, 4);
        printf("get button : 0x%x\n", val);
    }
    close(fd);

    return 0;
}