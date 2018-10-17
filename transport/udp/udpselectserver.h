#ifndef __UDP_SELECT_SERVER__
#define __UDP_SELECT_SERVER__

#include <unistd.h>
#include <stdio.h>

typedef int (*on_connect_callback)(void* handle, int clientid, void* userdata);
typedef int (*on_close_callback)(void* handle, int clientid, void* userdata);

void* udp_select_server_new(const char* localip, int localport, on_connect_callback connectcallback, on_close_callback closecallback, void* userdata);

int udp_select_server_read(void* handle, int clientid, char* data, int length);

int udp_select_server_write(void* handle, int clientid, const char* data, int length);

int udp_select_server_close(void* handle, int clientid);

int udp_select_server_free(void* handle);

#endif