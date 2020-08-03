#include"audiorec.h"

#include<QDebug>
#include<QString>
#define MAX_AUDIO_FRAME_SIZE 192000

int64_t lastReadPacktTime;
AVFilterContext *buffersinkCtx = NULL;
AVFilterContext *buffersrcCtx = NULL;
AVFilterGraph *filterGraph = NULL;
AVCodecContext*	outPutAudioEncContext = NULL;
int64_t audioCount = 0;
audiorec::audiorec()
{



}





int audiorec::StopcaptureAudio(){


}
int audiorec::StartcaptureAudio(){

     av_register_all();
     avformat_network_init();
     avfilter_register_all();
     avdevice_register_all();//Register Device
        pFormatAudioCtx = avformat_alloc_context();
        pFormatAudioCtx = avformat_alloc_context();

        AVInputFormat *ifmt=av_find_input_format("dshow");

        QString audiodevice=QStringLiteral("audio=麦克风 (Realtek High Definition Audio)");
        QByteArray audiobyte=audiodevice.toUtf8();
        char *src=audiobyte.data();

        AVDictionary* dict=nullptr;
        av_dict_set(&dict,"audio_buffer_size","20",0);
        if(avformat_open_input(&pFormatAudioCtx,src,ifmt,&dict)!=0){
            printf("Couldn't open input stream.\n");
            return -1;
        }

        if(avformat_find_stream_info(pFormatAudioCtx,NULL)<0)
        {
            printf("Couldn't find stream information.\n");
            return -1;
        }

        audioindex=-1;

        audioindex=av_find_best_stream(pFormatAudioCtx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
        if(audioindex==-1){
            printf("failed find best stream");

            return  -1;
        }

  pCodec=avcodec_find_decoder(pFormatAudioCtx->streams[0]->codec->codec_id);
    if(!pCodec){
        printf("find decoder failed");
        return -1;
    }

    if(avcodec_open2(pFormatAudioCtx->streams[0]->codec,pCodec,NULL)<0){
        printf("open decoder failed");
        return -1;
    }

    initAudioFilters();

    static AVCodec *encodec=NULL;
    encodec=avcodec_find_encoder(AV_CODEC_ID_AAC);
    static AVCodecContext *enctx=NULL;
    enctx=avcodec_alloc_context3(encodec);
    enctx->codec=encodec;
    enctx->sample_rate=48000;
    enctx->channel_layout=3;
    enctx->channels=2;
    enctx->sample_fmt=AV_SAMPLE_FMT_FLTP;
    enctx->codec_tag=0;
    enctx->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    if(avcodec_open2(enctx,encodec,NULL)<0){
        printf("open failed");
        return -1;
    }


const char *outfile="output.aac";
    AVFormatContext *outfmt=NULL;
    if(avformat_alloc_output_context2(&outfmt,NULL,NULL,outfile)<0){
       printf("fiailed alloc output");
        return  -1;
    }

    AVStream *outstream=avformat_new_stream(outfmt,enctx->codec);
    if(!outstream){
        printf("failed open ouput file");
        return  -1;
    }

    avcodec_copy_context(outstream->codec,enctx);

    if(avio_open(&outfmt->pb,outfile,AVIO_FLAG_WRITE)<0){
        printf("failed out file");
        return -1;
    }

    avformat_write_header(outfmt,NULL);


#if 0
    AVFrame* Frame = av_frame_alloc();
    Frame->nb_samples = codec_ctx->frame_size;
    Frame->format = codec_ctx->sample_fmt;
    Frame->channel_layout = codec_ctx->channel_layout;

    int size = av_samples_get_buffer_size(NULL, codec_ctx->channels, codec_ctx->frame_size,
        codec_ctx->sample_fmt, 1);
    uint8_t* frame_buf = (uint8_t *)av_malloc(size);
    avcodec_fill_audio_frame(Frame, codec_ctx->channels, codec_ctx->sample_fmt, (const uint8_t*)frame_buf, size, 1);
    int64_t in_channel_layout = av_get_default_channel_layout(codec_ctx->channels);


    AVPacket pkt;

    av_new_packet(&pkt, size);
    pkt.data = NULL;

    int got_frame = -1;
    int delayedFrame = 0;
    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    int audioCount = 0;
    const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
    AVFrame* Frame1 = av_frame_alloc();
#endif
    int loop = 1;
    int delayedFrame = 0;
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    AVFrame* pSrcAudioFrame = av_frame_alloc();
    int got_frame = 0;

    while (isStopTriggered!=true) {
        av_read_frame(pFormatAudioCtx, &packet);
        loop++;
        if (packet.stream_index == audioindex) {
            auto filterFrame = DecodeAudio(&packet, pSrcAudioFrame);

            if (filterFrame) {
                avcodec_encode_audio2(enctx, &pkt, filterFrame, &got_frame);
                if (got_frame) {
#if 1
                    auto streamTimeBase = outfmt->streams[pkt.stream_index]->time_base.den;
                    auto codecTimeBase = outfmt->streams[pkt.stream_index]->codec->time_base.den;
                    pkt.pts = pkt.dts = (1024 * streamTimeBase * audioCount) / codecTimeBase;
                    audioCount++;

                    auto inputStream = pFormatAudioCtx->streams[pkt.stream_index];
                    auto outputStream = outfmt->streams[pkt.stream_index];
                    av_packet_rescale_ts(&pkt, inputStream->time_base, outputStream->time_base);
#endif
                   // pkt.stream_index = out_stream->index;
                    av_interleaved_write_frame(outfmt, &pkt);
                    av_packet_unref(&pkt);
                    printf("output frame %3d\n", loop - delayedFrame);
                }
                else {
                    delayedFrame++;
                    av_packet_unref(&pkt);
                    printf("no output frame\n");
                }
            }
        }
        av_packet_unref(&packet);
    }
    flush_encoder(outfmt, 0);
    av_write_trailer(outfmt);
   // fclose(fp_audio);
    av_free(pSrcAudioFrame);
    avio_close(outfmt->pb);
    avformat_close_input(&pFormatAudioCtx);

    return 0;






}



int audiorec::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
        0x0020))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
            NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        /* mux encoded frame */
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}
 int  audiorec::select_channel_layout(const AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

 int select_sample_rate(const AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}


int audiorec::initAudioFilters()
{
    char args[512];
    int ret;
   const AVFilter *abuffersrc = avfilter_get_by_name("abuffer");
   const  AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    auto audioDecoderContext = pFormatAudioCtx->streams[0]->codec;
    if (!audioDecoderContext->channel_layout)
        audioDecoderContext->channel_layout = av_get_default_channel_layout(audioDecoderContext->channels);

    static const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE };
    static const  uint64_t out_channel_layouts[] = { audioDecoderContext->channel_layout, (uint64_t)-1 };
    static const int out_sample_rates[] = { audioDecoderContext->sample_rate , -1 };

    AVRational time_base = pFormatAudioCtx->streams[0]->time_base;
    filterGraph = avfilter_graph_alloc();
    filterGraph->nb_threads = 1;

    sprintf_s(args, sizeof(args),
        "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
        time_base.num, time_base.den, audioDecoderContext->sample_rate,
        av_get_sample_fmt_name(audioDecoderContext->sample_fmt), audioDecoderContext->channel_layout);

    ret = avfilter_graph_create_filter(&buffersrcCtx, abuffersrc, "in",
        args, NULL, filterGraph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
        return ret;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersinkCtx, abuffersink, "out",
        NULL, NULL, filterGraph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
        return ret;
    }

    ret = av_opt_set_int_list(buffersinkCtx, "sample_fmts", out_sample_fmts, -1,
        AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
        return ret;
    }

    ret = av_opt_set_int_list(buffersinkCtx, "channel_layouts", out_channel_layouts, -1,
        AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
        return ret;
    }

    ret = av_opt_set_int_list(buffersinkCtx, "sample_rates", out_sample_rates, -1,
        AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
        return ret;
    }

    /* Endpoints for the filter graph. */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrcCtx;;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersinkCtx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filterGraph, "anull",
        &inputs, &outputs, nullptr)) < 0)
        return ret;

    if ((ret = avfilter_graph_config(filterGraph, NULL)) < 0)
        return ret;

    av_buffersink_set_frame_size(buffersinkCtx, 1024);
    return 0;
}


AVFrame* audiorec::DecodeAudio(AVPacket* packet, AVFrame*pSrcAudioFrame)
{

    AVStream * stream = pFormatAudioCtx->streams[0];
    AVCodecContext* codecContext = stream->codec;
    int gotFrame;
    AVFrame *filtFrame = nullptr;
    auto length = avcodec_decode_audio4(codecContext, pSrcAudioFrame, &gotFrame, packet);
    if (length >= 0 && gotFrame != 0)
    {
        if (av_buffersrc_add_frame_flags(buffersrcCtx, pSrcAudioFrame, AV_BUFFERSRC_FLAG_PUSH) < 0) {
            av_log(NULL, AV_LOG_ERROR, "buffe src add frame error!\n");
            return nullptr;
        }

        filtFrame = av_frame_alloc();
        int ret = av_buffersink_get_frame_flags(buffersinkCtx, filtFrame, AV_BUFFERSINK_FLAG_NO_REQUEST);
        if (ret < 0)
        {
            av_frame_free(&filtFrame);
            goto error;
        }
        return filtFrame;
    }

error:
    return nullptr;
}

void audiorec::SetFlag(bool b){
    this->isStopTriggered=b;
}
