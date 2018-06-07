#include "ffmpeg_enc.h"
#include <stdio.h>
#include <string>

#ifdef  __cplusplus    
extern "C" {    
#endif    
    #include <libavcodec/avcodec.h>  
    #include <libavformat/avformat.h>  
    #include <libavutil/imgutils.h>
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/avutil.h>
#ifdef  __cplusplus    
}    
#endif

struct ffmpeg_enc_tag_t
{
	std::string videoframe,audioframe;
 	AVCodecContext *pCodecCtx;
 	AVCodec *pCodec;
    AVFrame *video_frame;
    AVFrame *audio_frame;
};

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}
static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

/*
    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }
*/
    return frame;
}
//////////////////////////////////////////////////////////////
void *ffmpeg_enc_alloc(int codecid)
{
    av_register_all();
    av_log_set_level(AV_LOG_DEBUG);
    
    /* find the video encoder */  
    codecid = AV_CODEC_ID_AAC;
    AVCodec *pCodec = avcodec_find_encoder((AVCodecID)codecid);//AV_CODEC_ID_H264);        
    if (!pCodec)   
    {  
        printf("codec not found! \n");  
        return NULL;  
    }  
      
    //初始化参数，下面的参数应该由具体的业务决定  
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);    

	ffmpeg_enc_tag_t *inst = new ffmpeg_enc_tag_t;
	inst->pCodecCtx = pCodecCtx;
	inst->pCodec = pCodec;
    inst->audio_frame = NULL;
    inst->video_frame = NULL;
    return inst;
}

int ffmpeg_enc_set_video(void *handle, int width, int height, int pixel_format)
{      
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

	//初始化参数，下面的参数应该由具体的业务决定  
    AVCodecContext *pCodecCtx = inst->pCodecCtx;
//    pCodecCtx->time_base.num = 1;  
//    pCodecCtx->frame_number = 1; //每包一个视频帧  
//    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
//    pCodecCtx->bit_rate = 0;  
//    pCodecCtx->time_base.den = 30;//帧率  
//    pCodecCtx->width = width;//视频宽  
//    pCodecCtx->height = height;//视频高  

    if(avcodec_open2(pCodecCtx, inst->pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }

    inst->video_frame = alloc_picture(AV_PIX_FMT_YUV420P, width, height);

    return 0;
}

int ffmpeg_enc_set_audio(void *handle, int sample_rate, int nb_samples, int channels)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    //初始化参数，下面的参数应该由具体的业务决定  
	AVCodecContext *pCodecCtx = inst->pCodecCtx;
//    pCodecCtx->time_base.num = 1;  
//    pCodecCtx->frame_number = 1; //每包一个视频帧  
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;  
    pCodecCtx->codec_id = AV_CODEC_ID_AAC;
//    pCodecCtx->bit_rate = 0;  
//    pCodecCtx->time_base.den = 30;//帧率  
//    pCodecCtx->width = width;//视频宽  
//    pCodecCtx->height = height;//视频高  

    pCodecCtx->bit_rate = 64000;  
    pCodecCtx->sample_rate = sample_rate;
    pCodecCtx->channels = channels;
//    output_codec_context->channel_layout = av_get_default_channel_layout(output_codec_context->channels);
    pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    
    if(avcodec_open2(pCodecCtx, inst->pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }
    
    inst->audio_frame = av_frame_alloc();
    inst->audio_frame->nb_samples= pCodecCtx->frame_size;
    inst->audio_frame->format= pCodecCtx->sample_fmt;

//    inst->audio_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO,sample_rate, nb_samples);

    int size = av_samples_get_buffer_size(NULL, pCodecCtx->channels,pCodecCtx->frame_size,pCodecCtx->sample_fmt, 1);
    uint8_t * frame_buf = (uint8_t *)av_malloc(size);
    avcodec_fill_audio_frame(inst->audio_frame, pCodecCtx->channels, pCodecCtx->sample_fmt,(const uint8_t*)frame_buf, size, 1);

    return 0;
}

int ffmpeg_enc_encode_audio(void *handle, const char* framedata, int length, 
            const char **packetdata, int *packetlength)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;
    if( !inst->audio_frame )
        return -1;

    memcpy(inst->audio_frame->data[0],framedata, length);
    printf("avcodec_encode_audio2 %d %d \n", inst->audio_frame->linesize[0], length);

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);
    int got_packet;
    int ret = avcodec_encode_audio2(inst->pCodecCtx, &pkt, inst->audio_frame, &got_packet);
    if (ret < 0) {
        fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
        return -1;
    }

    if( !got_packet )
        return -2;

    inst->audioframe.clear();
    inst->audioframe.append((const char*)pkt.data, pkt.size);

    *packetdata = inst->audioframe.data();
    *packetlength = inst->audioframe.size();

    av_free_packet(&pkt);
    return 0;
}

int ffmpeg_enc_encode_video(void *handle, const char* framedata, int length, 
            const char **packetdata, int *packetlength)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    int got_packet;
    int ret = avcodec_encode_video2(inst->pCodecCtx, &pkt, inst->video_frame, &got_packet);
    if (ret < 0) {
        fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
        return -1;
    }

    if( !got_packet )
        return -2;

    inst->videoframe.clear();
    inst->videoframe.append((const char*)pkt.data, pkt.size);

    *packetdata = inst->videoframe.data();
    *packetlength = inst->videoframe.size();
    av_free_packet(&pkt);

    return 0;

}

int ffmpeg_enc_destroy(void *handle)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;
	if( inst->pCodecCtx )
	{
		avcodec_close(inst->pCodecCtx);
		avcodec_free_context(&(inst->pCodecCtx));
	}

    if( inst->video_frame )
        av_frame_free(&inst->video_frame);
    if( inst->audio_frame )
        av_frame_free(&inst->audio_frame);
	delete inst;
	return 0;	
}

