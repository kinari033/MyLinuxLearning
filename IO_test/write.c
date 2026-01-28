#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * write function: ./write 3.txt str1 str2
 * ssize_t write(int fd , const void *buf, size_t count) __wur;warn unused result;
 * 
*/

int main(int argc , char **argv){

    if (argc < 3 ){
        printf("Usage: ./write %s <file> <string1> <string2> ...\n",argv[0]);
        return -1;
    }
    int fd ;
    fd = open(argv[1],O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0){
        printf("can not open file %s\n",argv[1]);
        printf("error num: %s (%d)\n", strerror(errno),errno);
        perror("error print: \n");
    }
    else{
        printf("open file %s fd: %d\n",argv[0],fd);
    }

    //write string1 2 3...
    for (int i = 2; i < argc; i++ ){
        ssize_t bytes_written = write(fd, argv[i] , strlen(argv[i]));

        if (bytes_written == -1){
            perror("write failed\n");
            break;
        }
        if (bytes_written != strlen(argv[i])){
            perror("write do not completely! please check\n");
            break;
        }
        printf("completely written !\n");
        
    }
    write(fd, "\r\n",2);//
    close(fd);
    return 0 ;

}
