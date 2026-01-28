#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#include <termios.h>

/**
 * ./send_recv <dev>
 * 
*/

int fd;
int ret;
unsigned char c;
char  buf[1000];
char time[100];
char lat[100];
char ns[100];
char lng[100];
char ew[100];
int start = 0;
int i;

int read_gps_raw_data(int fd , char *buf){

    while (1){

        ret = read(fd, &c, 1);
        if(ret ==1){
            if(c == '$'){
                start = 1;
            }
            if(start){
                buf[i++] = c;
            }
            if(c == '\n' || c == 'r'){
                return 0;
            }
        }
        else {
            return -1;
        }
    }
}

int parse_gps_raw_data(char *buf, char *time, char *lat, char *ns,char *lng, char *ew){
    char  tmp[10];

    if(buf[0] != '$'){
        return -1;
    }
    else if(strncmp(buf+3, "GGA", 3) != 0){
        return -1;
    }
    else if(strstr(buf, ",,,,,")){
        printf("place the GPS to the open area \n");
        return -1;
    }
    else{
        sscanf(buf, "%[^,], %[^,], %[^,], %[^,] ,%[^,], %[^,]", tmp, time, lat, ns, lng, ew);
        return 0; 
    }

    return 0;
}

int open_port(const char *path){
    // fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(path, O_RDWR | O_NOCTTY);
    if(fd < 0){
        return -1;
    }
    if (fcntl(fd, F_SETFL, 0) < 0){
        printf("fcntl failed ! \n");
        return -1;
    }

    return fd;
}

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop){
    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0){
        perror("set up serial 1");
        return -1;

    } 
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_oflag &= ~OPOST; 

    switch(nBits){
        case 7: 
            newtio.c_cflag |= CS7;
            break;
        case 8: 
            newtio.c_cflag |= CS8;
            break;            
    }

    switch(nEvent){
        case 'O': 
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E': 
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;  
        case 'N': 
            newtio.c_cflag &= ~PARENB;
            break;                       
    }    

    switch(nSpeed){
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;    
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break; 

    }

    if(nStop == 1){
        newtio.c_cflag &= ~CSTOPB;

    }
    else if(nStop == 2){
        newtio.c_cflag |= CSTOPB;

    }
    newtio.c_cc[VMIN] = 10;
    newtio.c_cc[VTIME] = 0;/* time wait for first data*/
    
    tcflush(fd, TCIFLUSH);
    if((tcsetattr(fd, TCSANOW, &newtio)) != 0){
        perror("com set error");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv){

    if(argc != 2){
        printf("Usage : %s </dev/ttyMXC5> \n", argv[0]);
        return -1;
    }
    /*write and  read*/
    fd = open_port(argv[1]);
    if(fd < 0){
        printf("can not open file %s, err(%d): %s \n", argv[1], errno, strerror(errno));
        return -1;
    }
    ret = set_opt(fd, 9600, 8, 'N', 1);
    if(ret){
        printf("set port  err(%d): %s \n",errno, strerror(errno));
        return -1;
    }
    printf("enter a char :");
    while(1){
/*read line , decode and printf */
        ret = read_gps_raw_data(fd, buf);
        if(ret ==0){
            ret = parse_gps_raw_data(buf, time, lat, ns, lng, ew);
        }
        
        if(ret ==0){
            printf("time: %s \n", time );
            printf("ns: %s \n", ns );
            printf("ew: %s \n", ew );
            printf("lat: %s \n", lat );
            printf("lng: %s \n", lng );
        }

    }
    return 0;

}