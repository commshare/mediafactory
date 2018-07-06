#include <stdio.h>

#include <string>

#include "framer.h"
#include "../../file/libh26x/h264demux.h"

struct Framer_tag_t
{
    void* framer_handle;
    H264Configuration_t h264config;
    
    std::string sourcetype;
    std::string sourcename;

    std::string framedata;
};

void* h264framer_alloc(const char* sourcename, H264Configuration_t *config)
{
    void* h264handle = H264Demux_Init((char*)sourcename, 1);
    if( !h264handle )
    {
        printf("H264Framer_Init error\n");
        return NULL;
    }
    if( H264Demux_GetConfig(h264handle, config) < 0 )
    {
        printf("H264Demux_GetConfig error\n");
        H264Demux_CLose(h264handle);
        return NULL;
    }
    printf("H264Demux_GetConfig:width %d height %d framerate %d timescale %d %d %d \n",
        config->width, config->height, config->framerate, config->timescale,
        config->spslen, config->ppslen);

    return h264handle;
}

int h264framer_getframe(void* handle, const char** frame, int *length)
{
    Framer_tag_t *inst = (Framer_tag_t*)handle;
    const char *h264frame = NULL;
    int framelength = -1;
    int ret = H264Demux_GetFrame(inst->framer_handle, &h264frame, &framelength);
    if( ret < 0 )
    {
        printf("ReadOneNaluFromBuf error\n");
        return -1;
    }

    int frametype = h264frame[4]&0x1f;
/*
    printf("frameinfo:%u %u %u %u %u frametype:%d framelength:%d \n", 
          h264frame[0], h264frame[1], h264frame[2], h264frame[3],h264frame[4], 
          frametype, framelength);
*/
    if( frametype == 5 )
    {
        H264Configuration_t config;
        H264Demux_GetConfig(inst->framer_handle, &config);
        inst->framedata.clear();
        inst->framedata.append(config.sps+4, config.spslen-4);
        inst->framedata.append(config.pps, config.ppslen);
        inst->framedata.append(h264frame, framelength);
    }
    else
    {
        inst->framedata.clear();
        inst->framedata.append(h264frame+4, framelength-4);
    }
    *frame = inst->framedata.data();
    *length = inst->framedata.size();

    return 0;
}

///////////////////////////////////////////////////////////////////
void* framer_alloc(const char* sourcename)
{
    std::string temp = sourcename;
    int pos = temp.rfind(".");
    if( pos < 0 )
        return NULL;

    H264Configuration_t h264config;
    void* framer_handle = h264framer_alloc(sourcename, &h264config);
    if( !framer_handle )
    {
        printf("h264framer_alloc error \n");
        return NULL;
    }

    Framer_tag_t *inst = new Framer_tag_t;
    inst->sourcename = sourcename;
    inst->sourcetype = temp.substr(pos+1);
    inst->h264config = h264config;
    inst->framer_handle = framer_handle;

    return inst;
}

int framer_getframe(void* handle, const char** frame, int *length)
{
    Framer_tag_t *inst = (Framer_tag_t*)handle;
    if( inst->sourcetype == "264" )
    {
        return h264framer_getframe(handle, frame, length);
    }

    return 0;
}

int framer_free(void *handle)
{
    return 0;
}