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
};

void* alloc_h264framer(H264Configuration_t *config)
{
    void* h264handle = H264Demux_Init((char*)"./test.264", 1);
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

    return h264handle;
}
///////////////////////////////////////////////////////////////////
void* framer_alloc(const char* sourcename)
{
    std::string temp = sourcename;
    int pos = temp.rfind(".");
    if( pos < 0 )
        return NULL;

    H264Configuration_t h264config;
    void* framer_handle = alloc_h264framer(&h264config);
    if( !framer_handle )
    {
        printf("alloc_h264framer error \n");
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

    return 0;
}

int framer_free(void *handle)
{
    return 0;
}