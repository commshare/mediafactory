#ifndef __TSMUX2_H__
#define __TSMUX2_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>

//////////////////////////////////////////////////////////////////
void *es2ts_alloc(const char* filename);

/* 
 *@remark: 整体发送数据的抽象逻辑处理函数接口 
 */  
int es2ts_writeH264(void *handle, const char* framedata, int nFrameLen, uint64_t timestamp);

int es2ts_destroy(void *handle);

#endif