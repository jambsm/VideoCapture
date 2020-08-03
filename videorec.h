#ifndef VIDEOREC_H
#define VIDEOREC_H
extern "C"
{
    #include "include/libavcodec/avcodec.h"
    #include "include/libavformat/avformat.h"
    #include "include/libswscale/swscale.h"
    #include "include/libavdevice/avdevice.h"
    #include"include/libavutil/imgutils.h"
}

class VideoRec
{
public:
    VideoRec();
    int StartcaptureVideo();
    int StopcaptureVideo();
    void SetFlag(bool b);
private:


    AVFormatContext	*pFormatVideoCtx;
    int				i, videoindex;
    AVCodecContext	*pCodecCtx;
    AVCodec			*pCodec;
    AVCodecContext	*pH264CodecCtx;
    AVCodec		*pH264Codec;
    int Audioindex;
    bool isStopTriggered=false;
};

#endif // VIDEOREC_H
