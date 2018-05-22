#include "ffmpeg_enc.h"
#include <stdio.h>
#include <string>

#ifdef  __cplusplus    
extern "C" {    
#endif    
    #include <libavcodec/avcodec.h>  
    #include <libavformat/avformat.h>  
    #include <libswscale/swscale.h>  
    #include <libavutil/imgutils.h>
#ifdef  __cplusplus    
}    
#endif

struct ffmpeg_enc_tag_t
{
	std::string frame;
 	AVCodecContext *pCodecCtx;
 	AVCodec *pCodec;
};

void *ffmpeg_enc_alloc(int codecid)
{
    av_register_all();
    /* find the video encoder */  
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

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }

    return 0;
}

int ffmpeg_enc_set_audio(void *handle, int width, int height, int pixel_format)
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

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }
    
    return 0;
}

int ffmpeg_enc_encode_audio(void *handle, const char* framedata, int length, const char **packetdata, int *packetlength)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);
    int got_packet;
    int ret = avcodec_encode_audio2(inst->pCodecCtx, &pkt, frame, &got_packet);
    if (ret < 0) {
        fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
        return -1;
    }

    return 0;
}

int ffmpeg_enc_encode_video(void *handle, const char* framedata, int length, const char **packetdata, int *packetlength)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    int got_packet;
    int ret = avcodec_encode_video2(inst->pCodecCtx, &pkt, frame, &got_packet);
    if (ret < 0) {
        fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
        return -1;
    }

    return 0;
}

int video_generator_destroy(void *handle)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;
	if( inst->pCodecCtx )
	{
		avcodec_close(inst->pCodecCtx);
		avcodec_free_context(&(inst->pCodecCtx));
	}

	delete inst;
	return 0;	
}
