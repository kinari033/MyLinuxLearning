
#include <configs.h>
#include <disp_manager.h>

static int FBDevInit(void);
static int FBShowPixel(int iX, int iY, unsigned int dwColor);
static int FBCleanScreen(unsigned int dwBackColor);
static int FBShowPage(PT_PixelDatas ptPixelDatas);


static int g_fd;

static struct fb_var_screeninfo g_tFBVar;
static struct fb_fix_screeninfo g_tFBFix;

static unsigned int g_dwScreenSize;
static unsigned char *g_pucFBMem;

static T_DispOpr g_tFBOpr = {
    .name          = "fb",
    .FBDevInit     = FBDevInit,
    .FBShowPixel   = FBShowPixel,
    .FBCleanScreen = FBCleanScreen,
    .FBShowPage    = FBShowPage,
};

static int FBDevInit(void){
    int ret;
    g_fd = open(FB_DEVICE_NAME, O_RDWR);
    if(g_fd < 0){
        DBG_PRINTF("open %s failed! errno=%d\n", FB_DEVICE_NAME, errno);
        return -1;
    }
    //获取一个宏 屏幕信息
    ret = ioctl(g_fd, FBIOGET_VSCREENINFO, &g_tFBVar);
    if (ret < 0 ){
        DBG_PRINTF("ioctl FBIOGET_VSCREENINFO failed! errno=%d\n", errno);
        DBG_PRINTF("ioctl cannot get fb_var_screeninfo\n");
        close(g_fd);
        return -1;

    }
    ret = ioctl(g_fd, FBIOGET_FSCREENINFO, &g_tFBVar);
    if (ret < 0 ){
        DBG_PRINTF("ioctl FBIOGET_FSCREENINFO failed! errno=%d\n", errno);
        DBG_PRINTF("ioctl cannot get fb_fix_screeninfo\n");
        close(g_fd);
        return -1;

    }
    g_dwScreenSize = g_tFBVar.xres * g_tFBVar.yres * g_tFBVar.bits_per_pixel / 8;
    DBG_PRINTF("fbdev init success! res:%dx%d, bpp:%d, screensize:%d\n",
                g_tFBVar.xres, g_tFBVar.yres, g_tFBVar.bits_per_pixel, g_dwScreenSize);
    g_pucFBMem = (unsigned char *)mmap(NULL, g_dwScreenSize,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, g_fd, 0);
    if (g_pucFBMem < 0){  
        DBG_PRINTF("mmap fbdev failed! errno=%d\n", errno);
        close(g_fd);
        return -1;
    }


    // close(g_fd);
    g_tFBOpr.iXRes = g_tFBVar.xres;
    g_tFBOpr.iYRes = g_tFBVar.yres;
    g_tFBOpr.iBpp  = g_tFBVar.bits_per_pixel;
    g_tFBOpr.iLineWidth  = g_tFBVar.xres * g_tFBVar.bits_per_pixel / 8;
    g_tFBOpr.pucFBMem = g_pucFBMem;


    return 0;

}
static int FBShowPixel(int iX, int iY, unsigned int dwColor){

    unsigned char *pucFB;
    unsigned short *pwFB16Bpp;
    unsigned int *pwFB32Bpp;
    unsigned short wColor16Bpp;/*565*/

    int iRed;
    int IGreen;
    int iBlue;


    if (iX >=g_tFBVar.xres ) || (iY >= g_tFBVar.yres)){
        DBG_PRINTF("FBShowPixel input error! iX=%d, iY=%d\n", iX, iY);
        return -1;

    }


    pucFB = g_pucFBMem + iY * g_tFBOpr.iLineWidth + iX * g_tFBOpr.iBpp / 8; 
    pwFB16Bpp = (unsigned short *)pucFB;
    pwFB32Bpp = (unsigned int *)pucFB;


    switch (g_tFBVar.bits_per_pixel){
        case 8:
            *pucFB = (unsigned char)dwColor;
            break;
        case 16:
            /*convert 888 to 565*/
            iRed = (dwColor >> (16+3)) & ox1f;
            iGreen = (dwColor >> (8+2)) & 0x3f;
            iBlue = (dwColor >> 3) & 0x1f;

            wColor16Bpp = (iRed << 11) | (iGreen << 5) | iBlue;
            *pwFB16Bpp = wColor16Bpp;
            break;
        case 32:
            *pwFB32Bpp = dwColor;
            break;
        default:
            DBG_PRINTF("FBShowPixel not support bpp=%d\n", g_tFBVar.bits_per_pixel);
            return -1;

    }
    return 0;
}
static int FBCleanScreen(unsigned int dwBackColor){

    unsigned char *pucFB;
    unsigned short *pwFB16Bpp;
    unsigned int *pwFB32Bpp;
    unsigned short wColor16Bpp;

    int iRed;
    int iGreen;
    int iBlue;
    int i = 0;

    pucFB = g_pucFBMem;
    pwFB16Bpp = (unsigned short *)pucFB;
    pwFB32Bpp = (unsigned int *)pucFB;

    switch (g_tFBVar.bits_per_pixel){
        case 8:
            memset(pucFB, dwBackColor, g_dwScreenSize);
            break;
        case 16:
            /*convert 888 to 565*/
            iRed = (dwColor >> (16+3)) & ox1f;
            iGreen = (dwColor >> (8+2)) & 0x3f;
            iBlue = (dwColor >> 3) & 0x1f;

            wColor16Bpp = (iRed << 11) | (iGreen << 5) | iBlue;
            while(i < g_dwScreenSize){
                *pwFB16Bpp = wColor16Bpp;
                pwFB16Bpp++;
                i += 2;

            }
            break;
        case 32:
            // memset(pwFB32Bpp, dwBackColor, g_dwScreenSize);
            while(i < g_dwScreenSize){
                *pwFB32Bpp = dwBackColor;
                pwFB32Bpp++;
                i += 4;

            }
            break;
        default:
            DBG_PRINTF("FBCleanScreen not support bpp=%d\n", g_tFBVar.bits_per_pixel);
            return -1;

    }
    return 0;
}
static int FBShowPage(PT_PixelDatas ptPixelDatas){
    if (g_tFBOpr.pucFBMem != ptPixelDatas->aucPixelDatas){
        memcpy(g_tFBOpr.pucFBMem, ptPixelDatas->aucPixelDatas, ptPixelDatas -> iTotalBytes);
    }

}

int FBInit(void){
    return RegisterDispOpr(&g_tFBOpr);

}

