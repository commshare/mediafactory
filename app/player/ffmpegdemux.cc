#include "ffmpegdemux.h"

struct ffmpegdemuxdesc_t
{
	AVFormatContext *pFormatCtx;
    int audioStreamid;  
    int videoStreamid;  
    AVCodecContext *pAudioCodecContext;
    AVCodecContext *pVideoCodecContext;

	video_frame_callback vfcallback;
	audio_frame_callback afcallback;
};

/////////////////////////////////////////////

AVCodecContext *AV_GetCodecContext(AVFormatContext *pFormatCtx,AVMediaType MediaType,int &iStreamID)
{
	if( !pFormatCtx )
		return NULL;
	
	AVCodecContext *pCodecCtx=NULL;
	for(unsigned int i=0; i<pFormatCtx->nb_streams; i++) 
	{
		AVStream *pStream=pFormatCtx->streams[i];
		if( pStream )
		{
			if( pStream->codec->codec_type==MediaType )
			{
				pCodecCtx=pFormatCtx->streams[i]->codec; 
				if( !pCodecCtx )
					return NULL;

				iStreamID=i;
			}
		}
	}

	return pCodecCtx;
}

int AV_DecodeVideo(AVCodecContext *pCodecCtx, AVFrame *pFrameData, AVPacket *pPacket)
{
	if( !pCodecCtx || !pFrameData ||  !pPacket )
		return -1;

	int VideoDecodeFinished=0;
	avcodec_decode_video2(pCodecCtx, pFrameData, &VideoDecodeFinished, pPacket);
	if( VideoDecodeFinished )
	{
//		av_free_packet(pPacket);
		return 1;
	}
	return 0;
}

int AV_DecodeAudio(AVCodecContext *pCodecCtx,AVFrame *pFrameData,AVPacket *pPacket)
{
// 	int i=0,j=1;
// 	PRINT("i=%d,j=%d",i,j);

	if( !pCodecCtx || !pFrameData ||  !pPacket )
		return -1;

	int AudioDecodeFinished=0;
	avcodec_decode_audio4(pCodecCtx, pFrameData, &AudioDecodeFinished, pPacket);
	if( AudioDecodeFinished )
	{
		av_free_packet(pPacket);
		return 1;
	}
	return 0;
}
/////////////////////////////////////////////////////

void* ffmpegdemux_open(const char* url, video_frame_callback, audio_frame_callback)
{
	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	avcodec_register_all();
#if CONFIG_AVDEVICE
	avdevice_register_all();
#endif
#if CONFIG_AVFILTER
	avfilter_register_all();
#endif
	av_register_all();
	avformat_network_init();

	//--------------------------open video source,get format infomation
	AVFormatContext *pFormatCtx = NULL;//avformat_alloc_context();
	int ret = avformat_open_input(&pFormatCtx, url, NULL,NULL);
	if( ret != 0 ) 
		return NULL;

	AVDictionary *options=NULL;
	if(avformat_find_stream_info(pFormatCtx,&options)<0) 
		return NULL;

	av_dump_format(pFormatCtx, 0, url, 0);

    int audioStreamid = -1;  
    int videoStreamid = -1;  
    AVCodecContext *pAudioCodecContext = AV_GetCodecContext(inst->pFormatCtx, AVMEDIA_TYPE_AUDIO, &audioStreamid);
    AVCodecContext *pVideoCodecContext = AV_GetCodecContext(inst->pFormatCtx, AVMEDIA_TYPE_VIDEO, &videoStreamid);;
    if( !pAudioCodecContext && !pVideoCodecContext )
	{
		avformat_close_input(&pFormatCtx);
		return NULL;
	}

	ffmpegdemuxdesc_t *inst = new ffmpegdemuxdesc_t;
	if( !inst )
	{
		avformat_close_input(&pFormatCtx);
		return NULL;
	}

	inst->pFormatCtx = pFormatCtx;
	inst->pAudioCodecContext = pAudioCodecContext;
	inst->pVideoCodecContext = pVideoCodecContext;
	inst->audioStreamid = audioStreamid;
	inst->videoStreamid = videoStreamid;
	inst->vfcallback = vfcallback;
	inst->afcallback = afcallback;

	return inst;
}

int ffmpegdemux_eventloop(void* handle)
{
	ffmpegdemuxdesc_t *inst = (ffmpegdemuxdesc_t*)handle;

////////////////////////////////////////////////////////////////////////////////////
	AVPacket packet; 

	if( av_read_frame(inst->pFormatCtx, &packet) < 0 )
		return -1;

	if( packet.stream_index == inst.videostreamid )
	{
		if( inst->vfcallback )
		{
			inst->vfcallback(packet.data, packet.size, packet.pts, inst->pVideoCodecContext->codec_id);
		}
//			printf("VideoFmt.streamID = %d \n", VideoFmt.streamID);
	}
	else if( packet.stream_index == inst->audiostreamid )
	{
		if( inst->afcallback )
		{
			inst->afcallback(packet.data, packet.size, packet.pts, inst->pAudioCodecContext->codec_id);
		}
	}
	else
	{
		printf("unknown streamid found \n");
	}

	return 0;
}

int ffmpegdemux_close(void* handle)
{
	ffmpegdemuxdesc_t *inst = (ffmpegdemuxdesc_t*)handle;

	avformat_close_input(&inst->pFormatCtx);
	delete inst;

	return 0;
}

int AV_GetClock(AVStream *pStream,int64_t iPacketPts,double *pfClock)
{
	if( !pStream || iPacketPts<0 || !pfClock )
		return 0;

	*pfClock = av_q2d(pStream->time_base)*iPacketPts;
	return 1;
}
