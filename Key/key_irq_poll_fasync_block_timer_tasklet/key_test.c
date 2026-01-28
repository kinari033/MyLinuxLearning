#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <poll.h>

#include <signal.h>

static int fd;

int main (int argc, char **argv){

    int val;
    int poll_ret;
    /*struct pollfd *fds, nfds_t nfds, int timeout*/
    struct pollfd fds[1];
    int timeout = 5000;

    int flags ;

    int i;

    if (argc != 2){
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }


    //defailt open mode as BLOCK
    fd = open(argv[1], O_RDWR | O_NONBLOCK);

    if(fd < 0){
        printf("cannot open file\n, err(%d): %s ",errno, strerror(errno));
        return -1;

    }else{
        printf("successfully open file %s \n", argv[1]);
    }

    for (i = 0; i< 10; i++){
        if(read(fd, &val, 4) == 4){
            printf("Get button : 0x%x\n", val);
        }else{
            printf("Get button : -1\n");
        }
    }
    flags = fcntl(fd, F_GETFL | O_NONBLOCK); // get filep flags (与open函数的设置标记位2选一)
    fcntl(fd, F_SETFL, flags &  ~O_NONBLOCK);//clear flags O_NONBLOCK
////////////* poll start*///////////////
    // fds[0].fd = fd;
    // fds[0].events = POLLIN;
    // while(1){
    //     poll_ret = poll(fds, 1, timeout);
    //     if((poll_ret == 1) && (fds[0].revents & POLLIN)){
    //         read(fd, &val, 4);
    //         printf("get button : 0x%x\n", val);
    //     }

    //     else{
    //         printf("time out%d\n", timeout);
    //     }
    // }
////////////* poll end*///////////////
    while(1){
        if(read(fd, &val, 4) == 4){
            printf("Get button : 0x%x\n", val);
        }else{
            printf("while Get button : -1\n");
        }
        sleep(2);
    }

    close(fd);

    return 0;
}