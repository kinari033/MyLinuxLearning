#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * write function: ./write 3.txt mode str1 str2
 * ssize_t write(int fd , const void *buf, size_t count) __wur;warn unused result;
 * 
*/
int is_valid_num(const char *str){
    char *endptr;
    errno = 0;
    long num = strtol(str,&endptr,10);

    if (*endptr != '\0'){
        fprintf(stderr,"input num '%s' is illegel('%c')\n",str, *endptr);
        return 1;
    }
    if (errno == ERANGE){
        fprintf(stderr,"input num '%s' is out range \n",str);
        return 1;
    }
    if (num < 0){
        fprintf(stderr,"input num '%s' must be positive \n",str);
        return 1;
    }
    return 0;

}
int main(int argc , char **argv){

    if (argc < 3 ){
        printf("Usage: %s <file> <write_mode>  ...\n",argv[0]);
        return -1;
    }

    if (strcmp(argv[2],"NEW") == 0){
        printf("write mode : %s\n",argv[2]);
        int fd = open(argv[1],O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (fd < 0){
            printf("can not open file %s\n",argv[1]);
            printf("error num: %s (%d)\n", strerror(errno),errno);
            perror("error print: \n");
        }
        else{
            printf("open file %s fd: %d\n",argv[0],fd);
        }

        //write string1 2 3...
        for (int i = 3; i < argc; i++ ){
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
            write(fd, "\r\n",2);
        }
        close(fd);
        return 0 ;
    }
    else if (strcmp(argv[2],"INSERT") == 0){
        printf("write mode : %s\n",argv[2]);
        int fd = open(argv[1],O_RDWR | O_CREAT , 0644);

        if (fd < 0){
            printf("can not open file %s\n",argv[1]);
            printf("error num: %s (%d)\n", strerror(errno),errno);
            perror("error print: \n");
        }
        else{
            printf("Successfully open file %s fd: %d\n",argv[0],fd);
        }
        
        if (is_valid_num(argv[3]) == 0){
            printf("insert offset %s\n",argv[3]);
            lseek(fd,3,SEEK_SET);
            write(fd,"test",4);
            // for(int i = 4;i < argc ; i++ ){
            //     for (int j; j < strlen(argv[i]);j++){
            //         lseek(fd,argv[3]+,);

            //     }


            // }
        }
        else{
            printf("insert position is something wrong \n");
            return -1;
        }
        close(fd);
        return 0 ;
    }
    
    else{
        printf("Usage: %s <file> <write_mode> \n",argv[0]);
        return -1;
    }

    return 0;

}
