#include <stdio.h>

#include "../../../transport/udp/udpselectserver.h"

int on_udprecv_callback(void* handle, char* data, int length, 
			const char* remoteip, int remoteport, void* userdata)
{

	printf("on_udprecv_callback length=%d, remoteport=%d\n", length, remoteport);
	return 0;
}

int udpserv()
{
	void* udpserverhandle = udp_select_server_new("127.0.0.1", 11011, on_udprecv_callback, NULL);
	if( !udpserverhandle )
	{
		printf("udp_select_server_new error\n");
		return -1;
	}

	char buffer[2048] = {0};
	while( 1 )
	{

        usleep(1000 * 200);
	}
	return 0;
}

int main()
{
	udpserv();

	return 0;
}