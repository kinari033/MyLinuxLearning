#include <configs.h>
#include <convert_manager.h>
int  Rgb2Rgb_is_support(int iPixelFormat_In, int iPixelFormat_Out){

    if (iPixelFormat_In != V4L2_PIX_FMT_RGB565)
        return 0;

    if ((iPixelFormat_Out != V4L2_PIX_FMT_RGB565) && (iPixelFormat_Out != V4L2_PIX_FMT_RGB32)){
        return 0;
    }

    return 1;        
}

static int Rgb2Rgb_convert(PT_VideoBuf ptVideoBuf_In, PT_VideoBuf ptVideoBuf_Out){

    struct PT_PixelDatas ptPixelDatasIn  = &ptVideoBuf_In -> tPixelDatas;
    struct PT_PixelDatas ptPixelDatasOut = &ptVideoBuf_Out -> tPixelDatas;

    ptPixelDatasOut-> iWidth = ptPixelDatasIn->iWidth;
    ptPixelDatasOut-> iHeight = ptPixelDatasIn->iHeight;

    int y;
    int x;
    int r,g,b;
    int color;
    unsigned short *pwSrc = ptPixelDatasIn->aucPixelDatas;
    unsigned int *pdwDest;

    if(ptVideoBuf_In->iPixelFormat != V4L2_PIX_FMT_RGB565){
        return -1;
    }


    if(ptVideoBuf_In->iPixelFormat == V4L2_PIX_FMT_RGB565){

        ptPixelDatasOut-> iWidth = ptPixelDatasIn->iWidth;
        ptPixelDatasOut-> iHeight = ptPixelDatasIn->iHeight;
        ptPixelDatasOut->Bpp = ptPixelDatasIn->Bpp;
        ptPixelDatasOut->iLineBytes = ptPixelDatasOut-> iWidth * ptPixelDatasOut->Bpp /8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes *ptPixelDatasOut->iHeight;

        /*分配内存*/
        if(!ptPixelDatasOut->aucPixelDatas){
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);

        }
            /*memory copy*/
        memcpy(ptPixelDatasOut->aucPixelDatas, ptPixelDatasIn->aucPixelDatas, ptPixelDatasOut->iTotalBytes);
        
        return 0;
    }

    if(ptVideoBuf_In->iPixelFormat == V4L2_PIX_FMT_RGB32){

        ptPixelDatasOut-> iWidth = ptPixelDatasIn->iWidth;
        ptPixelDatasOut-> iHeight = ptPixelDatasIn->iHeight;
        ptPixelDatasOut->Bpp = 32;
        ptPixelDatasOut->iLineBytes = ptPixelDatasOut-> iWidth * ptPixelDatasOut->Bpp /8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes *ptPixelDatasOut->iHeight;

        /*分配内存*/
        if(!ptPixelDatasOut->aucPixelDatas){
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);

        }

        /*处理数据*/
        /*逐行处理*/
        pdwDest = (unsigned int *)ptPixelDatasOut->aucPixelDatas;//目的地址
        for (y = 0; y < ptPixelDatasOut->iHeight; y++){
            for(x = 0; x < ptPixelDatasOut -> iWidth; x++){
                /*把每一个像素取出并处理*/
                color = *pwSrc++
                /*从rgb565格式（16位的数据）中提取出r,g,b*/
                r = color >> 11;
                g = (color >> 5)&(0x3f); //左移五位并保留其中的6位
                b = color & 0x1f;
                /*把r,g,b转换为RGB32  0xRRGGBB*/
                color = ((r << 3) << 16) | ( (g << 2) << 8) | (b << 3);
                /*32位写入目的地址*/
                *pdwDest = color++;
            }
        }
        return 0;
    }
    return -1;
}

static int Rgb2Rgb_convertExit( PT_VideoBuf ptVideoBuf_Out){

    //释放内存
    if (ptVideoBuf_Out-> tPixelDatas.aucPixelDatas){
        free(ptVideoBuf_Out->tPixelDatas.aucPixelDatas);
        ptVideoBuf_Out->tPixelDatas.aucPixelDatas = NULL;

    }
    return 0;
}


static T_VideoConvert g_tRgb2RgbConvert = {
    .is_support = Rgb2Rgb_is_support,
    .convert = Rgb2Rgb_convert,
    .convertExit =  Rgb2Rgb_convertExit,
};




extern void initLut(void)
int Rgb2RGBInit(void){
    initLut();
    return RegisterVideoConvertOpr(&g_tRgb2RgbConvert);
} 