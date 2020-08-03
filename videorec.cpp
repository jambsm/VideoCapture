#include "videorec.h"
#include<QtDebug>
#include<include/libavutil/log.h>
#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
#define AVFMT_RAWPICTURE 0x0020
//'1' Use Dshow
//'0' Use VFW
#define USE_DSHOW 0
#define USE_H264BSF 0
#define USE_AACBSF 0
VideoRec::VideoRec()
{

}

void VideoRec::SetFlag(bool b){
    this->isStopTriggered=b;

}
int VideoRec::StopcaptureVideo(){
//isStopTriggered=true;
}

int VideoRec::StartcaptureVideo(){

    av_register_all();
    avformat_network_init();
    avdevice_register_all();//Register Device
    pFormatVideoCtx = avformat_alloc_context();
    const char *outfile="output.h264";
    av_log_set_level(AV_LOG_ERROR);
    //抓取屏幕
    AVFormatContext *out_formatctx=NULL;
    AVStream *in_stream;
   AVDictionary* dict = nullptr;
     av_dict_set(&dict, "max_delay", "100", 0);
     av_dict_set(&dict,"probesize","4*1024*512",0);
    AVInputFormat *ifmt=av_find_input_format("gdigrab");
    if(avformat_open_input(&pFormatVideoCtx,"desktop",ifmt,&dict)!=0){
            printf("Couldn't open input stream. ");
            return -1;
        }

    if(avformat_find_stream_info(pFormatVideoCtx,NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoindex=av_find_best_stream(pFormatVideoCtx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
        in_stream=pFormatVideoCtx->streams[videoindex];
        pCodecCtx=pFormatVideoCtx->streams[videoindex]->codec;
        pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec==NULL)
        {
            printf("Codec not found.\n");
            return -1;
        }


        if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
        {
            printf("Could not open codec.\n");
            return -1;
        }





        //编码过程
        AVFrame	*pFrame,*pFrameYUV;
        pFrame=av_frame_alloc();
        pFrameYUV=av_frame_alloc();
        pFrameYUV->format=pCodecCtx->pix_fmt;
        pFrameYUV->width=pCodecCtx->width;
        pFrameYUV->height=pCodecCtx->height;
        av_log(NULL,AV_LOG_DEBUG,"\nWidth:",pFrameYUV->width);
        av_log(NULL,AV_LOG_DEBUG,"\nHeight:",pFrameYUV->height);
        uint8_t *out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
        avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
        int ret, got_picture;

      // AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
       // AVPacket *packetH264=(AVPacket *)av_malloc(sizeof(AVPacket));
       AVPacket *packet=av_packet_alloc();
       AVPacket *packetH264=av_packet_alloc();

        FILE *fp_h264=fopen(outfile,"wb");



        //ret=avformat_write_header(out_formatctx,NULL);

        struct SwsContext *img_convert_ctx;
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        ///这里打印出视频的宽高
        fprintf(stderr,"w= %d h= %d\n",pCodecCtx->width, pCodecCtx->height);


        //Video H264编码
        pH264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
               if(!pH264Codec)
               {
                 fprintf(stderr, "---------h264 codec not found----\n");
                 exit(1);
               }
               pH264CodecCtx = avcodec_alloc_context3(pH264Codec);
               pH264CodecCtx->codec_id = AV_CODEC_ID_H264;
               pH264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
               pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
               pH264CodecCtx->width =1920;
               pH264CodecCtx->height = 1080;
               pH264CodecCtx->time_base.num = 1;
               pH264CodecCtx->time_base.den = 15;//帧率(既一秒钟多少张图片)
               pH264CodecCtx->bit_rate = 400000; //比特率(调节这个大小可以改变编码后视频的质量)
               pH264CodecCtx->gop_size=12;
               //H264 还可以设置很多参数 自行研究吧
               pH264CodecCtx->qmin = 10;
               pH264CodecCtx->qmax = 51;
               // some formats want stream headers to be separate
               if (pH264CodecCtx->flags & AVFMT_GLOBALHEADER)
                   pH264CodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
               // Set Option
               AVDictionary *param = 0;
               //H.264
               av_dict_set(&param, "preset", "fast", 0);
               av_dict_set(&param, "tune", "zerolatency", 0);  //实现实时编码
               if (avcodec_open2(pH264CodecCtx, pH264Codec,&param) < 0){
                 printf("Failed to open video encoder1! 编码器打开失败！\n");
                 return false;
               }



               ret=avformat_alloc_output_context2(&out_formatctx,NULL,NULL,outfile);
               AVStream *outstream=avformat_new_stream(out_formatctx,pH264CodecCtx->codec);
               avcodec_copy_context(outstream->codec,pH264CodecCtx);

               if(avio_open(&out_formatctx->pb,"output.h264",AVIO_FLAG_WRITE)<0){
                   printf("failed out file");
                   return -1;
               }

           av_dump_format(out_formatctx,0,outfile,1);


        //写入文件
int audioCount=0;

            while(isStopTriggered!=true)
        {
            if(av_read_frame(pFormatVideoCtx, packet) < 0)
            {
                break;
            }
            if(packet->stream_index==videoindex)
            {
                ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                if(ret < 0){
                    printf("Decode Error.\n");
                    return -1;
                }
                if(got_picture)
                {
                    sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                    int y_size=pCodecCtx->width*pCodecCtx->height;

                    //fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
                    //fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
                   // fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V



                    pFrameYUV->pkt_dts=pFrameYUV->pts=audioCount;
                    pFrameYUV->pkt_pos=-1;




                   //h264文件写入
                    /*int ret1 = avcodec_encode_video2(pH264CodecCtx, packet,pFrameYUV, &got_picture);
                                             if(ret1 < 0){
                                                 printf("Failed to encode! \n");
                                                 return -1;
                                          }
                                          */
                           avcodec_send_frame(pH264CodecCtx,pFrameYUV);
                           avcodec_receive_packet(pH264CodecCtx,packet);
                                          //av_interleaved_write_frame(out_formatctx,&packet);
                                         av_packet_rescale_ts(packet, in_stream->time_base, outstream->time_base);
                                          ret = fwrite(packet->data,1 , packet->size, fp_h264 );
                                          if (0 > ret)
                                          {
                                              printf("write into output.h264 fail\n");
                                          }

                audioCount++;

                }
            }
            av_packet_unref(packet);

        }


        sws_freeContext(img_convert_ctx);

       //fclose(fp_h264);
        av_free(out_buffer);
        av_free(pFrameYUV);
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatVideoCtx);
        avformat_close_input(&out_formatctx);
}


