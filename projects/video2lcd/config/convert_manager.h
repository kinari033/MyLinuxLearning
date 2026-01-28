#ifndef _CONVERT_MANAGER_H
#define _CONVERT_MANAGER_H

// #include <configs.h>
#include <video_manager.h>
/*格式转换*/

typedef struct VideoConvertOprs {
    int (*is_support)(int iPixelFormat_In, int iPixelFormat_Out);
    int (*convert)(PT_VideoBuf ptVideoBuf_In, PT_VideoBuf ptVideoBuf_Out);
    int (*convertExit)( PT_VideoBuf ptVideoBuf_Out);//release buffer
};

PT_VideoConvert GetVideoConvertForFormats(int iPexelFormatIn, int iPexelFormatOut);
int VideoConvertInit(void);


#endif // !