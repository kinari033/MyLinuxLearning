#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/**
 * ./hello_drv_simple_test -w abc
 * ./hello_drv_simple_test -r
*/

int main (int argc, char **argv){
    char buf[1024];

    if (argc < 2){
        printf("Usage: %s -w <string>\n", argv[0]);
        printf("       %s -r\n", argv[0]);
        return -1;
    }

    int fd = open("/dev/xyz", O_RDWR);
    if (fd == -1){
        printf("can not open file /dev/xyz\n");
        return -1;
    }
    else{
        printf("open file /dev/xyz successfully!\n");
    }
    /*file operation  */

    if ((0 == strcmp(argv[1],"-w")) && (argc == 3)){
        int len = strlen(argv[2]) + 1; 
        len = len  < 1024 ? len : 1024;
        int write_bytes = write(fd, argv[2], len);
        printf("write driver : %d \n", write_bytes);
    }

    else{

        int len = read (fd , buf , 1024);
        printf("read driver : %d \n", len);
        buf[1023] = '\0';
        printf("APP read : %s \n", buf);
    }
    close(fd);
    return 0;
}