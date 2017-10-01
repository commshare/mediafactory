#ifndef __SDLPLAY_H__
#define __SDLPLAY_H__

typedef void (*sdlplay_audio_callback)(void *userdata, uint8_t *stream, int len);

void* sdlplay_open(void* parenthwnd);

int sdlplay_set_video(void* handle, const char* windowtitle, int iWidth,int iHeight );

int sdlplay_set_audio(void* handle, int iSampleRate,int iChannels,void *pUserData, sdlplay_audio_callback callback);

int sdlplay_display_yuv(void* handle, uint8_t* data[], int linesize[]);
int sdlplay_display(void* handle, uint8_t* data[], int linesize[]);

int sdlplay_close(void* handle);

#endif