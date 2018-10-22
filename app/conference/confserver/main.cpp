#include <stdio.h>

#include "../../../transport/udp/udpselectserver.h"

int on_udpconnect_callback(void* handle, int clientid, void* userdata)
{
	return 0;
}

int on_udpclose_callback(void* handle, int clientid, void* userdata)
{
	return 0;
}

int main()
{

	void* udpserverhandle = udp_select_server_new("", 11011, on_udpconnect_callback, on_udpclose_callback, NULL);

	return 0;
}