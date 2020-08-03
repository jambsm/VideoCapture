#ifndef AUDIOREC_H
#define AUDIOREC_H
#include<QString>
extern "C"
{
    #include "include/libavcodec/avcodec.h"
    #include "include/libavformat/avformat.h"
    #include "include/libswscale/swscale.h"
    #include "include/libavdevice/avdevice.h"
    #include "include/libavutil/audio_fifo.h"
    #include"include/libavfilter/avfilter.h"
    #include"include/libavfilter/buffersink.h"
    #include"include/libavfilter/buffersrc.h"


}

class audiorec
{
public:
    audiorec();
    int StartcaptureAudio();
    int StopcaptureAudio();
    void SetFlag(bool b);
    bool isStopTriggered=false;
    int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);

     int select_channel_layout(const AVCodec *codec);
      int select_sample_rate(const AVCodec *codec);
    int initAudioFilters();
    AVFrame* DecodeAudio(AVPacket* packet, AVFrame*pSrcAudioFrame);
private:
      AVFormatContext	*pFormatAudioCtx;
       int				i, audioindex;
       AVCodecContext	*pCodecCtx;
       AVCodec			*pCodec;


};

#endif // AUDIOREC_H
