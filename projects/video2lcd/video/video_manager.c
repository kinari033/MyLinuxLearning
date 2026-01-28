#include <configs.h>
#include <video_manager.h>
/*static 保证该变量和函数在该文件内使用，私有化
* 或者计数器状态保持
*/
static PT_VideoOprs g_ptVideoOprHead = NULL;//定义一个链表头指针


/**
 * 头插法注册VideoOprs结构体
*/
//头插法
int RegisterVideoOpr(PT_VideoOprs ptVideoOprs)
{
    PT_VideoOprs ptVideoOpr_tmp = g_ptVideoOprHead;//定义一个ptVideoOpr指向链表头

    while(ptVideoOpr_tmp)//遍历链表 看看ptVideoOpr是否已经注册过了
    {
        if(ptVideoOpr_tmp == ptVideoOprs)//若是出现相同节点，拒绝注册
        {
            return -1;
        }

        ptVideoOpr_tmp = ptVideoOpr_tmp->ptNext;//往下遍历
    }
    //新节点ptVideoOprs指向原头节点
    ptVideoOprs->ptNext = g_ptVideoOprHead;
    g_ptVideoOprHead = ptVideoOprs;
    return 0;
}

void ShowVideoOprs(void){

    int i = 0;
    PT_VideoOprs ptVideoOpr_tmp = g_ptVideoOprHead;
    while(ptVideoOpr_tmp){
        printf("%02d %s\n", i++, ptVideoOpr_tmp -> name);
        ptVideoOpr_tmp = ptVideoOpr_tmp -> ptNext;
    }
}

PT_VideoOprs GetVideoOpr (char *pcName){
    PT_VideoOprs ptVideoOpr_tmp = g_ptVideoOprHead;

    while(ptVideoOpr_tmp){

        if (strcmp(ptVideoOpr_tmp -> name, pcName) == 0)
        {
            return ptVideoOpr_tmp;
        }
        ptVideoOpr_tmp = ptVideoOpr_tmp -> ptNext;
    }

    return NULL;

}
/**
 * 
 * 初始化视频设备
*/

int VideoDevceInit(char *strDevName, PT_VideoDevice ptVideoDevice){
    int iErr;
    PT_VideoDevice ptVideoDevice_tmp = g_ptVideoOprHead;

    while(ptVideoDevice_tmp){

        iErr = ptVideoDevice_tmp->InitDevice(strDevName, ptVideoDevice);//v4l2.c中的InitDevice函数指针
        if (!iErr){
            return 0;
        }
        ptVideoDevice_tmp = ptVideoDevice_tmp -> ptNext;
    }
    return -1;

}

int videoDeviceInit(char *strDevName, PT_VideoDevice ptVideoDevice){
    /*对链表的每一项调用它的初始化函数*/
    int iErr;
    PT_VideoOprs ptTmp = g_ptVideoOprHead;

    while(ptTmp){
        iErr = ptTmp->InitDevice(strDevName, ptVideoDevice);
        if(!iErr){
            return 0;
        }
        ptTmp = ptTmp->ptNext;
    }
    return -1;

}

int VideoInit(void){

    int iErr;

    iErr = v4l2Init();

    return iErr;
}