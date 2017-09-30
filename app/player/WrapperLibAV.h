#ifndef __WRAPPER_LIBAV_H__
#define __WRAPPER_LIBAV_H__

#ifdef __cplusplus
	#include <string>
extern "C"{
#endif
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
	#include "libavutil/avutil.h"
	#include "libavutil/opt.h"
	#include "libavfilter/avfilter.h"
	#include "libavdevice/avdevice.h"
	#include "libavutil/opt.h"
	#include "libavutil/error.h"
	#include "libavutil/mathematics.h"
	#include "libavutil/samplefmt.h"
#ifdef __cplusplus
}
#endif
#define INT64_C uint64_t

typedef struct tagMediaFormat
{
	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVFrame *pFrame;
	SwrContext *pResampleCtx;

	int streamID;
	int outChannelNum;
	int outChannelLayout;
	int tryInitTimes;
	AVSampleFormat outSampleFmt;
}MediaFormat;

int AV_Initialize();

AVFormatContext *AV_OpenVideoSource(const char *pstrFileNmae);

AVCodecContext *AV_GetCodecContext(AVFormatContext *pFormatCtx,AVMediaType MediaType,int &iMediaCodecID);

int AV_DecodeVideo(AVCodecContext *pCodecCtx, AVFrame *pFrameData, AVPacket *pPacket);

int AV_DecodeAudio(AVCodecContext *pCodecCtx,AVFrame *pFrameData,AVPacket *pPacket);

int AV_GetNextStreamFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iMediaStreamTypeID, AVMediaType MediaType, void *pFrameData, int &iDecodedLen);

int AV_GetNextPacket( AVFormatContext *pFormatCtx, AVPacket &avPacket );

int AV_GetClock(AVStream *pStream,int64_t iPacketPts,double *pfClock);

bool AV_NeedTranscode(int format);

//////////////////////////////output////////////////////////////////////////////////////////

int AV_InitOutputFmt(AVStream *pStream, const char *protocol, const char *ip, unsigned int port, AVFormatContext **fmtctx);

int AV_InitOutputFmt2(AVStream *pStream, const char *protocol, const char *ip, unsigned int port, unsigned int localport, AVFormatContext **fmtctx);

int AV_InitOutputRtp(AVFormatContext *pFmtCtx, const char *protocol, const char *ip, unsigned int port);

int AV_SendOutputPacket(AVFormatContext *fmtctx, AVStream *pStreamIn, AVStream *pStreamOut, AVPacket *pkt);

#endif