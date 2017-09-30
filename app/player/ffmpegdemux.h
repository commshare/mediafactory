#ifndef __FFMPEGDEMUX_H__
#define __FFMPEGDEMUX_H__

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

typedef int (*video_frame_callback)(uint8_t *data, int length, int64_t pts, int codec);
typedef int (*audio_frame_callback)(uint8_t *data, int length, int64_t pts, int codec );

void* ffmpegdemux_open(const char* url, video_frame_callback, audio_frame_callback);

int ffmpegdemux_eventloop(void* handle);

int ffmpegdemux_close(void* handle);

#endif