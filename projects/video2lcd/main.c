#include <configs.h> 
#include <disp_manager.h>
#include <convert_manager.h>
#include <video_manager.h>
#include <render.h>


/**
 * 
 * video2lcd </dev/video0,video1>
*/
int main(int argc, char **argv){
    int iErr;

    T_VideoDevice tVideoDev;

    PT_VideoBuf ptVideoBufCur;
    PT_VideoConvert ptVideoConvert;
    PT_VideoConvert tConvertBuf; 
    T_VideoBuf tVideoBuf;
    T_VideoBuf tZoomBuf;
    T_VideoBuf tFrameBuf;



    int iPixelFormatofVideo;
    int iPixelFormatofDisplay;

    int iLcdWidth;
    int iLcdHeight;
    int iLcdBpp;

    flaot k;

    int iTopLeftX;
    int iTopLeftY;


    if(argc != 2){
        DBG_PRINTF("Usage:\n");
        DBG_PRINTF("%s <free_types_file>\n",argv[0]);
        return -1;
    }
    /*以系列的初始化*/

    //framebuffer
    DisplayInit();
    SelectAndInitDefaultDispDev('fb');
    GetDispResolution(&iLcdWidth, &iLcdHeight, &iLcdBpp);//在init的时候就要获得分辨率
    getVideoBufForDisplay(&tFrameBuf);


    VideoInit();//注册一个operation结构体
    iErr = videoDeviceInit(argv[1], &tVideoDev);
    if(!iErr){
        DBG_PRINTF("VideoDeviceInit for %s erro: \n",argv[1]);
        return -1;
    }

    iPixelFormatofVideo  = tVideoDev.ptVideoOprs->v4l2GetFormat();
    VideoConvertInit();
    ptVideoConvert = GetVideoConvertForFormats(iPixelFormatofVideo, iPixelFormatofDisplay);
    if(ptVideoConvert == NULL){
        DBG_PRINTF('can not support this format \n');
        return -1;
    }

    /* 启动摄像头设备*/
    iErr = tVideoDev.ptVideoOprs->StartDevice(&tVideoDev);
    if(!iErr){
        DBG_PRINTF("start device  for %s erro: \n",argv[1]);
        return -1;
    }    

    memset(&tVideoBuf, 0 , sizeof(tVideoBuf));//清空Buffer
    memset(&tConvertBuf, 0 , sizeof(tConvertBuf));//清空tConvertBuf
    tConvertBuf.tPixelDatas.Bpp = iLcdBpp;
    memset(&tZoomBuf, 0 , sizeof(tZoomBuf));//清空tConvertBuf

    while(1){
        /*读入摄像头的数据*/
        iErr = tVideoDev.ptVideoOprs->GetFrame(&tVideoDev, &tVideoBuf);
        if(!iErr){
            DBG_PRINTF("get frame for %s erro: \n",argv[1]);
            return -1;
        }   

        ptVideoBufCur = &tVideoBuf; 
        /*转换为RGB数据*/
        if(iPixelFormatofVideo != iPixelFormatofDisplay){
            iErr = ptVideoConvert->convert(&tVideoBuf, &tConvertBuf);
            if(!iErr){
                DBG_PRINTF("convert format for %s erro: \n",argv[1]);
                return -1;
            } 
            ptVideoBufCur = &tConvertBuf; 
            
        }

        /*如果图像分辨率大于LCD->缩放*/
        if(ptVideoBufCur->tPixelDatas.iWidth > iLcdWidth || ptVideoBufCur->tPixelDatas.iHeight > iLcdHeight){
            /*确定缩放后数据的分辨*/
            k  = (float) ptVideoBufCur->tPixelDatas.iHeight /ptVideoBufCur->tPixelDatas.iWidth;
            tZoomBuf.tPixelDatas.iWidth = iLcdWidth;
            tZoomBuf.tPixelDatas.iHeight = iLcdWidth * k;
            if(tZoomBuf.tPixelDatas.iHeight > iLcdHeight){
                tZoomBuf.tPixelDatas.iWidth = iLcdHeight / k;
                tZoomBuf.tPixelDatas.iHeight = iLcdHeight;
            }

            tZoomBuf.tPixelDatas.Bpp = iLcdBpp;
            tZoomBuf.tPixelDatas.iLineBytes = tZoomBuf.tPixelDatas.iWidth * tZoomBuf.tPixelDatas.Bpp / 8;
            tZoomBuf.tPixelDatas.iTotalBytes  = tZoomBuf.tPixelDatas.iLineBytes *tZoomBuf.tPixelDatas.iHeight;

            if (tZoomBuf.tPixelDatas.aucPixelDatas == NULL){
                tZoomBuf.tPixelDatas.aucPixelDatas = malloc(tZoomBuf.tPixelDatas.iTotalBytes);
            }

            tZoomBuf.tPixelDatas
            PicZoom(&ptVideoBufCur->tPixelDatas, &tZoomBuf.tPixelDatas);
            ptVideoBufCur = &tZoomBuf;
         }

        /*合并进frameBuffer*/
        iTopLeftX = (iLcdWidth- ptVideoBufCur->tPixelDatas.iWidth) / 2;
        iTopLeftY = (iLcdHeight- ptVideoBufCur->tPixelDatas.iHeight) / 2;
        PicMerge(iTopLeftX, iTopLeftY, &ptVideoBufCur->tPixelDatas, PT_PixelDatas );


        /*frameBuffer刷到lcd上->显示*/



    }

}

