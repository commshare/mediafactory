#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <errno.h>

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

#include <string>
#include "rtph264.h"  

typedef struct
{
    std::string rtp_buffer;
    int packet_length;
    int last_packet_length;
    int packet_count;
    unsigned long sequence_number;
    unsigned int timestamp_increse;
    unsigned int timestamp_current;
}rtp_h264_mux_desc_t;

#pragma pack(push,1)
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;  
    unsigned char F:1;
} NALU_HEADER; /**//* 1 BYTES */  
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;   
    unsigned char F:1;      
} FU_INDICATOR; /**//* 1 BYTES */  
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char R:1;  
    unsigned char E:1;  
    unsigned char S:1;      
} FU_HEADER; /**//* 1 BYTES */        
#pragma pack(pop)

void* rtp_mux_init()
{
    rtp_h264_mux_desc_t* handle = new rtp_h264_mux_desc_t;
    handle->packet_length = MAX_RTP_BODY_LENGTH + 14;//1460;
    handle->last_packet_length = 0;
    handle->sequence_number = 0;
    handle->timestamp_increse = 90000/25;
    handle->timestamp_current = 0;

    return (void*)handle;
}

int rtp_set_h264_frame_over_udp(void* handle, const char* frame_buffer, int frame_length)
{
    rtp_h264_mux_desc_t* rtp_mux = (rtp_h264_mux_desc_t*)handle;
    rtp_mux->rtp_buffer.clear();

    NALU_HEADER     nalu_hdr = *(NALU_HEADER*)frame_buffer;  
    FU_INDICATOR    fu_ind;  
    FU_HEADER       fu_hdr;  

   //设置RTP HEADER，  
    RTP_FIXED_HEADER rtp_hdr;      
    memset(&rtp_hdr,0,sizeof(rtp_hdr));  
    rtp_hdr.payload     = H264;  //负载类型号，  
    rtp_hdr.version     = 2;  //版本号，此版本固定为2  
    rtp_hdr.marker    = 0;   //标志位，由具体协议规定其值。  
    rtp_hdr.ssrc     = htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一  

    //  当一个NALU小于1400字节的时候，采用一个单RTP包发送  
    if(frame_length <= MAX_RTP_BODY_LENGTH)  
    {     
        //设置rtp M 位；  
        rtp_hdr.marker = 1;  
        rtp_hdr.seq_no  = htons(rtp_mux->sequence_number++); //序列号，每发送一个RTP包增1，htons，将主机字节序转成网络字节序。  

        rtp_hdr.timestamp=htonl(rtp_mux->timestamp_current);  
        rtp_mux->timestamp_current += rtp_mux->timestamp_increse;  

        rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));
        rtp_mux->rtp_buffer.append(frame_buffer, frame_length);

        rtp_mux->packet_count = 1;
        rtp_mux->last_packet_length = frame_length + 12;
    }
    else
    {  
        //得到该nalu需要用多少长度为1400字节的RTP包来发送  
        int k = frame_length / MAX_RTP_BODY_LENGTH;//需要k个1400字节的RTP包，这里为什么不加1呢？因为是从0开始计数的。  
        int last = frame_length % MAX_RTP_BODY_LENGTH;//最后一个RTP包的需要装载的字节数  
        int t = 0;//用于指示当前发送的是第几个分片RTP包  
        rtp_mux->packet_count = k+1;

        rtp_hdr.timestamp=htonl(rtp_mux->timestamp_current);  
        rtp_mux->timestamp_current += rtp_mux->timestamp_increse;  
        while(t <= k)  
        {  
            rtp_hdr.seq_no = htons(rtp_mux->sequence_number++); //序列号，每发送一个RTP包增1  
            if( 0 == t )//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位,t = 0时进入此逻辑。  
            {  
                //设置rtp M 位；  
                rtp_hdr.marker = 0;  //最后一个NALU时，该值设置成1，其他都设置成0。  
                rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));
        
                //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                fu_ind.F = nalu_hdr.F;  
                fu_ind.NRI = nalu_hdr.NRI;  
                fu_ind.TYPE = 28;  //FU-A类型。  
                rtp_mux->rtp_buffer.append((char*)&fu_ind, sizeof(fu_ind));

                //设置FU HEADER,并将这个HEADER填入sendbuf[13]  
                fu_hdr.E = 0;  
                fu_hdr.R = 0;  
                fu_hdr.S = 1;  
                fu_hdr.TYPE = nalu_hdr.TYPE;  
                rtp_mux->rtp_buffer.append((char*)&fu_hdr, sizeof(fu_hdr));
                rtp_mux->rtp_buffer.append(frame_buffer + 1, MAX_RTP_BODY_LENGTH);
                t++;  
            }
            //发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位  
            else if(k == t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当 l> 1386时）。  
            {  

                //设置rtp M 位；当前传输的是最后一个分片时该位置1  
                rtp_hdr.marker = 1;  
                rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));
        
                //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                fu_ind.F = nalu_hdr.F;  
                fu_ind.NRI = nalu_hdr.NRI;  
                fu_ind.TYPE = 28;  //FU-A类型。  
                rtp_mux->rtp_buffer.append((char*)&fu_ind, sizeof(fu_ind));

                //设置FU HEADER,并将这个HEADER填入sendbuf[13]  
                fu_hdr.R = 0;  
                fu_hdr.S = 0;  
                fu_hdr.E = 1;  
                fu_hdr.TYPE = nalu_hdr.TYPE;  
                rtp_mux->rtp_buffer.append((char*)&fu_hdr, sizeof(fu_hdr));
                rtp_mux->rtp_buffer.append(frame_buffer + t * MAX_RTP_BODY_LENGTH + 1, last-1);
                rtp_mux->last_packet_length = last - 1 + 14;
                t++;  
            }  
            //既不是第一个分片，也不是最后一个分片的处理。  
            else if(t < k && 0 != t)  
            {  
                //设置rtp M 位；  
                rtp_hdr.marker = 0;  
                rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));

                //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                fu_ind.F = nalu_hdr.F;  
                fu_ind.NRI = nalu_hdr.NRI;  
                fu_ind.TYPE = 28;  //FU-A类型。  
                rtp_mux->rtp_buffer.append((char*)&fu_ind, sizeof(fu_ind));

                //设置FU HEADER,并将这个HEADER填入sendbuf[13]  
                fu_hdr.R = 0;  
                fu_hdr.S = 0;  
                fu_hdr.E = 0;  
                fu_hdr.TYPE = nalu_hdr.TYPE;  
                rtp_mux->rtp_buffer.append((char*)&fu_hdr, sizeof(fu_hdr));
                rtp_mux->rtp_buffer.append(frame_buffer + t * MAX_RTP_BODY_LENGTH + 1, MAX_RTP_BODY_LENGTH);
                t++;  
            }  
        }  
    }  

    return 0;    
}

int rtp_get_h264_packet_over_udp(void* handle, const char **rtp_buffer, int *rtp_packet_length,
            int *rtp_last_packet_length, int *rtp_packet_count)
{
    rtp_h264_mux_desc_t* rtp_mux = (rtp_h264_mux_desc_t*)handle;
    *rtp_buffer = rtp_mux->rtp_buffer.data();
    *rtp_packet_length = rtp_mux->packet_length;
    *rtp_last_packet_length = rtp_mux->last_packet_length;
    *rtp_packet_count = rtp_mux->packet_count;

    return 0;
}

int rtp_mux_close(void* handle)
{
    rtp_h264_mux_desc_t* rtp_mux = (rtp_h264_mux_desc_t*)handle;
    delete rtp_mux;
    return 0;
}