#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

void *psmux_alloc(const char* filename);

int psmux_writeframe(void* handle, char *pData, int nFrameLen, uint64_t timestamp, int isvideo);

int psmux_free(void *handle);