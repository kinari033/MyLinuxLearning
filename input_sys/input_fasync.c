#include <linux/input.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <poll.h>
#include <signal.h>


int fd;
int count = 0;
struct input_event InputEvent;
void my_sig_handle(int sig){
    while(read(fd, &InputEvent, sizeof(InputEvent)) == sizeof(InputEvent) ){
        printf("get event : type = 0x%0x, code = 0x%x, value = 0x%0x\n", \
                            InputEvent.type, InputEvent.code, InputEvent.value);
    }
}
/**
 * usage: ./get_input_info /dev/input/event1
*/

int main (int argc, char **argv){
    int err;
    int i;
    int ret;
    struct input_id id;
    unsigned char byte;
    int bit;
    unsigned int evbit[2];

    int flags;

    char *ev_names[] = {
                    "EV_SYN ",
                    "EV_KEY ",
                    "EV_REL ",
                    "EV_ABS ",
                    "EV_MSC ",
                    "EV_SW  ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "NULL ",
                    "EV_LED ",
                    "EV_SND ",
                    "NULL ",
                    "EV_REP ",
                    "EV_FF  ",
                    "EV_PWR",
    };

    struct pollfd fds[1];
    nfds_t nfds = 1;

    if(argc != 2){
        printf("Usage: %s </dev/input/event1> \n", argv[1]);
        return -1;
    }


    /*register signal handle*/
    signal(SIGIO, my_sig_handle);

    /* open driver function*/
    fd = open(argv[1], O_RDWR | O_NONBLOCK);

    if(fd < 0){
        printf("cannot open file, ERR(%d):%s\n",errno, strerror(errno));
        return -2;
    }
    else{
        printf("as noblock open file %s successfully\n", argv[1]);
    }



    err = ioctl(fd, EVIOCGID, &id);
    if(err == 0){
        printf("bust_ype = 0x%x\n", id.bustype);
        printf("vendor = 0x%x\n", id.vendor);
        printf("product = 0x%x\n", id.product);
        printf("version = 0x%x\n", id.version);
    }

    ret = ioctl(fd, EVIOCGBIT(0,sizeof(evbit)), &evbit);
    if(ret > 0 && ret <= sizeof(evbit)){
        printf("support ev type: ");
        for (i = 0; i < ret; i++)
        {
                byte = ((unsigned char *)evbit)[i];
                for (bit = 0; bit < 8; bit++)
                {
                        if (byte & (1<<bit)) {
                                printf("%s ", ev_names[i*8 + bit]);
                        }
                }
        }
        printf("\n");
    }


    /*processing id*/
    fcntl(fd, F_SETOWN, getpid());
    /*enable fasync*/
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | FASYNC);

    while(1){
        printf("main loop count = %d \n", count);
        count++;
        sleep(2);
    }

    close(fd);
    return 0;

}