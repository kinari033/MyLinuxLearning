#include "configs.h"

/**
 * socket
 * bind
 * recvfrom/sendto
*/


#define BACKLOG 10

int iSocketServer;
int iSocketClient;

int iRet;

struct sockaddr_in t_sockServerAddr;/*addr in order to avoid compiler warnings*/
struct sockaddr_in t_sockClientAddr;/*addr in order to avoid compiler warnings*/

int iAddrLen;
unsigned char RecvClientBuf[1000];
int iClientNum = -1;

int main(int argc, char **argv){

    iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);
    if(iSocketServer < 0){
        printf("cannot socket err(%d): %s\n", errno,strerror(errno));
        return -1;
    }

    t_sockServerAddr.sin_family = AF_INET;
    t_sockServerAddr.sin_port = htons(SERVER_PORT);
    t_sockServerAddr.sin_addr.s_addr = INADDR_ANY;
    memset(t_sockServerAddr.sin_zero, 0, 8);

    iRet =  bind(iSocketServer, (const struct sockaddr *)&t_sockServerAddr,
        sizeof(const struct sockaddr));

    if(iRet == -1){
        printf("bind err(%d):%s \n", errno,strerror(errno));
        return -1;
    }

    while(1){
        /*accept*/
        /*       ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen);*/
        iAddrLen = sizeof(struct sockaddr);
        iRet = recvfrom(iSocketServer, RecvClientBuf, 999, 0, (struct sockaddr *)&t_sockServerAddr, (socklen_t *)&iAddrLen);
        if(iRet < 0){
            printf("can not receive msg from ip:%d,port:%d\n",t_sockServerAddr.sin_addr.s_addr, t_sockServerAddr.sin_port);
            printf("err(%d):%s\n", errno, strerror(errno));
            return -1;
        }
        else{
            RecvClientBuf[iRet] = '\0';
            printf("get msg from%s : %s \n", inet_ntoa(t_sockServerAddr.sin_addr), RecvClientBuf);
        }
    }

    // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    close(iSocketServer);
    return 0;
}