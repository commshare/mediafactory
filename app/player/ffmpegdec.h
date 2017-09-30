#ifndef __FFMPEGDEC_H__
#define __FFMPEGDEC_H__

#include "typesdefined.h"

typedef int (*video_raw_callback)(uint8_t *data, int length, int64_t pts, int codec);
typedef int (*audio_raw_callback)(uint8_t *data, int length, int64_t pts, int codec );

void* ffmpegdec_open(int video_codecid, int audiocodecid, video_raw_callback vrcallback, audio_raw_callback audio_raw_callback);

int ffmpegdec_decode(void *inst, int codecid, uint8_t *pBuffer, int dwBufsize);

int ffmpegdec_close(void* inst);

#endif