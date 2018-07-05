#ifndef __FRAMER_H__
#define __FRAMER_H__

void* framer_alloc(const char* sourcename);

int framer_getframe(void* handle, const char** frame, int *length);

int framer_free(void *handle);

#endif