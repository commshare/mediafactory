#include <thread>

#include <stdio.h>
#include <unistd.h>

#include "SDL2/SDL.h"
#include "sdlplay.h"

struct SDL2Desc_t 
{
    SDL_Window *screen;   
    SDL_Renderer* sdlRenderer;  
    SDL_Texture* sdlTexture;  
    SDL_Rect sdlRect; 
};

int sdl_pollevent(SDL2Desc_t *inst)
{
	SDL_Event event;
	SDL_PollEvent(&event);

	if (event.type == SDL_KEYDOWN) 
	{
		switch (event.key.keysym.sym) 
		{
		case SDLK_f:
			/* press key 'f' to toggle fullscreen */			
/*
			if (flags&SDL_WINDOW_FULLSCREEN_DESKTOP)
				flags &=
					~SDL_WINDOW_FULLSCREEN_DESKTOP;
			else
				flags |=
					SDL_WINDOW_FULLSCREEN_DESKTOP;

			SDL_SetWindowFullscreen(inst->screen, flags);
*/
			break;

		default:
			break;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////
void* sdlplay_open(void* parenthwnd)
{
	//Èç¹ûÊ¹ÓÃ±¾µØ´°¿Ú£¬ÐèÒªÉèÖÃ´°¿Ú¾ä±ú
/*	if( parenthwnd )
	{
		char variable[256]; 
		sprintf(variable,"SDL_WINDOWID=0x%1x",parenthwnd); // ¸ñÊ½»¯×Ö·û´® 
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

int sdlplay_set_video(void* handle, const char* windowtitle, int iWidth,int iHeight)
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

int sdlplay_set_audio(void* handle, int iSampleRate,int iChannels,void *pUserData, sdlplay_audio_callback callback)
{
	printf("SDL_InitializeAudio %d %d \n",iSampleRate, iChannels);

	SDL_AudioSpec wanted_spec, spec;  
	wanted_spec.freq = iSampleRate;  
	wanted_spec.channels = iChannels;  
	wanted_spec.userdata = pUserData;  
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;  
	wanted_spec.callback = callback;  
	wanted_spec.samples = 1024;
	if(SDL_OpenAudio(&wanted_spec, &spec) < 0)  
	{  
		printf("SDL_OpenAudio: %s/n", SDL_GetError());  
		return -1;  
	}  
	SDL_PauseAudio(0);

	return 0;
}

int sdlplay_display_yuv(void* handle, uint8_t* data[], int linesize[])
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	sdl_pollevent(inst);

    SDL_UpdateYUVTexture(inst->sdlTexture, &inst->sdlRect, data[0], linesize[0], 
	    data[1], linesize[1], data[2], linesize[2]);  
      
    SDL_RenderClear( inst->sdlRenderer );    
    SDL_RenderCopy( inst->sdlRenderer, inst->sdlTexture,  NULL, &inst->sdlRect);    
    SDL_RenderPresent( inst->sdlRenderer );    

    return 0;
}

int sdlplay_display(void* handle, uint8_t* data[], int linesize[])
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	sdl_pollevent(inst);

    SDL_UpdateTexture(inst->sdlTexture, &inst->sdlRect, data, linesize[0]);
      
    SDL_RenderClear( inst->sdlRenderer );    
    SDL_RenderCopy( inst->sdlRenderer, inst->sdlTexture,  NULL, &inst->sdlRect);    
    SDL_RenderPresent( inst->sdlRenderer );    

    return 0;
}

int sdlplay_close(void* handle)
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;

    SDL_Quit();  
	delete inst;  

	return 0;	
}
