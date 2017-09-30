#ifndef __WRAPPER_SDL_H__
#define __WRAPPER_SDL_H__

#include "SDL2/SDL.h"

void* SDL_Initialize(void* parenthwnd);

int SDL_InitializeVideo(void* handle, const char* windowtitle, int iWidth,int iHeight );

int SDL_InitializeAudio(void* handle, int iSampleRate,int iChannels,void *pUserData,void (SDLCALL *FuncCallback)(void *userdata, Uint8 *stream, int len));

int SDL_DisplayYUV(void* handle, uint8_t* data[], int linesize[]);
int SDL_Display(void* handle, uint8_t* data[], int linesize[]);

int SDL_close(void* handle);

#endif