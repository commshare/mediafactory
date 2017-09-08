#ifndef __COLORSCALE_H__
#define __COLORSCALE_H__
#include <windef.h>

bool  RGB2YUV420(LPBYTE RgbBuf,UINT nWidth,UINT nHeight,LPBYTE yuvBuf,unsigned long *len);

bool YUV4202RGB(LPBYTE yuvBuf,UINT nWidth,UINT nHeight,LPBYTE pRgbBuf,unsigned long *len);

#endif