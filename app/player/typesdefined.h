#ifndef __TYPESDEFINED_H__
#define __TYPESDEFINED_H__

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <deque>

#include "typesdefined.h"
#include "dmic.h"
#include "dmicparser.h"
#include "dmiapi.h"

typedef enum 
{
	UNKNOWN,
	RGB24
}PIXFORAMT;

typedef struct 
{
	int width;
	int height;
	PIXFORAMT fmt;
	char *data;
	int length;
}RawDataInfo_t;

typedef int (*RawDataCallback)(RawDataInfo_t *pInfo, void *pUserData);

typedef struct
{
	uint8_t channelid;
	uint32_t frameid;
	uint16_t slicecount;
	uint8_t payloadtype;
	int64_t frametime;

	char data[1024 * 1000];
	int length;
	uint16_t slices;
	time_t intime;
}FrameData_t;
typedef std::deque<FrameData_t> QUEUEFRAMEDATA;

typedef struct
{
	uint8_t channelid;
	uint32_t frameid;
	uint16_t slicecount;
	uint8_t payloadtype;
	int64_t frametime;
}FrameInfo_t;

typedef std::deque<FrameInfo_t> QUEUEFRAMEINFO;

#define DATABUFFERSIZE 2 * 1024 * 1024
typedef struct 
{
		int sockfd;
    DMICInfo_t info;
    std::string streamID;
    std::string sessionID;

    std::string streamType;
    VECCHANNELINFO channels;

    pthread_t id_netproc;  
    pthread_t id_decodeproc;  
    framecallback fcallback;
    responsecallback rcallback;

    pthread_mutex_t mutexframes;
	
		FrameData_t framedata0;
		FrameData_t framedata1;
		
		void* pdecoder;
		RawDataInfo_t rawdata;
		
		QUEUEFRAMEDATA framequeue;
		QUEUEFRAMEINFO frameinfos;

		char buffer[DATABUFFERSIZE];
		int offset;
}DMIInstance_t;

#endif