
#include <configs.h>
#include <video_manager.h>



static struct global_ supported_fmts[] ={
    V4L2_PIX_FMT_YUYV,
    V4L2_PIX_FMT_MJPEG,
    V4L2_PIX_FMT_RGB565,
    // V4L2_PIX_FMT_RGB24,
    // V4L2_PIX_FMT_RGB32,
};

static int v4l2GetFrameForReadWrite(PT_VideoDevice ptVideoDev,PT_VideoBuf ptVideoBuf);
static int v4l2PutFrameForReadWrite(PT_VideoDevice ptVideoDev, PT_VideoBuf ptVideoBuf);
static T_VideoOprs g_tV4l2VideoOprs;

static int is_supported_format(unsigned int fmt){

    int i;
    for (i = 0; i < sizeof(supported_fmts)/sizeof(supported_fmts[0]); i++){
        if (supported_fmts[i].fmt == fmt){
            return 1;
        }
    }
    return 0;
}
//构造VideoOprs 结构体
static T_VideoOprs g_tV4l2VideoOprs = {

    .name           = "v4l2",
    .InitDevice     = v4l2InitDevice,
    .ExitDevice     = v4l2ExitDevice,
    .GetFrame       = v4l2GetFrameForStreaming,
    .GetFormat      = v4l2GetFormat,
    .PutFrame       = v4l2PutFrameForStreaming,
    .StartDevice    = v4l2StartDevice,
    .StopDevice     = v4l2StopDevice,
};

//注册这个结构体
/**
 * name: v4l2Init
 * func: 注册v4l2视频设备操作函数
 * step: open
 *       VIDIOC_QUERYCAP    ->是否视屏捕捉设备，
 *       VIDIOC_EUM_FMT     ->查询支持哪种格式 （我们只支持YUV, RGB, MJPEG）
 *       VIDIOC_S_FMT       ->设置摄像头使用哪种格式
 *       VIDIOC_REQBUFS     ->申请缓冲区
 * //对于streaming的IO这么做：
 *       VIDIOC_QUERYBUF    ->确认每一个缓冲区的信息并 mmap到用户空间
 *       VIDIOC_QBUF        ->每一个buf放入队列，
 *       VIDIOC_STREAMON    ->启动设备
 *       poll               ->等待有数据
 *       VIDIOC_DQBUF       ->从队列中取出一个buf处理
 * 
 * //对于read/write IO：
 *      read               ->读取一帧数据 
 *      处理数据
 * 
 *       VIDIOC_STREAMOFF   ->关闭设备
*/
int v4l2Init(void){

    return RegisterVideoOpr(&g_tV4l2VideoOprs);
}

//函数实现, 在这个需要适配接口

static int v4l2InitDevice(char *strDevName, PT_VideoDevice ptVideoDev){

    int ifd;
    int iErr; 
    int i;
    //驱动源码的结构体
    struct v4l2_capability t_v4l2Cap;
    struct v4l2_fmtdesc t_v4l2FmtDesc;
    struct v4l2_format t_v4l2Fmt;
    struct v4l2_requesrbuffers t_v4l2ReqBufs;
    struct v4l2_buffer t_v4l2Buf;
    //打开设备 

    int iLcdWidth;
    int iLcdHeight;
    int iLcdBpp;

    ifd = open(strDevName, O_RDWR);
    if (ifd < 0){

        DBG_PRINTF("cannot open %s, Err(%d):%s \n", strDevName, errno, strerror(errno));

        return -1;
    }

    ptVideoDev->iFd = ifd;//方便exitDEvice函数关闭文件

    /*根据结构体 调用各种ioctl*/
    //兼容各种驱动格式
    
    iErr = ioctl(ifd, VIDIOC_QUERYCAP, &t_v4l2Cap);
    memset(&t_v4l2Cap, 0, sizeof(t_v4l2Cap));
    iErr = ioctl(ifd, VIDIOC_QUERYCAP, &t_v4l2Cap);
    if (iErr < 0){
        DBG_PRINTF("ioctl VIDIOC_QUERYCAP failed, Err(%d):%s \n", errno, strerror(errno));
        goto error_exit;
    }

    //判断是否为摄像头视屏
    if((t_v4l2Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0){
        DBG_PRINTF("%s is not a video capture device\n", strDevName);
        goto error_exit;
    }
    //判断是否为streaming模式
    if(t_v4l2Cap.capabilities & V4L2_CAP_STREAMING){
        DBG_PRINTF("%s is support streaming IO\n", strDevName);
    }
    //判断是否为read/write模式
    if(t_v4l2Cap.capabilities & V4L2_CAP_READWRITE){
        DBG_PRINTF("%s support read/write IO\n", strDevName);
    }
    //查询支持什么format？
    memset(&t_v4l2FmtDesc , 0 , sizeof(t_v4l2FmtDesc));
    t_v4l2FmtDesc.index = 0;
    t_v4l2FmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while((iErr = ioctl(ifd, VIDIOC_ENUM_FMT, &t_v4l2FmtDesc)) == 0){
        // DBG_PRINTF("  fmt %d: %s\n", t_v4l2FmtDesc.index, t_v4l2FmtDesc.pixelformat, (char *)&t_v4l2FmtDesc.description);
        // t_v4l2FmtDesc.index++;
        if(is_supported_format(t_v4l2FmtDesc.pixelformat)){
            ptVideoDev->iPixelFormat = t_v4l2FmtDesc.pixelformat;
            break;
        }
        t_v4l2FmtDesc.index++;

    }
    //要是没有支持的格式返回错误
    if (!ptVideoDev->iPixelFormat){
        DBG_PRINTF("%s has no supported format\n", strDevName);
        goto error_exit;
    }

    /*set format*/
    GetDispResolution(&iLcdWidth, &iLcdHeight, &iLcdBpp);

    memset(&t_v4l2Fmt, 0, sizeof(t_v4l2Fmt));
    t_v4l2Fmt.types = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    t_v4l2Fmt.fmt.pix.width = iLcdWidth;
    t_v4l2Fmt.fmt.pix.height = iLcdHeight;
    t_v4l2Fmt.fmt.pix.format = ptVideoDev ->iPixelFormat;
    t_v4l2Fmt.fmt.pix.field = V4L2_FIELD_ANY;
    /*如果某些参数不支持，会自己调节参数并返回应用程序
    */
    iErr = ioctl(ifd, VIDIOC_S_FMT, &t_v4l2Fmt);
    if (iErr < 0){
        DBG_PRINTF("ioctl VIDIOC_S_FMT failed, Err(%d):%s \n", errno, strerror(errno));
        goto error_exit;
    }
    ptVideoDev->iWidth = t_v4l2Fmt.fmt.pix.width;
    ptVideoDev->iHeight = t_v4l2Fmt.fmt.pix.height;
    // ptVideoDev->iBpp = t_v4l2Fmt.fmt.pix.bytesperpixel * 8;

    /*申请缓冲区*/
    memset(&t_v4l2ReqBufs, 0 ,sizeof(struct v4l2_requestbuffers));//初始化:对这个结构体清0 为什么要这一步操作？
    t_v4l2ReqBufs.count = NB_BUFFER;//申请4个缓冲区
    t_v4l2ReqBufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    t_v4l2ReqBufs.memory = V4L2_MEMORY_MMAP;
    iErr = ioctl(ifd, VIDIOC_REQBUFS, &t_v4l2ReqBufs);
    if (iErr < 0){
        DBG_PRINTF("ioctl VIDIOC_REQBUFS failed, Err(%d):%s \n", errno, strerror(errno));
        goto error_exit;
    }

    ptVideoDev->iVideoBufcount = t_v4l2ReqBufs.count;
    /*把 buf mmap 到应用(分配)用户程序空间*/ /*读写接口的话只需要read/write*/
    /*VIDIOC_QUERYBUF*/
    /*只有对于streaming  的设备才需要mmap*/
    /*使用for循环查询每一个分配的buff*/
    if (t_v4l2Cap.capabilities & V4L2_CAP_STREAMING){
        DBG_PRINTF("%s support streaming IO\n", strDevName);
        /*查询每一个buf的信息，再mmap*/

        for (i = 0; i < ptVideoDev->iVideoBufcount; i++){
            mmset(&t_v4l2Buf, 0, sizeof(struct v4l2_buffer));
            t_v4l2Buf.index     = i;
            t_v4l2Buf.type      = v4L2_BUF_TYPE_VIDEO_CAPTURE;
            t_v4l2Buf.memory    = v4L2_MEMORY_MMAP;
            // iErr = ioctl(ifd, );
            iErr = ioctl(ifd, VIDIOC_QUERYBUF, &v4l2_buffer);
            if (iErr < 0) {
                DBG_PRINTF("Unable to query buffer,err(%d): %s\n",errno,strerrno(errno));
                goto error_exit;
            }
        }
        ptVideoDev->iVideoBufMaxLen = v4l2_buffer.length;
        /*把buffer 映射到用户空间*/
        ptVideoDev->pucVideoBuf[i] = mmap(0,/*start anywhere*/
        v4l2_buffer.length, PORT_READ, MAP_SHARED, ifd,
        v4l2_buffer.m.offset);

        if (ptVideoDev->pucVideoBuf[i] = MAP_FAILED){
            DEG_PRINTF("Unable to map buffer,err(%d):%s\n",errno, strerrno(errno));
            goto error_exit;

        }
        /*把Buffer 放入队列*/
        /*VIDIOC_QBUF*/
        for (i = 0; i < ptVideoDev->iVideoBufcount; i++){
            mmset(&t_v4l2Buf, 0, sizeof(struct v4l2_buffer));
            t_v4l2Buf.index     = i;
            t_v4l2Buf.type      = v4L2_BUF_TYPE_VIDEO_CAPTURE;
            t_v4l2Buf.memory    = v4L2_MEMORY_MMAP;
            // iErr = ioctl(ifd, );
            iErr = ioctl(ifd, VIDIOC_QBUF, &v4l2_buffer);
            if (iErr < 0) {
                DBG_PRINTF("Unable to queue buffer,err(%d): %s\n",errno,strerrno(errno));
                goto error_exit;
            }
        }        
    }

    /*read/write IO*/
    /*只需要调用read函数 使用malloc分配内存*/
    if (t_v4l2Cap.capabilities & V4L2_CAP_READWRITE){
        DBG_PRINTF("%s support read/write IO\n", strDevName);

        g_tV4l2VideoOprs.GetFrame = v4l2GetFrameForReadWrite;
        g_tV4l2VideoOprs.PutFrame = v4l2PutFrameForReadWrite;

        /*查询每一个buf的信息，再mmap*/

        ptVideoDev->iVideoBufcount = 1;
        ptVideoDev->iVideoBufMaxLen = ptVideoDev->iWidth * ptVideoDev->iHeight * 4;
        //YUV 一个像素2个字节，MJEPG 更小，RGB三字节，不会超过四个字节
        ptVideoDev->pucVideoBuf[0] = malloc(ptVideoDev->iWidth * ptVideoDev->iHeight * 4);//分配内存
    }

    return 0;

error_exit:
    close(ifd);
    return -1;

}

static int v4l2ExitDevice(PT_VideoDevice ptVideoDev){    
    int i;
    for (i =  0; i < ptVideoDev->iVideoBufcount; i++){

        if(ptVideoDev->pucVideoBuf[i] ){
            munmap(ptVideoDev->pucVideoBuf[i], ptVideoDev->iVideoBufMaxLen);
            ptVideoDev->pucVideoBuf[i] = NULL;
        }
    }
    close(ptVideoDev->ifd);
    return 0;
}
    /*2 situation*/
    /*streaming*/ /*read write*/

static int v4l2GetFormat(PT_VideoDevice ptVideoDev){

    return ptVideoDev->iPixelFormat;
}

static int v4l2GetFrameForStreaming(PT_VideoDevice ptVideoDev,PT_VideoBuf ptVideoBuf){ 
    int iErr;
    struct pollfd tPollfd[1];
    struct v4l2_buffer t_v4l2Buf;

    tPollfd[0].fd = ptVideoDev -> iFd;
    tPollfd[0].events = POLLIN; //是否有POLLIN的事件（在驱动中）
    tPollfd[0].revents = POLLIN;


    /*poll函数确定是否有数据*/
    iErr= poll(tPollfd, 1, -1);//poll(struct pollfd tPollfd, nfds, timeout)
    if (iErr <= 0){
        DBG_PRIINTF("POLLED ERROR(%d): %s\n", errno, strerrno(errno));
        return -1;
    }
    /*VIDIOC_QBUF,从队列中取取出来*/
    mmset(&t_v4l2Buf, 0, sizeof(struct v4l2_buffer));
    ptVideoDev->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ptVideoDev->memory = V4L2_MEMORY_MMAP;
    iErr = ioctl(ptVideoDev->iFd, VIDIOC_QBUF, &t_v4l2Buf);
    if (iErr < 0 ){
        DBG_PRINTF("Unable to dequeue buffer , err(%d): %s\n", errno , strerrno(errno));
        return -1;

    }

    ptVideoDev->iVIdeoBufIndex              = t_v4l2Buf.index;//需要知道位置，为下一步的getBuffer
    ptVideoBuf->iPixelFormat                = ptVideoDev->iPixelFormat;
    ptVideoBuf->tPixelDatas.iWidth          = ptVideoDev->iWidth;
    ptVideoBuf->tPixelDatas.iHeight         = ptVideoDev->iHeight;
    ptVideoBuf->tPixelDatas.Bpp             =   (ptVideoDev->iPixelFormat == V4L2_PIX_FMT_YUYV) ? 16 : \
                                                (ptVideoDev->iPixelFormat == V4L2_PIX_FMT_MJPEG) ? : 0 \
                                                (ptVideoDev->iPixelFormat == V4L2_PIX_FMT_RGB565) : 16 ;
    ptVideoBuf->tPixelDatas.iLineBytes      = ptVideoDev->iWidth * ptVideoBuf->tPixelDatas.Bpp / 8;
    ptVideoBuf->tPixelDatas.iTotalBytes     = t_v4l2Buf.bytesused;
    ptVideoBuf->tPixelDatas.aucPixelDatas   = ptVideoDev->pucVideoBuf[t_v4l2Buf.index];

    return 0;
}

static int v4l2PutFrameForStreaming(PT_VideoDevice ptVideoDev, PT_VideoBuf ptVideoBuf){    
    int iErr;
    struct v4l2_buffer t_v4l2Buf;

    mmset(&t_v4l2Buf, 0, sizeof(struct v4l2_buffer));
    t_v4l2Buf.index = ptVideoDev->iVIdeoBufIndex;
    t_v4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ptVideoDev->memory = V4L2_MEMORY_MMAP;
    iErr = ioctl(ptVideoDev -> iFd, VIDIOC_QBUF, &t_v4l2Buf);
    if(iErr < 0){
        DBG_PRINTF("Unable to queue buf, err(%d): %s\n", errno, strerrno(errno));
        return -1;
    }



    return 0;
}

static int v4l2GetFrameForReadWrite(PT_VideoDevice ptVideoDev,PT_VideoBuf ptVideoBuf){ 
    int iErr;

    iErr = read(ptVideoDev->iFd, ptVideoDev->pucVideoBuf[0], ptVideoDev->iVideoBufMaxLen);
    if (iErr < 0){
        DBG_PRINTF("Unable to read IO, err(%d):%s\n", errno, strerrno(errno));
        return -1;
    }

    ptVideoDev->iVIdeoBufIndex              = t_v4l2Buf.index;//需要知道位置，为下一步的getBuffer
    ptVideoBuf->iPixelFormat                = ptVideoDev->iPixelFormat;
    ptVideoBuf->tPixelDatas.iWidth          = ptVideoDev->iWidth;
    ptVideoBuf->tPixelDatas.iHeight         = ptVideoDev->iHeight;
    ptVideoBuf->tPixelDatas.Bpp             =   (ptVideoDev->iPixelFormat == V4L2_PIX_FMT_YUYV) ? 16 : \
                                                (ptVideoDev->iPixelFormat == V4L2_PIX_FMT_MJPEG) ? : 0 \
                                                (ptVideoDev->iPixelFormat == V4L2_PIX_FMT_RGB565) : 16 ;
    ptVideoBuf->tPixelDatas.iLineBytes      = ptVideoDev->iWidth * ptVideoBuf->tPixelDatas.Bpp / 8;
    ptVideoBuf->tPixelDatas.iTotalBytes     = iErr;
    ptVideoBuf->tPixelDatas.aucPixelDatas   = ptVideoDev->pucVideoBuf[0];    
    return 0;
}

static int v4l2PutFrameForReadWrite(PT_VideoDevice ptVideoDev, PT_VideoBuf ptVideoBuf){    
    /*nothing need to do*/
    return 0;
}
/**
 * 
 * streamon
*/
static int v4l2StartDevice(PT_VideoDevice ptVideoDev){    
    int itype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iErr;
    iErr = ioctl(ptVideoDev->iFd, VIDIOC_STREAMON, &itype);
    if (ret < 0){
        DBG_PRINTF("Unable to %s capture, err(%d):%s\n","start", errno, strerrno(errno));
        return -1;
    }

    return 0;
}

static int v4l2StopDevice(PT_VideoDevice ptVideoDev){    
    int itype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iErr;
    iErr = ioctl(ptVideoDev->iFd, VIDIOC_STREAMOFF, &itype);
    if (ret < 0){
        DBG_PRINTF("Unable to %s capture, err(%d):%s\n","Stop", errno, strerrno(errno));
        return -1;
    }
    return 0;
}
