#ifndef _VIDEO_MANAGER_H
#define _VIDEO_MANAGER_H

#include <configs.h>
#include <pic_operation.h>

#define NB_BUFFER 4

struct VideoOprs;
struct VideoDevice;
typedef struct VideoDevice T_VideoDevice, *PT_VideoDevice;
typedef struct VideoOprs T_VideoOprs, *PT_VideoOprs;


typedef struct VideoOprs{
    int (*InitDevice)(char *strDevName, PT_VideoDevice ptVideoDev);
    int (*ExitDevice)(PT_VideoDevice ptVideoDev);
    int (*GetFrame)(PT_VideoDevice ptVideoDev,PT_VideoBuf ptVideoBuf);
    int (*GetFormat)(PT_VideoDevice ptVideoDev);
    int (*PutFrame)(PT_VideoDevice ptVideoDev, PT_VideoBuf ptVideoBuf);
    int (*StartDevice)(PT_VideoDevice ptVideoDev);
    int (*StopDevice)(PT_VideoDevice ptVideoDev);
    
}T_VideoOprs, *PT_VideoOprs;

typedef struct VideoDevice{

    int iFd;
    int iPixelFormat;
    int iWidth;
    int iHeight;
    /*BUF*/
    int iVideoBufcount;
    int iVideoBufMaxLen;
    int iVIdeoBufIndex;
    unsigned char *pucVideoBuf[NB_BUFFER];
    /*function pointers*/
    PT_VideoOprs ptVideoOprs;

}T_VideoDevice, *PT_VideoDevice;


/*video Buffer*/
typedef struct VideoBuf{
    T_PixelDatas tPixelDatas;
    int iPixelFormat; /*YUV RGB JPEG*/

}T_VideoBuf, *PT_VideoBuf;

/**
 * v4l2
*/
int videoDeviceInit(char *strDevName, PT_VideoDevice ptVideoDevice);
int V4l2Init(void);

#endif // 


