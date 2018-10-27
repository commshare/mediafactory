#include <stdio.h>

#include "../../../transport/udp/udpselectserver.h"
#include "../../../protocol/librtp/rtpdemux_h264.h"
#include "../../../util/filewritter.h"

void *filewritterhandle = NULL;
void *rtpdemux_h264 = NULL;

int on_rtpdemux_h264frame_callback(const char* frame, int framelength, uint32_t timestamp, uint32_t ssrc)
{
	printf("on_rtpdemux_h264frame_callback length=%u, timestamp=%u\n", framelength, timestamp);
/*
    int frametype = frame[4]&0x1f;
    printf("frameinfo:%u %u %u %u %u frametype:%d framelength:%d \n", 
          frame[0], frame[1], frame[2], frame[3],frame[4], 
          frametype, framelength);
*/
	filewritter_write(filewritterhandle, (char*)frame, framelength);

	return 0;
}

int on_udprecv_callback(void* handle, char* data, int length, 
			const char* remoteip, int remoteport, void* userdata)
{

//	printf("on_udprecv_callback length=%d, remoteport=%d\n", length, remoteport);

	int ret = rtpdemux_h264_setpacket(rtpdemux_h264, data, length);
	
	return 0;
}

int udpserv()
{
	rtpdemux_h264 = rtpdemux_h264_alloc(on_rtpdemux_h264frame_callback);
	filewritterhandle = filewritter_alloc("D:/1.264", 1);

    const char* localip = "192.168.0.100";//"127.0.0.1";
    void* udpserverhandle = udp_select_server_new(localip, 11011, on_udprecv_callback, NULL);
	if( !udpserverhandle )
	{
		printf("udp_select_server_new error\n");
		return -1;
	}
	

	while( 1 )
	{
        usleep(1000 * 2000);
	}
	return 0;
}

int main()
{
	udpserv();

	return 0;
}