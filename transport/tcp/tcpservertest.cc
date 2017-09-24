#include <stdio.h>

#include "tcpserver.h"

int myon_connect_callback(void* handle, int sockfd, void* userdata)
{
	printf("myon_connect_callback %d \n", sockfd);
	return 0;
}

int myon_close_callback(void* handle, int sockfd, void* userdata)
{
	printf("myon_close_callback %d \n", sockfd);

	return 0;
}

int main()
{
	void* handle = tcp_server_new("127.0.0.1", 11110, myon_connect_callback, myon_close_callback, NULL);

	tcp_server_eventloop(handle);

//int tcp_server_read(void* handle, int sockfd, char* data, int length, int *reallength);

//int tcp_server_write(void* handle, int sockfd, const char* data, int length);
	
	tcp_server_free(handle);

	return 0;
}