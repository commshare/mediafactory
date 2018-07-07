#include <stdio.h>

#include <string>

#include "muxer.h"
#include "../../file/libh26x/h264demux.h"

struct muxer_tag_t
{
    void* muxer_handle;
    
    std::string sourcetype;
    std::string sourcename;

    std::string framedata;
};

///////////////////////////////////////////////////////////////////
void* muxer_alloc(const char* sourcename)
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
    inst->framer_handle = framer_handle;

    return inst;
}

int muxer_getframe(void* handle, const char** frame, int *length)
{
    Framer_tag_t *inst = (Framer_tag_t*)handle;
    if( inst->sourcetype == "264" )
    {
        return h264framer_getframe(handle, frame, length);
    }

    return 0;
}

int muxer_free(void *handle)
{
    return 0;
}
