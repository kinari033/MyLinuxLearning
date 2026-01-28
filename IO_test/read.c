/**
 * ssize_t read (int fd , void *buf , size_t count)
 * 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv ){
    if (argc != 2){
        printf("Usage : open file %s <file> \n",argv[0]);
    }

    int fd = open(argv[1],O_RDONLY);
    if (fd < 0 ){
        printf("can not open file %s erro: %s (%d)\n",argv[1],strerror(errno),errno);
    }
    else{
        printf("open file %s and read, fd = %d\n",argv[1],fd);

    }

    unsigned char buf[100];
    while (1)
    {
        ssize_t bytes_read = read(fd , buf, sizeof(buf)-1);
        if (bytes_read < 0){
            perror("read error:");
            close (fd);
            return -1;
        }

        else if (bytes_read == 0) break;

        else {
            buf[bytes_read] = '\0';
            printf("%s\n",buf);
        }

    }
    
    close(fd);
    return 0 ;
}