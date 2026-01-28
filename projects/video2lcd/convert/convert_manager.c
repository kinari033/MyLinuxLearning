#include <configs.h>
#include <convert_manager.h>

// static struct VideoConvertOprs  = {
//     .is_support     = is_support(int iPixelFormat_In, int iPixelFormat_Out);
//     .convert        = convert(PT_VideoBuf ptVideoBuf_In, PT_VideoBuf ptVideoBuf_Out);
//     .convertExit    = convertExit(PT_VideoBuf ptVideoBuf_Out);//release buffer
// };


/*static 保证该变量和函数在该文件内使用，私有化
* 或者计数器状态保持
*/
static PT_VideoConvert g_ptVideoOprHead = NULL;//定义一个链表头指针


/**
 * 头插法注册VideoConvert结构体
*/
//头插法
int RegisterVideoConvertOpr(PT_VideoConvert ptVideoConvert)
{
    PT_VideoConvert ptVideoOpr_tmp = g_ptVideoOprHead;//定义一个ptVideoOpr指向链表头

    while(ptVideoOpr_tmp)//遍历链表 看看ptVideoOpr是否已经注册过了
    {
        if(ptVideoOpr_tmp == ptVideoConvert)//若是出现相同节点，拒绝注册
        {
            return -1;
        }

        ptVideoOpr_tmp = ptVideoOpr_tmp->ptNext;//往下遍历
    }
    //新节点ptVideoConvert指向原头节点
    ptVideoConvert->ptNext = g_ptVideoOprHead;
    g_ptVideoOprHead = ptVideoConvert;
    return 0;
}

void ShowVideoConvert(void){

    int i = 0;
    PT_VideoConvert ptVideoOpr_tmp = g_ptVideoOprHead;
    while(ptVideoOpr_tmp){
        printf("%02d %s\n", i++, ptVideoOpr_tmp -> name);
        ptVideoOpr_tmp = ptVideoOpr_tmp -> ptNext;
    }
}

PT_VideoConvert GetVideoOpr (char *pcName){
    PT_VideoConvert ptVideoOpr_tmp = g_ptVideoOprHead;

    while(ptVideoOpr_tmp){

        if (strcmp(ptVideoOpr_tmp -> name, pcName) == 0)
        {
            return ptVideoOpr_tmp;
        }
        ptVideoOpr_tmp = ptVideoOpr_tmp -> ptNext;
    }

    return NULL;

}

PT_VideoConvert GetVideoConvertForFormats(int iPexelFormatIn, int iPexelFormatOut){
    int iErr;
    PT_VideoConvert ptTmp = g_ptVideoOprHead;
    while (ptTmp){

        if(ptTmp->is_support(iPexelFormatIn,iPexelFormatOut)){
            return ptTmp;
        }
        ptTmp = ptTmp -> ptNext;
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

int VideoConvertInit(void){
    int iErr;

    iErr = YUV2RGBInit();
    iErr |= MJPEG2RGBInit();
    iErr |= RGB2RGBInit();
    return iErr;
}