#include <stdio.h>

#include "demuxer.h"
#include "muxer.h"

#include "../../../transport/udp/udpclient.h"

int sendfile(const char* filepath)
{
    int clientport = 0;
    printf(" clientport = %d \n", clientport);

    void* udphandle = udp_client_new("", clientport, -1, 0);
    if( !udphandle )
    {
      printf("udp_client_new error \n");
      return NULL;
    }

    ////////////////////////////////////////////////
    void *muxerhandle = muxer_alloc("AAC");
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

        muxer_setframe(muxerhandle, h264frame, framelength);

        const char* rtp_buffer = NULL;
        int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;
        muxer_getpacket(muxerhandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

        for( int i = 0;i < rtp_packet_count;i++ )
        {
            if( i == rtp_packet_count - 1)
            {
                ret = udp_client_write(udphandle, rtp_buffer, last_rtp_packet_length);
            }
            else
            {
                ret = udp_client_write(udphandle, rtp_buffer, rtp_packet_length);
            }

            if( ret < 0 )
            {
              printf("udp_client_write error\n");
              break;
            }
            rtp_buffer += rtp_packet_length;
        }
        
        if( ret < 0 )
        {
            break;
        }
        usleep(1000 * 40);
    }
  
    udp_client_free(udphandle);
    muxer_free(muxerhandle);
    demuxer_free(framerhandle);
    printf("udpmediaproc exit \n");

	return 0;
}

int main()
{
	sendfile("test.264");
	
	return 0;
}