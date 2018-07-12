#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

void *psmux_alloc();

int psmux_writeframe(char *pData, int nFrameLen, Data_Info_s* pPacker, int isvideo);

int psmux_free(void *handle);