#ifndef __FFMPEGMUX_H__
#define __FFMPEGMUX_H__

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

int AV_InitOutputFmt(AVStream *pStream, const char *protocol, const char *ip, unsigned int port, AVFormatContext **fmtctx);

int AV_InitOutputFmt2(AVStream *pStream, const char *protocol, const char *ip, unsigned int port, unsigned int localport, AVFormatContext **fmtctx);

int AV_InitOutputRtp(AVFormatContext *pFmtCtx, const char *protocol, const char *ip, unsigned int port);

int AV_SendOutputPacket(AVFormatContext *fmtctx, AVStream *pStreamIn, AVStream *pStreamOut, AVPacket *pkt);

#endif