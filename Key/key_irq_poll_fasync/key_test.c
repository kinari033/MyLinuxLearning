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

static void signal_func(int sig){
    /*read*/
    int val;
    read(fd, &val, 4);
    /*printf*/
    printf("get button : 0x%x\n", val);

}


int main (int argc, char **argv){

    int val;
    int poll_ret;
    /*struct pollfd *fds, nfds_t nfds, int timeout*/
    struct pollfd fds[1];
    int timeout = 5000;

    int flags ;

    if (argc != 2){
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }
    /*signal*/
    signal(SIGIO, signal_func);//send signal
    
    //
    fd = open(argv[1], O_RDWR);

    fcntl(fd, F_SETOWN, getpid());//get filp pid
    flags = fcntl(fd, F_GETFL); // get filep flags
    fcntl(fd, F_SETFL, flags | FASYNC);//set flags
    if(fd < 0){
        printf("cannot open file\n, err(%d): %s ",errno, strerror(errno));
        return -1;

    }else{
        printf("successfully open file %s \n", argv[1]);
    }
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
        printf("hello world \n");
        sleep(2);
    }

    close(fd);

    return 0;
}