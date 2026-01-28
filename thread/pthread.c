#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include <semaphore.h>

int err;
pthread_t tid;
int count = 0;

static sem_t g_sem;
static int g_pshared = 0;

unsigned char buf[1000];

static pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER;


static void *my_thread_function (void *data){

    while(1){
        // while(hasData == 0);
        err = sem_wait(&g_sem);
        if(err != 0){
            printf("cannot sem_wait.err(%d): %s\n", errno,strerror(errno));
        }
        /*printf*/
        pthread_mutex_lock(&g_tMutex);
        printf("recv data: %s \n", buf);
        pthread_mutex_unlock(&g_tMutex);
    }
    return NULL;
}

int main(int argc, char **argv){
    unsigned char temp_buf[1000];
    //creat receive thread
    err = pthread_create(&tid, NULL, my_thread_function, NULL );
    if(err != 0){
        printf("cannot create thread.err(%d): %s\n", err,strerror(err));
        return -1;
    }
    // main thread read stdio, send it to receive thread
    err = sem_init(&g_sem, g_pshared, 0);
    if(err != 0){
        printf("sem_init, err(%d): %s\n", errno,strerror(errno));
        return -1;
    }
    while(1){
        // printf("this is main thread \n");
        fgets(temp_buf, 1000, stdin);
        pthread_mutex_lock(&g_tMutex);
        memcpy(buf, temp_buf, 1000);
        pthread_mutex_unlock(&g_tMutex);
        sem_post(&g_sem);
    }
    return 0;
}