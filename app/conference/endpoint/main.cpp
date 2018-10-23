#include <stdio.h>

#include "demuxer.h"
#include "muxer.h"

#include "../../../transport/udp/udpclientex.h"

int sendfile(const char* filepath)
{
    const char* remoteip = "192.168.0.100";//"127.0.0.1";
    int remoteport = 11011;
        printf("======\n");
    void* udphandle = udp_clientex_new(-1, 0);
//    void* udphandle = udp_client_new("127.0.0.1", 11011, -1, 0);
    if( !udphandle )
    {
      printf("udp_client_new error \n");
      return NULL;
    }

        printf("======\n");
    ////////////////////////////////////////////////
    void *muxerhandle = muxer_alloc("264");
    void *framerhandle = demuxer_alloc(filepath);

    while( 1 )
    {
        const char *h264frame = NULL;
        int framelength = -1;
        int ret = demuxer_getframe(framerhandle, &h264frame, &framelength);
        if( ret < 0 )
        {
            printf("framer_getframe error\n");
            break;
        }
        printf("======1\n");

        muxer_setframe(muxerhandle, h264frame, framelength);

        const char* rtp_buffer = NULL;
        int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;
        muxer_getpacket(muxerhandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

        printf("rtp_packet_length=%d, last_rtp_packet_length=%d,rtp_packet_count=%d,framelength=%d\n", 
            rtp_packet_length,last_rtp_packet_length,rtp_packet_count,framelength);
        for( int i = 0;i < rtp_packet_count;i++ )
        {
            if( i == rtp_packet_count - 1)
            {
                ret = udp_clientex_write(udphandle, rtp_buffer, last_rtp_packet_length, remoteip, remoteport);
//                ret = udp_client_write(udphandle, rtp_buffer, last_rtp_packet_length);
            }
            else
            {
                ret = udp_clientex_write(udphandle, rtp_buffer, rtp_packet_length, remoteip, remoteport);
//                ret = udp_client_write(udphandle, rtp_buffer, rtp_packet_length);
            }

            if( ret < 0 )
            {
              printf("udp_client_write error\n");
              break;
            }
            rtp_buffer += rtp_packet_length;
        }

        printf("aaaaaaaaaaaaa\n");
        if( ret < 0 )
            break;
        usleep(1000 * 40);
//        usleep(1000 * 1000);
    }
  
    udp_clientex_free(udphandle);
    muxer_free(muxerhandle);
    demuxer_free(framerhandle);
    printf("udpmediaproc exit \n");

	return 0;
}

int main()
{
        printf("======\n");
	sendfile("/test.264");
	
	return 0;
}