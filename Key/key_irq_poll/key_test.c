#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <poll.h>

int main (int argc, char **argv){


    int fd ;
    int val;
    int poll_ret;
    /*struct pollfd *fds, nfds_t nfds, int timeout*/
    struct pollfd fds[1];
    int timeout = 5000;

    if (argc != 2){
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd < 0){
        printf("cannot open file\n, err(%d): %s ",errno, strerror(errno));
        return -1;

    }else{
        printf("successfully open file %s \n", argv[1]);
    }

    fds[0].fd = fd;
    fds[0].events = POLLIN;
    while(1){
        poll_ret = poll(fds, 1, timeout);
        if((poll_ret == 1) && (fds[0].revents & POLLIN)){
            read(fd, &val, 4);
            printf("get button : 0x%x\n", val);
        }

        else{
            printf("time out%d\n", timeout);
        }
    }
    close(fd);

    return 0;
}