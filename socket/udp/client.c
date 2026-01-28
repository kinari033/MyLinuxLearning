#include "configs.h"
#include <sys/socket.h>

/**
 * 
 * socket
 * connect
 * send/recv
*/

int iSocketClient;

int iRet;

struct sockaddr_in t_sockClientAddr;
socklen_t sockClientAddrLen;

unsigned char sendBuf[1000];

int main (int argc, char **argv){

    if (argc != 2){
        printf("Usage: %s <server ip> \n", argv[0]);
        return -1;
    }

    t_sockClientAddr.sin_family = AF_INET;
    t_sockClientAddr.sin_port = htons(SERVER_PORT);
    iRet = inet_aton(argv[1], &t_sockClientAddr.sin_addr);
    if(iRet == 0){
        printf("the address is invalid err(%d): %s \n",errno, strerror(errno));
        return -1;
    }    
    // t_sockClientAddr.sin_addr.s_addr = INADDR_ANY;
    memset(t_sockClientAddr.sin_zero, 0, 8);

    /*int this fd dose not contain any addr msg*/
    iSocketClient =socket(AF_INET, SOCK_DGRAM, 0);/*int socket(int domain, int type, int protocol);*/
    sockClientAddrLen = sizeof(struct sockaddr);
    /*int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);*/
    iRet = connect (iSocketClient, (const struct sockaddr *)&t_sockClientAddr, sockClientAddrLen);
    if(iRet < 0){
        printf("cannot connect err(%d): %s \n",errno, strerror(errno));
        return -1;
    }
    while(1){
        if(fgets((char *)sendBuf, 999, stdin) != NULL){
            iRet = send(iSocketClient, sendBuf, strlen((char *)sendBuf), 0);/* ssize_t send(int sockfd, const void *buf, size_t len, int flags); */
            if (iRet <= 0){
                printf("can not send msg ,err(%d):%s\n", errno,strerror(errno));
                return -1;
            }
            else{
                printf("send %s \n",sendBuf);
            }
        }
    }

    return 0;
}
