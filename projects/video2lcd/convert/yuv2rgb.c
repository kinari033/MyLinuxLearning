#include <configs.h>
#include <convert_manager.h>



int  Yuv2Rgb_is_support(int iPixelFormat_In, int iPixelFormat_Out){

    if (iPixelFormat_In != V4L2_PIX_FMT_YUYV)
        return 0;

    if ((iPixelFormat_Out != V4L2_PIX_FMT_RGB565) && (iPixelFormat_Out != V4L2_PIX_FMT_RGB32)){
        return 0;
    }

    return 1;        
}

static unsigned int
Pyuv422torgb565(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned char *output_pt = output_ptr;

    unsigned int r, g, b;
    unsigned int color;
    
	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;
		r = R_FROMYV(Y,V);
		g = G_FROMYUV(Y,U,V); //b
		b = B_FROMYU(Y,U); //v

        r = r >> 3;
        g = g >> 2;
        b = b >> 3;
        color = (r << 11) | (g << 5) | b;
        *output_pt++ = color & 0xff;
        *output_pt++ = (color >> 8) & 0xff;
			
		r = R_FROMYV(Y1,V);
		g = G_FROMYUV(Y1,U,V); //b
		b = B_FROMYU(Y1,U); //v
		
        r = r >> 3;
        g = g >> 2;
        b = b >> 3;
        color = (r << 11) | (g << 5) | b;
        *output_pt++ = color & 0xff;
        *output_pt++ = (color >> 8) & 0xff;
	}
	
	return 0;
} 




/* translate YUV422Packed to rgb24 */

static unsigned int
Pyuv422torgb32(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned int *output_pt = (unsigned int *)output_ptr;

    unsigned int r, g, b;
    unsigned int color;

	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;

        r = R_FROMYV(Y,V);
		g = G_FROMYUV(Y,U,V); //b
		b = B_FROMYU(Y,U); //v
		/* rgb888 */
		color = (r << 16) | (g << 8) | b;
        *output_pt++ = color;
			
		r = R_FROMYV(Y1,V);
		g = G_FROMYUV(Y1,U,V); //b
		b = B_FROMYU(Y1,U); //v
		color = (r << 16) | (g << 8) | b;
        *output_pt++ = color;
	}
	
	return 0;
}

int  uv2Rgb_convert(PT_VideoBuf ptVideoBuf_In, PT_VideoBuf ptVideoBuf_Out){
    
    struct PT_PixelDatas ptPixelDatasIn  = &ptVideoBuf_In -> tPixelDatas;
    struct PT_PixelDatas ptPixelDatasOut = &ptVideoBuf_Out -> tPixelDatas;

    ptPixelDatasOut-> iWidth = ptPixelDatasIn->iWidth;
    ptPixelDatasOut-> iHeight = ptPixelDatasIn->iHeight;


    if(ptVideoBuf_Out -> iPixelFormat == V4l2_PIX_FMT_RGB565){
        ptPixelDatasOut->Bpp = 16;
        ptPixelDatasOut->iLineBytes = ptPixelDatasOut-> iWidth * ptPixelDatasOut-> iHeight / 8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes *  ptPixelDatasOut-> iHeight;
        /*如果存储数据是空的 使用malloc分配一个存储空间*/
        if (!ptPixelDatasOut -> aucPixelDatas){
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);

        }
        Pyuv422torgb565(
                ptPixelDatasIn-> aucPixelDatas, ptPixelDatasOut-> aucPixelDatas,
                ptPixelDatasOut-> iWidth , ptPixelDatasOut-> iHeight
        );
    }

    else if (ptVideoBuf_Out -> iPixelFormat == V4l2_PIX_FMT_RGB32){
        ptPixelDatasOut->Bpp = 32;
        ptPixelDatasOut->iLineBytes = ptPixelDatasOut-> iWidth * ptPixelDatasOut-> iHeight / 8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes *  ptPixelDatasOut-> iHeight;
        
        /*如果存储数据是空的 使用malloc分配一个存储空间*/
        if (!ptPixelDatasOut -> aucPixelDatas){
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);

        }
        Pyuv422torgb32(
            ptPixelDatasIn-> aucPixelDatas, ptPixelDatasOut-> aucPixelDatas,
            ptPixelDatasOut-> iWidth , ptPixelDatasOut-> iHeight
        );
    }
    return 0;
}

int Yuv2Rgb_convertExit( PT_VideoBuf ptVideoBuf_Out){
    if (ptVideoBuf_Out-> tPixelDatas.aucPixelDatas){
        free(ptVideoBuf_Out->tPixelDatas.aucPixelDatas);
        ptVideoBuf_Out->tPixelDatas.aucPixelDatas = NULL;

    }

    return 0;
}

/*定义设置一个结构体*/
static T_VideoConvert g_tYuv2RgbConvert = {
    .is_support = Yuv2Rgb_is_support,
    .convert = Yuv2Rgb_convert,
    .convertExit = Yuv2Rgb_convertExit,
};

extern void initLut(void)
int YUV2RGBInit(void){
    initLut();
    return RegisterVideoConvertOpr(&g_tYuv2RgbConvert);
} 