#ifndef __RTPH264_h__
#define __RTPH264_h__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

#include "rtp.h"

void* rtp_mux_init(unsigned long ssrc);

int rtp_set_h264_frame_over_udp(void* handle, const char* frame_buffer, int frame_length);

int rtp_get_h264_packet_over_udp(void* handle, const char **rtp_buffer, int *rtp_packet_length,
            int *rtp_last_packet_length, int *rtp_packet_count);

int rtp_mux_close(void* handle);

#endif