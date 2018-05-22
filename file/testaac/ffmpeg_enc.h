#ifndef __FFMPEG_ENC_H__
#define __FFMPEG_ENC_H__
#include <stdint.h>

void *ffmpeg_enc_alloc(int codecid);

int ffmpeg_enc_set_video(void *handle, int width, int height, int pixel_format);

int ffmpeg_enc_set_audio(void *handle, int width, int height, int pixel_format);

int ffmpeg_enc_encode_audio(void *handle, const char* framedata, int length, const char **packetdata, int *packetlength);

int ffmpeg_enc_encode_video(void *handle, const char* framedata, int length, const char **packetdata, int *packetlength);

int video_generator_destroy(void *handle)

#endif