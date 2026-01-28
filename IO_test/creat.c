#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * Usage : ./create 2.txt
 * argc = 2
 * argc[0] = ./create 
 * argv[1] = 2.txt
 * 
 * mode : O_RDONLY;O_WRONLY;O_RDWR;O_APPEND
*/

int main(int argc, char ** argv){
    int fd ;
    if (argc != 2){
        printf("Usage : %s <file>\n",argv[0]);
        return -1;

    }
    fd = open(argv[1],O_RDWR|O_CREAT|O_TRUNC,0777);//
    if (fd < 0){
        printf("can not open file %s \n",argv[1]);
        printf("errno = %d\n",errno);
        printf("err: %s\n",strerror(errno));
        perror("print err");
    }
    else{

        printf("fd = %d\n",fd);
    }
    while (1)
    {
        /* code */
        sleep(10);
    }
    close(fd);
    return 0;

}