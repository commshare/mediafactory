/*

  Copyright (c) 2015 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <string>

#include "../../protocol/librtsp/rtspdemux.h"
#include "../../protocol/librtp/rtph264.h"
#include "../../file/libh26x/h264demux.h"
#include "rtspserver.h"

#include "../../transport/tcp/tcpserver.h"
#include "../../transport/udp/udpclient.h"

typedef struct 
{
    std::string client_ip;
    int client_rtp_port;  
    unsigned int session_id;
    int client_fd;
    int local_rtp_sock;
    int local_rtcp_sock;
    int local_rtp_port;
    int local_rtcp_port;
    void* handle;
}rtsp_session_desc_t;

unsigned int session_id = 100;

void* mediaproc(void *arg)
{
    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    int clientport = session->client_rtp_port;
    printf(" clientport = %d \n", clientport);

    void* udphandle = udp_client_new(session->client_ip.c_str(), clientport, -1, session->local_rtp_sock);
    if( !udphandle )
    {
      printf("udp_client_new error \n");
      return NULL;
    }

    ////////////////////////////////////////////////
    void* rtphandle = rtp_mux_init(1);
    void* h264handle = H264Demux_Init((char*)"./test.264", 1);
    if( !h264handle )
    {
      printf("H264Framer_Init error\n");
      return NULL;
    }
    H264Configuration_t config;
    if( H264Demux_GetConfig(h264handle, &config) < 0 )
    {
      printf("H264Demux_GetConfig error\n");
      return NULL;
    }
    printf("H264Demux_GetConfig:width %d height %d framerate %d timescale %d %d %d \n",
        config.width, config.height, config.framerate, config.timescale,
        config.spslen, config.ppslen);

    const char *h264frame = NULL;
    int framelength = -1;
    unsigned int timestamp = 0;

    std::string temp_frame;

    const char* rtp_buffer = NULL;
    int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;

    while( 1 )
    {
        int ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
        if( ret < 0 )
        {
          printf("ReadOneNaluFromBuf error\n");
          break;
        }

        int frametype = h264frame[4]&0x1f;
        if( frametype == 5 )
        {
            H264Demux_GetConfig(h264handle, &config);
            temp_frame.clear();
            temp_frame.append(config.sps+4, config.spslen-4);
            temp_frame.append(config.pps, config.ppslen);
            temp_frame.append(h264frame, framelength);
        }
        else
        {
            temp_frame.clear();
            temp_frame.append(h264frame+4, framelength-4);
        }

//        rtp_set_h264_frame_over_udp(rtphandle, h264frame+4, framelength-4);
        rtp_set_h264_frame_over_udp(rtphandle, temp_frame.data(), temp_frame.size());
        rtp_get_h264_packet_over_udp(rtphandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

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

        usleep(1000 * 40);
    }
  
    udp_client_free(udphandle);
    return NULL;
}

void *handle_request(void *arg) {

    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    int cli_nfd = session->client_fd;
    printf("handle_request\n");

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* 获取本端的socket地址 */
//    nRet = getsockname(cli_nfd->osfd,(struct sockaddr*)&addr,&addr_len);
    getpeername(session->client_fd,(struct sockaddr*)&addr,&addr_len);
    session->client_ip = inet_ntoa(addr.sin_addr);
    printf("clientip = %s \n", session->client_ip.c_str());

    if( 0 > get_local_rtp_rtcp_port(&session->local_rtp_sock, &session->local_rtp_port, 
                &session->local_rtcp_sock, &session->local_rtcp_port))
    {
        printf("get_local_rtp_rtcp_port error\n");
        return NULL;
    }

    char buf[1024] = {0};
    sprintf(buf, "%d", session->session_id);
    void* rtspdemuxhandle = rtsp_demux_init(buf, session->local_rtp_port);

    std::string buffer, msg;
    for(;;) {
      sleep(1);

      memset(buf,0,sizeof(buf));
      int nr = tcp_server_read(session->handle, session->client_fd, buf, sizeof(buf));
      if (nr < 0)
        break;

      buffer.append(buf,nr);
//      printf("buffer\n%s\n", buffer.c_str());

      if( 0 != url_getmsg(buffer, msg) )
        continue;

      printf("msg\n%s\n", msg.c_str());
      int canSend = 0;
      std::string response = rtsp_demux_parse(rtspdemuxhandle, msg.c_str(), canSend);
//      std::string response = rtsp_parse(buf,canSend);
      if( response != "")
      {
        printf("response\n%s\n",response.c_str());
        int nw = tcp_server_write(session->handle, session->client_fd, (char*)response.c_str(), response.size());
        if (nw < 0)
          break;
      }

      if( canSend )
      {
        printf("canSend\n");

        session->client_rtp_port = rtsp_demux_get_client_rtp_port(rtspdemuxhandle);
        std::thread t(mediaproc, (void*)session);
        t.detach();
      }
    }

  close(cli_nfd);
}

//////////////////////////////////////////////////////////
int myon_connect_callback(void* handle, int sockfd, void* userdata)
{
  printf("myon_connect_callback %d \n", sockfd);

  rtsp_session_desc_t* session = new rtsp_session_desc_t;
  session->session_id = session_id++;
  session->client_fd = sockfd;
  session->handle = handle;

  std::thread t(handle_request, (void*)session);
  t.detach();

  return 0;
}

int myon_close_callback(void* handle, int sockfd, void* userdata)
{
  printf("myon_close_callback %d \n", sockfd);

  return 0;
}

void *rtspserv(int port)
{
  void* handle = tcp_server_new("0.0.0.0", port, myon_connect_callback, myon_close_callback, NULL);

//int tcp_server_write(void* handle, int sockfd, const char* data, int length);

//  tcp_server_free(handle);

  return 0;
}