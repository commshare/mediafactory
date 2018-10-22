#include <stdio.h>

#include "../../../transport/udp/udpselectserver.h"

int clientid = -1;
int on_udpconnect_callback(void* handle, int clientid, void* userdata)
{
	return 0;
}

int on_udpclose_callback(void* handle, int clientid, void* userdata)
{
	return 0;
}

int udpserv()
{
	void* udpserverhandle = udp_select_server_new("127.0.0.1", 11011, on_udpconnect_callback, on_udpclose_callback, NULL);
	if( !udpserverhandle )
	{
		printf("udp_select_server_new error\n");
		return -1;
	}

	char buffer[2048] = {0};
	while( 1 )
	{
		if( clientid > 0 )
		{
			int readlen = udp_select_server_read(udpserverhandle, clientid, buffer, sizeof(buffer));
			printf("readlen = %d \n", readlen);			
		}

        usleep(1000 * 20);
	}
	return 0;
}

int main()
{
	udpserv();

	return 0;
}