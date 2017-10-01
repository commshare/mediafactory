#include <list>
#include <atomic>
#include <deque>
#include <unistd.h>

#include "WrapperLibAV.h"
#include "WrapperSDL.h"

#include "player.h"
#include "rescale.h"
#include "resample.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

void* sdlhandle = NULL;
void* rescalehandle = NULL;
image_rescaled image;
void* resamplehandle = NULL;
sound_resampled soundframe;

struct PACKETINFO
{
	PACKETINFO(){iUsage=0;}
	PACKETINFO( const PACKETINFO &packet )
	{
		memcpy(data,packet.data,sizeof(data));
		iSize=packet.iSize;
		iUsage=packet.iUsage;
		ipts=packet.ipts;
	}
	Uint8 data[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	int iSize;
	int iUsage;
	int ipts;
};

template<typename ItemType>
class TQueue
{
public:
	TQueue(){m_pMutex=NULL;m_pCond=NULL;}
	~TQueue()
	{
		if( m_pMutex )
		{
			SDL_DestroyMutex(m_pMutex);
		}
		if( m_pCond )
		{
			SDL_DestroyCond(m_pCond);
		}
	}
	int Init()
	{
		m_lQueueSize=0;
		m_pMutex=SDL_CreateMutex();
		m_pCond=SDL_CreateCond();
		if( m_pMutex && m_pCond )
			return 1;
		else
			return 0;
	}
	void Push(ItemType val)
	{
		Lock();
		m_queueItems.push(val);
		m_lQueueSize++;
		UnLock();
	}
	void Pop(void)
	{
		Lock();
		m_queueItems.pop();
		m_lQueueSize--;
		UnLock();
	}
	int Get(ItemType **pPacket)
	{
		int iResult=0;
		Lock();
		if( m_queueItems.size() )
		{
			*pPacket=&m_queueItems.front();
			iResult=1;
		}
		UnLock();

		return iResult;
	}
	uint64_t Size()
	{
		return m_lQueueSize;
	}
	int Stream_Push(ItemType val)
	{
		m_listItems.push_back(val);
		m_lQueueSize++;
		return 1;
	}
	int Stream_Pop()
	{
		if( m_lQueueSize >= 2 )
		{
			m_listItems.pop_front();
			m_lQueueSize--;
			return 1;
		}
		return 0;
	}
	int Stream_Get(ItemType **pPacket)
	{
		int iResult=0;
		if( m_lQueueSize >= 2 )
		{
			*pPacket=&m_listItems.front();
			iResult=1;
		}
		return iResult;
	}
	int Signal(void)
	{
		return SDL_CondSignal(m_pCond);
	}
	int WaitForSignal()
	{
		return SDL_CondWait(m_pCond,NULL);
	}
private:
	int Lock()
	{
		return SDL_LockMutex(m_pMutex);
	}
	int UnLock()
	{
		return SDL_UnlockMutex(m_pMutex);
	}
private:
	std::deque<ItemType> m_queueItems;
	std::list<ItemType> m_listItems;
	SDL_mutex *m_pMutex;
	SDL_cond *m_pCond;
	std::atomic<uint64_t> m_lQueueSize; 
};

typedef struct
{
	AVCodecContext *pAudioCodecCtx;
	AVCodecContext *pVideoCodecCtx;

	volatile double lfAudioClock;
	volatile double lfVideoClock;

	AVStream *pAudioStream;
	AVStream *pVideoStream;

	AVFrame *pAudioFrame;
	AVFrame *pVideoFrame;

	TQueue<PACKETINFO> queueAudioFrame;
	TQueue<AVPacket> queueVideoFrame;
}VideoState;
VideoState g_VideoState;

void AudioCallback(void *unused, Uint8 *stream, int len)
{
	VideoState *pVideoState = (VideoState *)unused;
	Uint8 *pBuf=stream;
	int length=len;
	int iFilledSize=0;
	while( length>0 )
	{
		PACKETINFO *pPacket=NULL;
//		if( pVideoState->queueAudioFrame.Get(&pPacket) )
		if( pVideoState->queueAudioFrame.Stream_Get(&pPacket) )
		{
			double dClock=-1.0;
			AV_GetClock(pVideoState->pAudioStream,pPacket->ipts,&dClock);
//			printf("Audio Clock=%lf\n",dClock);
			pVideoState->lfAudioClock=dClock;

			int iPacketSize=pPacket->iSize-pPacket->iUsage;
			int len1=length-iPacketSize;
			if( len1>=0 )
			{
				memcpy(pBuf,pPacket->data+pPacket->iUsage,iPacketSize);
				pBuf+=iPacketSize;
				pVideoState->queueAudioFrame.Stream_Pop();
			}
			else if( len1<0 )
			{
				memcpy(pBuf,pPacket->data+pPacket->iUsage,length);
				pPacket->iUsage+=length;
			}
			length=len1;
		}
		else
		{
			memset(stream,0,len);
		}
	}
}

static uint8_t *audio_chunk;  
static uint32_t audio_len;  
static uint8_t *audio_pos;  
  
void fill_audio(void *udata,Uint8 *stream,int len){  
    SDL_memset(stream,0,len);  
    if(audio_len ==0){  
        return;  
    }  
    len=(len>audio_len?audio_len:len);  
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);  
    audio_pos+=len;  
    audio_len-=len;  
  
}  

int VideoThread(void *arg)
{
	if( !arg )return 0;

	VideoState *pVideoState=(VideoState*)arg;
	while( 1 )
	{
		if( pVideoState->pAudioCodecCtx && pVideoState->lfVideoClock > pVideoState->lfAudioClock )
		{
			usleep(10 * 1000);
			continue;
		}

		AVPacket *packet=NULL;
		if( pVideoState->queueVideoFrame.Stream_Get(&packet) )
		{
			if( AV_DecodeVideo(pVideoState->pVideoCodecCtx,pVideoState->pVideoFrame,packet) )
			{
				double dClock=-1.0;
				AV_GetClock(pVideoState->pVideoStream,packet->pts,&dClock);
//				printf("Video Clock=%lf\n",dClock);
//				printf("Video pixel format1=%d\n",pVideoState->pVideoCodecCtx->pix_fmt);
//				printf("pVideoState->pVideoFrame width=%d\n", pVideoState->pVideoFrame->width);

				pVideoState->lfVideoClock=dClock;

				rescale_image(rescalehandle, pVideoState->pVideoFrame->data, pVideoState->pVideoFrame->linesize, &image);
				SDL_DisplayYUV(sdlhandle, image.data, image.linesize);
			}
			av_free_packet(packet);
			pVideoState->queueVideoFrame.Stream_Pop();
		}
		usleep(10 * 1000);
	}
	return 1;
}

int InitializeVideoCodecFormat(MediaFormat *format,VideoState *state)
{
	//get stream type ID and decoders
	format->streamID = -1;
	format->tryInitTimes++;
	AVCodecContext *pVideoCodecCtx=AV_GetCodecContext(format->pFormatCtx,AVMEDIA_TYPE_VIDEO,format->streamID);
	if( !pVideoCodecCtx )
	{
		return -1;
	}

	//-----------------show every video frame
	AVFrame *pFrameYUV = av_frame_alloc(); 
	if( !pFrameYUV )
		return -1;

	rescalehandle = rescale_open(pVideoCodecCtx->width,pVideoCodecCtx->height,pVideoCodecCtx->pix_fmt);
	if( SDL_InitializeVideo(sdlhandle, "wwl", pVideoCodecCtx->width,pVideoCodecCtx->height) < 0 )
	{
		printf("SDL_InitializeVideo error \n");
		return -1;
	}

	state->pVideoCodecCtx=pVideoCodecCtx;
	state->pVideoFrame=pFrameYUV;
	state->lfVideoClock=0.0;
	state->pVideoStream=format->pFormatCtx->streams[format->streamID];
	if( state->queueVideoFrame.Init() )
		SDL_CreateThread(VideoThread, "", state);

	format->pCodecCtx = pVideoCodecCtx;
	format->pFrame = pFrameYUV;

	return 0;
}

int InitializeAudioCodecFormat(MediaFormat *format,VideoState *state)
{
	format->tryInitTimes++;
	format->streamID = -1;
	AVCodecContext *pAudioCodecCtx=AV_GetCodecContext(format->pFormatCtx,AVMEDIA_TYPE_AUDIO,format->streamID);
	if( !pAudioCodecCtx )
		return -1;

	format->outSampleFmt = AV_SAMPLE_FMT_S16;
	format->pCodecCtx = pAudioCodecCtx;
	format->pFrame = av_frame_alloc(); 
	state->pAudioCodecCtx=pAudioCodecCtx;
	state->pAudioStream=format->pFormatCtx->streams[format->streamID];
	state->lfAudioClock=0.0;
	if (pAudioCodecCtx->channels == 1)
		format->outChannelLayout = AV_CH_LAYOUT_MONO;
	else
		format->outChannelLayout= AV_CH_LAYOUT_STEREO;
	format->outChannelNum = av_get_channel_layout_nb_channels(format->outChannelLayout);

	int in_channel_layout = av_get_default_channel_layout(pAudioCodecCtx->channels);  

	resamplehandle = resample_open(in_channel_layout, 
		pAudioCodecCtx->sample_fmt, pAudioCodecCtx->sample_rate);

	if( !state->queueAudioFrame.Init() )
		return -1;

//	if( SDL_InitializeAudio(sdlhandle, pAudioCodecCtx->sample_rate,pAudioCodecCtx->channels,state,fill_audio) < 0 )
	if( SDL_InitializeAudio(sdlhandle, pAudioCodecCtx->sample_rate,pAudioCodecCtx->channels,state,AudioCallback) < 0 )
	{
		printf("SDL_InitializeAudio error\n");
		return -1;
	}

	return 0;
}

int ShowSDL(std::string strFileName,void* hwnd)
{
	//initialize ffmpeg
	AV_Initialize();
	sdlhandle = SDL_Initialize(hwnd);
/////////////////////////////////////////////////////////////////////////////////////////
	//open video source
	AVFormatContext *pFormatCtx = AV_OpenVideoSource(strFileName.c_str());
	if (!pFormatCtx)
		return -1;

	//get stream type ID and decoders
	MediaFormat VideoFmt = {pFormatCtx,0};
//	InitializeVideoCodecFormat(&VideoFmt,&g_VideoState);

	MediaFormat AudioFmt = {pFormatCtx,0};
//	InitializeAudioCodecFormat(&AudioFmt,&g_VideoState);

////////////////////////////////////////////////////////////////////////////////////
	AVPacket packet; 
	while( 1 )
	{
		int iVideoLen,iAudioLen;
		if( (g_VideoState.pVideoCodecCtx && (iVideoLen=g_VideoState.queueVideoFrame.Size())>5) 
			&& (g_VideoState.pAudioCodecCtx && (iAudioLen=g_VideoState.queueAudioFrame.Size())>5)
			)
		{
			printf("audiolistsize=%d,videolistsize=%d\n",iAudioLen,iVideoLen);
			usleep(100 * 1000 );
			continue;
		}

		if( av_read_frame(pFormatCtx, &packet) < 0 )
			break;

		if( !VideoFmt.pCodecCtx && VideoFmt.tryInitTimes < 2 )
			InitializeVideoCodecFormat(&VideoFmt,&g_VideoState);
		if( !AudioFmt.pCodecCtx && !AudioFmt.tryInitTimes < 2 )
			InitializeAudioCodecFormat(&AudioFmt,&g_VideoState);

		if( packet.stream_index==VideoFmt.streamID )
		{
//			printf("VideoFmt.streamID = %d \n", VideoFmt.streamID);
			g_VideoState.queueVideoFrame.Stream_Push(packet);
		}
		else if( packet.stream_index==AudioFmt.streamID )
		{
			if( AV_DecodeAudio(AudioFmt.pCodecCtx,AudioFmt.pFrame,&packet) )
			{
				PACKETINFO info;
				info.ipts=packet.pts;
				resample_sound(resamplehandle, AudioFmt.pFrame->data, AudioFmt.pFrame->linesize, 
					AudioFmt.pFrame->nb_samples, &soundframe);

				memcpy(info.data, soundframe.data, soundframe.linesize);
				info.iSize=soundframe.linesize;
				g_VideoState.queueAudioFrame.Stream_Push(info);
			}
			av_free_packet(&packet);
		}

		usleep(10 * 1000);
	}

	//-------------------release resource
// 	if( pFrameYUV )
// 		av_free(pFrameYUV);
// 	if( pFrameAudio )
// 		av_free(pFrameAudio);
// 
// 	if( pResampleCtx )
// 		avresample_free(&pResampleCtx);
// 
// 	if( pVideoCodecCtx )
// 		avcodec_close(pVideoCodecCtx);
// 	if( pAudioCodecCtx )
// 		avcodec_close(pAudioCodecCtx);
// 	if( pFormatCtx )
// 		avformat_close_input(&pFormatCtx);

	return 1;
}

//////////////////////////////////server//////////////////////////////////////////////////////
void logFF(void *, int level, const char *fmt, va_list ap)
{
	printf(fmt, ap);
}

int OpenRtpStream(std::string strFileName, void* hwnd, std::string strProtocol, std::string strIP, int port, int localport)
{
	//initialize ffmpeg
	AV_Initialize();
	SDL_Initialize(hwnd);
	/////////////////////////////////////////////////////////////////////////////////////////
	//open video source
	av_log_set_callback(&logFF);

	AVFormatContext *pFormatCtx = AV_OpenVideoSource(strFileName.c_str());

	//get stream type ID and decoders
	MediaFormat VideoFmt = { pFormatCtx, 0 };
	InitializeVideoCodecFormat(&VideoFmt, &g_VideoState);

	int nVideoStreamID =-1;
	AVCodecContext *pVideoCodecCtx = AV_GetCodecContext(pFormatCtx, AVMEDIA_TYPE_VIDEO, nVideoStreamID);
	AVFormatContext *pRtpVideoFmtCtx = NULL;
	if (pVideoCodecCtx)
	{
		AV_InitOutputFmt2(pFormatCtx->streams[nVideoStreamID], strProtocol.c_str(), strIP.c_str(), port, localport, &pRtpVideoFmtCtx);
	}

	MediaFormat AudioFmt = { pFormatCtx, 0 };
	InitializeAudioCodecFormat(&AudioFmt, &g_VideoState);
	int nAudioStreamID = -1;
	AVCodecContext *pAudioCodecCtx = AV_GetCodecContext(pFormatCtx, AVMEDIA_TYPE_AUDIO, nAudioStreamID);
	AVFormatContext *pRtpAudioFmtCtx = NULL;
	if (pAudioCodecCtx)
	{
		AV_InitOutputFmt2(pFormatCtx->streams[nAudioStreamID], strProtocol.c_str(), strIP.c_str(), port, localport, &pRtpAudioFmtCtx);
	}

	////////////////////////////////////////////////////////////////////////////////////
	AVPacket packet;
	while (1)
	{
		if (av_read_frame(pFormatCtx, &packet) >= 0)
		{
			if (packet.stream_index == nVideoStreamID)
				AV_SendOutputPacket(pRtpVideoFmtCtx, pFormatCtx->streams[packet.stream_index], pRtpVideoFmtCtx->streams[0], &packet);

			if (packet.stream_index == nAudioStreamID)
				AV_SendOutputPacket(pRtpAudioFmtCtx, pFormatCtx->streams[packet.stream_index], pRtpAudioFmtCtx->streams[0], &packet);

			av_free_packet(&packet);
		}

		usleep(10 * 1000);
	}

	//-------------------release resource
	// 	if( pFrameYUV )
	// 		av_free(pFrameYUV);
	// 	if( pFrameAudio )
	// 		av_free(pFrameAudio);
	// 
	// 	if( pResampleCtx )
	// 		avresample_free(&pResampleCtx);
	// 
	// 	if( pVideoCodecCtx )
	// 		avcodec_close(pVideoCodecCtx);
	// 	if( pAudioCodecCtx )
	// 		avcodec_close(pAudioCodecCtx);
	// 	if( pFormatCtx )
	// 		avformat_close_input(&pFormatCtx);

	return 1;
}
