#include "configs.h"

/**
 * socket
 * bind
 * listen
 * accept
 * recv/send
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

    iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
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

    iRet = listen(iSocketServer, BACKLOG);
    if(iRet == -1){
        printf("listen err(%d):%s \n", errno,strerror(errno));
        return -1;
    }
    while(1){
        /*accept*/
        iAddrLen = sizeof(struct sockaddr);
        iSocketClient = accept(iSocketServer, (struct sockaddr *)&t_sockClientAddr, (socklen_t *)&iAddrLen);
        if(iSocketClient < 0){ 
            printf(" socket client accept err(%d):%s \n", errno,strerror(errno));
            return -1;
            
        }else {
            iClientNum++;
            printf("socket client %d accessed, ip : %s \n", iClientNum, inet_ntoa(t_sockClientAddr.sin_addr));
    
            /*fork() == 0 -> enter child process; fork > 0 father process*/

            if(!fork()){//child process enter "while" ,mean 
                while(1){
                    /*accept client data and show*/
                    iRet = recv(iSocketClient, RecvClientBuf, 999, 0);
                    if(iRet == -1){
                        printf("receive err(%d):%s \n", errno,strerror(errno));
                        close(iSocketClient);
                        return -1;
                    }
                    else{
                        RecvClientBuf[iRet] = '\0';
                        printf("get data from client %d  ,data : %s \n", iClientNum, RecvClientBuf);
                    }
                }
            }

            /*father process*/
            else{

            }
        }
    }

    // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    close(iSocketServer);
    return 0;
}