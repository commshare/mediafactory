#ifndef __PCMDEMUX__H__
#define __PCMDEMUX__H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

typedef struct {
    uint32_t samplerate;
    uint32_t samplerateindex;
    uint32_t profilelevel;
    uint32_t channelcount;
    uint32_t bitcount;    
}PCMConfiguration_t;

void* PCMDemux_Init(const char* filepath, uint32_t bitcount, uint32_t channelcount,
	uint32_t samplerate, int circleread);

int PCMDemux_GetConfig(void *handle, PCMConfiguration_t *config);

int PCMDemux_GetFrame(void* handle, const char **frame, int *length);

int PCMDemux_CLose(void *handle);

#endif