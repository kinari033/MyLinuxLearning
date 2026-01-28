# include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/**
 * ./open 1.text
 * *argv[0] = "./open"
 * *argv[1] = "1.text"
*/

int main(int argc , char **argv){
    int fd;
    if (argc != 2){
        printf("USage: %s <file>\n",argv[0]);
        return -1;
    }
    if (argc ==2){
        fd = open(argv[1],O_RDWR);
        if (fd < 0){
            printf("can not open file %s\n",argv[1]);
            //print erro num;
            printf("errno  = %d\n",errno);
            printf("err: %s\n",strerror(errno));
            perror("open");//
        }
        
        printf("fd = %d\n",fd);
    }

    while (1)
    {
        sleep(10);
    }

    close(fd);
    return 0 ;

}