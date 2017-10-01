#include <list>
#include <atomic>
#include <deque>
#include <unistd.h>
#include <thread>

#include "player.h"
#include "ffmpegdemux.h"
#include "ffmpegdec.h"
#include "WrapperSDL.h"

#include "rescale.h"
#include "resample.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

struct PACKETINFO
{
	PACKETINFO(){iUsage=0;}
	PACKETINFO( const PACKETINFO &packet )
	{
		memcpy(data,packet.data,sizeof(data));
		iSize=packet.iSize;
		iUsage=packet.iUsage;
		ipts=packet.ipts;
		streamid = packet.streamid;
		codecid = packet.codecid;
	}
	Uint8 data[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	int iSize;
	int iUsage;
	int ipts;
	int streamid;
	int codecid;
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
	volatile double lfAudioClock;
	volatile double lfVideoClock;

	TQueue<PACKETINFO> queueAudioFrame;
	TQueue<PACKETINFO> queueVideoFrame;
}VideoState;

VideoState g_VideoState;

void* sdlhandle = NULL;
void* rescalehandle = NULL;
image_rescaled image;
void* resamplehandle = NULL;
sound_resampled soundframe;
void* demuxhandle = NULL;
std::thread threadeventloop;

void AudioCallback(void *userdata, Uint8 *stream, int len)
{
	VideoState *pVideoState = (VideoState *)userdata;
	Uint8 *pBuf=stream;
	int length=len;
	int iFilledSize=0;
	while( length>0 )
	{
		PACKETINFO *pPacket=NULL;
//		if( pVideoState->queueAudioFrame.Get(&pPacket) )
		if( pVideoState->queueAudioFrame.Stream_Get(&pPacket) )
		{
			double dClock=-ffmpegdemux_getclock(demuxhandle, pPacket->streamid, pPacket->ipts);
			pVideoState->lfAudioClock=dClock;
			printf("Audio Clock=%lf %lf %d %d \n",pVideoState->lfAudioClock, dClock, pPacket->streamid, pPacket->ipts);

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

int VideoThread(void *arg)
{
	if( !arg )return 0;

	VideoState *pVideoState=(VideoState*)arg;
	ffmpegdemuxframe_t frame;

	while( 1 )
	{
		if( fabs(pVideoState->lfVideoClock - pVideoState->lfAudioClock) * 1000 < 500 )
		{
			printf("clock wait %lf %lf\n", pVideoState->lfVideoClock, pVideoState->lfAudioClock);
			usleep(10 * 1000);
			continue;
		}

		PACKETINFO *packet=NULL;
		if( pVideoState->queueVideoFrame.Stream_Get(&packet) )
		{
//			printf("videoframe size=%d \n", packet->iSize);
			int ret = ffmpegdemux_decode(demuxhandle, packet->codecid, packet->data, packet->iSize, &frame);
			if( ret < 0 )
			{
				pVideoState->queueVideoFrame.Stream_Pop();
				usleep(10 * 1000);
				continue;
			}	

			if( !rescalehandle )
			{
				rescalehandle = rescale_open(frame.info.image.width, frame.info.image.height, frame.info.image.pix_fmt);
				if( SDL_InitializeVideo(sdlhandle, "wwl", frame.info.image.width, frame.info.image.height) < 0 )
				{
					printf("SDL_InitializeVideo error \n");
					return -1;
				}					
			}

			double dClock=-ffmpegdemux_getclock(demuxhandle, packet->streamid, packet->ipts);
//			printf("Video Clock=%lf %d %d \n",dClock, packet->streamid, packet->ipts);
			pVideoState->lfVideoClock=dClock;

			rescale_image(rescalehandle, frame.data, frame.linesize, &image);
			SDL_DisplayYUV(sdlhandle, image.data, image.linesize);

			pVideoState->queueVideoFrame.Stream_Pop();
		}
		usleep(10 * 1000);
	}
	return 1;
}

int player_playurl(std::string url,void* hwnd)
{
	//initialize ffmpeg
	demuxhandle = ffmpegdemux_open(url.c_str());
	if( !demuxhandle )
	{
		printf("ffmpegdemux_open error \n");
		return -1;		
	}

	sdlhandle = SDL_Initialize(hwnd);
	if( !sdlhandle )
	{
		printf("SDL_Initialize error \n");
		return -1;		
	}

	g_VideoState.lfVideoClock=0.0;
	g_VideoState.lfAudioClock=0.0;
	g_VideoState.queueVideoFrame.Init();

//		SDL_CreateThread(VideoThread, "", &g_VideoState);
    threadeventloop = std::thread(VideoThread, (void*)&g_VideoState);

////////////////////////////////////////////////////////////////////////////////////
	ffmpegdemuxpacket_t packet; 
	ffmpegdemuxframe_t frame;

	while( 1 )
	{
		int iVideoLen,iAudioLen;
		if( (iVideoLen=g_VideoState.queueVideoFrame.Size())>5
			&& (iAudioLen=g_VideoState.queueAudioFrame.Size())>5
			)
		{
			printf("audiolistsize=%d,videolistsize=%d\n",iAudioLen,iVideoLen);
			usleep(100 * 1000 );
			continue;
		}

		if( ffmpegdemux_read(demuxhandle, &packet) < 0 )
			break;

		if( packet.codec_type == 0 )
		{
			PACKETINFO info;
			info.ipts=packet.pts;
			memcpy(info.data, packet.data, packet.size);
			info.iSize=packet.size;
			info.streamid = packet.stream_index;
			info.codecid = packet.codec_id;
			g_VideoState.queueVideoFrame.Stream_Push(info);
		}
		else if( packet.codec_type == 1 )
		{
			int ret = ffmpegdemux_decode(demuxhandle, packet.codec_id, packet.data, packet.size, &frame);
			if( ret == 0 )
			{
				if( !resamplehandle )
				{
					resamplehandle = resample_open(frame.info.sample.channels, 
						frame.info.sample.sample_fmt, frame.info.sample.sample_rate);

					if( !g_VideoState.queueAudioFrame.Init() )
					{
						printf("queueAudioFrame init error\n");
						return -1;
					}

					if( SDL_InitializeAudio(sdlhandle, 
						frame.info.sample.sample_rate,frame.info.sample.channels, &g_VideoState, AudioCallback) < 0 )
					{
						printf("SDL_InitializeAudio error\n");
						return -1;
					}					
				}

				PACKETINFO info;
				info.ipts=packet.pts;

				resample_sound(resamplehandle, frame.data, frame.linesize, 
					frame.info.sample.nb_samples, &soundframe);

				memcpy(info.data, soundframe.data, soundframe.linesize);
				info.iSize=soundframe.linesize;
				info.streamid = packet.stream_index;
				info.codecid = packet.codec_id;
				g_VideoState.queueAudioFrame.Stream_Push(info);
			}
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
