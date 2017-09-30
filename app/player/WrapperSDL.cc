#ifdef __cplusplus
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

#include "WrapperSDL.h"
#include "WrapperLibAV.h"

struct SDL2Desc_t 
{
    SDL_Window *screen;   
    SDL_Renderer* sdlRenderer;  
    SDL_Texture* sdlTexture;  
    SDL_Rect sdlRect;  
};

void* SDL_Initialize(void* parenthwnd)
{
	//如果使用本地窗口，需要设置窗口句柄
/*	if( parenthwnd )
	{
		char variable[256]; 
		sprintf(variable,"SDL_WINDOWID=0x%1x",parenthwnd); // 格式化字符串 
		int ret = SDL_putenv(variable);  
		if( ret != 0 )
			return NULL;
	}
*/
	int iReturn=SDL_Init(SDL_INIT_EVERYTHING);
	if( iReturn != 0 )
		return NULL;

	SDL2Desc_t *inst = new SDL2Desc_t;
	return inst;
}

int SDL_InitializeVideo(void* handle, const char* windowtitle, int iWidth,int iHeight )
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	printf("SDL_InitializeVideo %d %d \n", iWidth, iHeight);

    //SDL 2.0 Support for multiple windows  
    inst->screen = SDL_CreateWindow(windowtitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  
        iWidth, iHeight, SDL_WINDOW_OPENGL);    
    if(!inst->screen) {    
        printf("SDL: could not create window - exiting:%s\n",SDL_GetError());    
        return -1;  
    }  

    inst->sdlRenderer = SDL_CreateRenderer(inst->screen, -1, 0);    
    //IYUV: Y + U + V  (3 planes)  
    //YV12: Y + V + U  (3 planes)  
    inst->sdlTexture = SDL_CreateTexture(inst->sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, iWidth, iHeight);    
  
    inst->sdlRect.x=0;  
    inst->sdlRect.y=0;  
    inst->sdlRect.w=iWidth;  
    inst->sdlRect.h=iHeight;  

	return 0;
}

int SDL_InitializeAudio(void* handle, int iSampleRate,int iChannels,void *pUserData,void (SDLCALL *FuncCallback)(void *userdata, Uint8 *stream, int len))
{
	printf("SDL_InitializeAudio %d %d \n",iSampleRate, iChannels);

	SDL_AudioSpec wanted_spec, spec;  
	wanted_spec.freq = iSampleRate;  
	wanted_spec.channels = iChannels;  
	wanted_spec.userdata = pUserData;  
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;  
	wanted_spec.callback = FuncCallback;  
	wanted_spec.samples = 1024;
	if(SDL_OpenAudio(&wanted_spec, &spec) < 0)  
	{  
		fprintf(stderr, "SDL_OpenAudio: %s/n", SDL_GetError());  
		return -1;  
	}  
	SDL_PauseAudio(0);

	return 0;
}

int SDL_DisplayYUV(void* handle, uint8_t* data[], int linesize[])
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;

    SDL_UpdateYUVTexture(inst->sdlTexture, &inst->sdlRect, data[0], linesize[0], 
	    data[1], linesize[1], data[2], linesize[2]);  
      
    SDL_RenderClear( inst->sdlRenderer );    
    SDL_RenderCopy( inst->sdlRenderer, inst->sdlTexture,  NULL, &inst->sdlRect);    
    SDL_RenderPresent( inst->sdlRenderer );    

    return 0;
}

int SDL_Display(void* handle, uint8_t* data[], int linesize[])
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;

    SDL_UpdateTexture(inst->sdlTexture, &inst->sdlRect, data, linesize[0]);
      
    SDL_RenderClear( inst->sdlRenderer );    
    SDL_RenderCopy( inst->sdlRenderer, inst->sdlTexture,  NULL, &inst->sdlRect);    
    SDL_RenderPresent( inst->sdlRenderer );    

    return 0;
}

int SDL_close(void* handle)
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;

    SDL_Quit();  
	delete inst;  

	return 0;	
}
