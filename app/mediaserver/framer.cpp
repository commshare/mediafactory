#include <stdio.h>

#include <string>
#include "framer.h"

struct Framer_tag_t
{

    std::string sourcetype;
    std::string sourcename;
};
///////////////////////////////////////////////////////////////////
void* framer_alloc(const char* sourcename)
{
    std::string temp = sourcename;
    int pos = temp.rfind(".");
    if( pos < 0 )
        return NULL;

    Framer_tag_t *inst = new Framer_tag_t;
    inst->sourcename = sourcename;
    inst->sourcetype = temp.substr(pos+1);

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