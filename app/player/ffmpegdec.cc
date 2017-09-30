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
#include "ffmpegdec.h"

typedef struct 
{
 	AVCodecContext *pAudioCodecCtx;  
    AVCodec *pAudioCodec;
    AVFrame *pAudioFrame;  
    AVCodecContext *pVideoCodecCtx;  
    AVCodec *pVideoCodec;
    AVFrame *pVideoFrame;  

    video_raw_callback vrcallback;
    audio_raw_callback arcallback;
}ffmpegdeccontext_t;

int AV_DecodeVideo(AVCodecContext *pCodecCtx, AVFrame *pFrameData, AVPacket *pPacket)
{
    if( !pCodecCtx || !pFrameData ||  !pPacket )
        return -1;

    int VideoDecodeFinished=0;
    avcodec_decode_video2(pCodecCtx, pFrameData, &VideoDecodeFinished, pPacket);
    if( VideoDecodeFinished )
    {
        return 0;
    }
    return -2;
}

int AV_DecodeAudio(AVCodecContext *pCodecCtx,AVFrame *pFrameData,AVPacket *pPacket)
{
    if( !pCodecCtx || !pFrameData ||  !pPacket )
        return -1;

    int AudioDecodeFinished=0;
    avcodec_decode_audio4(pCodecCtx, pFrameData, &AudioDecodeFinished, pPacket);
    if( AudioDecodeFinished )
    {
        return 0;
    }
    return -2;
}

int AV_AllocCodecContext(int codecid, AVCodecContext **CodecCtx, AVCodec **Codec)
{
    /* find the video encoder */  
    AVCodec *pCodec = avcodec_find_decoder(codecid);//AV_CODEC_ID_H264);        
    if (!pCodec)   
    {  
        printf("codec not found! \n");  
        return -1;  
    }  
      
    //初始化参数，下面的参数应该由具体的业务决定  
    AVCodecContext *CodecCtx = avcodec_alloc_context3(pCodec);      
//    pCodecCtx->time_base.num = 1;  
//    pCodecCtx->frame_number = 1; //每包一个视频帧  
//    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
//    pCodecCtx->bit_rate = 0;  
//    pCodecCtx->time_base.den = 30;//帧率  
//    pCodecCtx->width = width;//视频宽  
//    pCodecCtx->height = height;//视频高  

    if(avcodec_open2(CodecCtx, pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }

    *CodecCtx = pCodecCtx;
    *Codec = pCodec;
    return 0;
}

//////////////////////////////////////////////////////////////
void* ffmpegdec_open(int video_codecid, int audiocodecid, video_raw_callback vrcallback, audio_raw_callback arcallback)
{
    //下面初始化h264解码库  
    av_register_all();  
    avcodec_register_all();  
    
    AVCodecContext *pVideoCodecCtx = NULL;
    AVCodec *pVideoCodec = NULL;
    if( AV_AllocCodecContext(video_codecid, &pVideoCodecCtx, &pVideoCodec) < 0 )
    {
        printf("AV_AllocCodecContext error \n");  
        return NULL;          
    }

    AVCodecContext *pAudioCodecCtx = NULL;
    AVCodec *pAudioCodec = NULL;
    if( AV_AllocCodecContext(audiocodecid, &pAudioCodecCtx, &pAudioCodec) < 0 )
    {
        printf("AV_AllocCodecContext error \n");  
        return NULL;          
    }

    ffmpegdeccontext_t *obj = new ffmpegdeccontext_t;
    if( !obj )
    {
        printf("malloc object failed \n");  
        return NULL;  
    }
    obj->vrcallback = vrcallback;
    obj->arcallback = arcallback;
    obj->pVideoFrame = obj->pAudioFrame = NULL;  

    if( pVideoCodecCtx && pVideoCodec )
    {
        obj->pVideoFrame = av_frame_alloc();  
        obj->pVideoCodecCtx = pVideoCodecCtx;
        obj->pVideoCodec = pVideoCodec;        
    }

    if( pAudioCodecCtx && pAudioCodec )
    {
        obj->pAudioFrame = av_frame_alloc();  
        obj->pAudioCodecCtx = pAudioCodecCtx;
        obj->pAudioCodec = pAudioCodec;        
    }

    return obj;
}

int ffmpegdec_decode(void *inst, int codecid, uint8_t *pBuffer, int dwBufsize)
{
    ffmpegdeccontext_t *obj = (ffmpegdeccontext_t*) inst;

    AVPacket packet = {0};        
    av_init_packet(&packet);
    packet.data = pBuffer;//这里填入一个指向完整H264数据帧的指针  
    packet.size = dwBufsize;//这个填入H264数据帧的大小  
      
    if( obj->pVideoCodecCtx && obj->pVideoCodecCtx->codecid == codecid )
    {
        int ret = AV_DecodeVideo(obj->pVideoCodecCtx, obj->pVideoFrame, &packet);
        if( !ret && obj->vrcallback)
        {
            obj->vrcallback(obj->pVideoFrame->data, obj->pVideoFrame->linesize);
        }
    }
    else if( obj->pAudioCodecCtx && obj->pAudioCodecCtx->codecid == codecid )
    {
        int ret = AV_DecodeAudio(obj->pAudioCodecCtx, obj->pAudioFrame, &packet);
        if( !ret && obj->arcallback )
        {
            obj->arcallback(obj->pAudioFrame->data, obj->pAudioFrame->linesize);
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

int ffmpegdec_close(void* inst)
{
	if( !inst )
	{
		printf("decoder handle is null \n");
		return -1;
	}
	
	ffmpegdeccontext_t *obj = (ffmpegdeccontext_t*)inst;

	if( obj->pVideoCodecCtx )
	{
		avcodec_close(obj->pVideoCodecCtx);
		avcodec_free_context(&(obj->pVideoCodecCtx));
	}
    if( obj->pAudioCodecCtx )
    {
        avcodec_close(obj->pAudioCodecCtx);
        avcodec_free_context(&(obj->pAudioCodecCtx));
    }

	if( obj->pVideoFrame )
		av_frame_free(&(obj->pVideoFrame));
    if( obj->pAudioFrame )
        av_frame_free(&(obj->pAudioFrame));

	return 0;
}
