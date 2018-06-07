#ifndef __FFMPEG_MUX_H__
#define __FFMPEG_MUX_H__
#include <stdint.h>

void *ffmpegmux_alloc(const char* filenamewithsuffix);

int ffmpegmux_addvideostream(void* handle, int width, int height);

int ffmpegmux_addaudiostream(void* handle, int sample_rate, int channels);

int ffmpegmux_open(void* handle);

int ffmpegmux_write_frame(void *handle, int stream_index, const char* frame, int length, uint64_t pts);

int ffmpegmux_destroy(void* handle);

#endif