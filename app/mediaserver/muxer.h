#ifndef __MUXER_H__
#define __MUXER_H__

void* muxer_alloc(const char* sourcename);

int muxer_getframe(void* handle, const char** frame, int *length);

int muxer_free(void *handle);

#endif