#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>

#include "es2ts.h"

// CRC32
static uint32_t CRC_encode(const char* data, int len)
{
#define CRC_poly_32 0x04c11db7

    int byte_count=0, bit_count=0;
    uint32_t CRC = 0xffffffff;

    while (byte_count < len)
    {
        if (((uint8_t)(CRC>>31)^(*data>>(7-bit_count++)&1)) != 0)
            CRC = CRC << 1^CRC_poly_32;
        else 
            CRC = CRC << 1;

        if (bit_count > 7)
        {
            bit_count = 0;
            byte_count++;
            data++;
        }
    }

    return CRC;
}

/* remark:接口函数定义 */  
int bits_initwrite(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data)  
{  
    if (!p_data)  
    {  
        return -1;  
    }  
    p_buffer->i_size = i_size;  
    p_buffer->i_data = 0;  
    p_buffer->i_mask = 0x80;  
    p_buffer->p_data = p_data;  
    p_buffer->p_data[0] = 0;  
    return 0;  
}  
  
void bits_align(BITS_BUFFER_S *p_buffer)  
{  
    if (p_buffer->i_mask != 0x80 && p_buffer->i_data < p_buffer->i_size)  
    {  
        p_buffer->i_mask = 0x80;  
        p_buffer->i_data++;  
        p_buffer->p_data[p_buffer->i_data] = 0x00;  
    }  
}  
  
void bits_write(BITS_BUFFER_S *p_buffer, int i_count, unsigned long i_bits)  
{  
    while (i_count > 0)  
    {  
        i_count--;  
  
        if ((i_bits >> i_count ) & 0x01)  
        {  
            p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;  
        }  
        else  
        {  
            p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;  
        }  
        p_buffer->i_mask >>= 1;  
        if (p_buffer->i_mask == 0)  
        {  
            p_buffer->i_data++;  
            p_buffer->i_mask = 0x80;  
        }  
    }  
}  
    
int bits_initread(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data)  
{  
    if (!p_data)  
    {  
        return -1;  
    }  
    p_buffer->i_size = i_size;  
    p_buffer->i_data = 0;  
    p_buffer->i_mask = 0x80;  
    p_buffer->p_data = p_data;  
    return 0;  
}  
  
int bits_read(BITS_BUFFER_S *p_buffer, int i_count, unsigned long *i_bits)  
{  
    if (!i_bits)  
    {  
        return -1;  
    }  
    *i_bits = 0;  
      
    while (i_count > 0)  
    {  
        i_count--;  
  
        if (p_buffer->p_data[p_buffer->i_data] & p_buffer->i_mask)  
        {  
            *i_bits |= 0x01;  
        }  
  
        if (i_count)  
        {  
            *i_bits = *i_bits << 1;  
        }  
          
        p_buffer->i_mask >>= 1;  
        if(p_buffer->i_mask == 0)  
        {  
            p_buffer->i_data++;  
            p_buffer->i_mask = 0x80;  
        }  
    }  
  
    return 0;  
}

/* *@remark: ts头相关封装 
 *  PSI 包括了PAT、PMT、NIT、CAT 
 *  PSI--Program Specific Information, PAT--program association table, PMT--program map table 
 *  NIT--network information table, CAT--Conditional Access Table 
 *  一个网络中可以有多个TS流(用PAT中的ts_id区分) 
 *  一个TS流中可以有多个频道(用PAT中的pnumber、pmt_pid区分) 
 *  一个频道中可以有多个PES流(用PMT中的mpt_stream区分) 
 */  
int ts_header(char *buf, int pid, int payload_unit_start_indicator, int adaptation_field_control, int counter)  
{    
    if (!buf)  
    {  
        return 0;  
    }  
  
    BITS_BUFFER_S bits;  
    bits_initwrite(&bits, 32, (unsigned char *)buf);  
  
    bits_write(&bits, 8, 0x47);         // sync_byte, 固定为0x47,表示后面的是一个TS分组  
  
    // payload unit start indicator根据TS packet究竟是包含PES packet还是包含PSI data而设置不同值  
    // 1. 若包含的是PES packet header, 设为1,  如果是PES packet余下内容, 则设为0  
    // 2. 若包含的是PSI data, 设为1, 则payload的第一个byte将是point_field, 0则表示payload中没有point_field  
    // 3. 若此TS packet为null packet, 此flag设为0  
    bits_write(&bits, 1, 0);            // transport error indicator  
    bits_write(&bits, 1, payload_unit_start_indicator);       // payload unit start indicator  
    bits_write(&bits, 1, 0);            // transport priority, 1表示高优先级  
    bits_write(&bits, 13, pid);    // pid, 0x00 PAT, 0x01 CAT  
  
    bits_write(&bits, 2, 0);            // transport scrambling control, 传输加扰控制      
    if( adaptation_field_control )
        bits_write(&bits, 2, 0x03);     // 第一位表示有无调整字段，第二位表示有无有效负载  
    else 
        bits_write(&bits, 2, 0x01);     // adaptation field control, 00 forbid, 01 have payload, 10 have adaptation, 11 have payload and adaptation  

    bits_write(&bits, 4, counter&0x0F);  

    bits_align(&bits);  
    return bits.i_data;  
}  

int ts_writecrc(char *buf, uint32_t crc)
{
    BITS_BUFFER_S bits;  
      
    if (!buf)  
    {  
        return 0;  
    }  
    bits_initwrite(&bits, 32, (unsigned char *)buf);  
  
    bits_write(&bits, 32, crc);   

    bits_align(&bits);  
    return bits.i_data;  
}

int ts_pat_header(char *buf)  
{  
    BITS_BUFFER_S bits;  
      
    if (!buf)  
    {  
        return 0;  
    }  
    bits_initwrite(&bits, 32, (unsigned char *)buf);  
  
    bits_write(&bits, 8, 0x00);             // table id, 固定为0x00   
      
    bits_write(&bits, 1, 1);                // section syntax indicator, 固定为1  
    bits_write(&bits, 1, 0);                // zero, 0  
    bits_write(&bits, 2, 0x03);             // reserved1, 固定为0x03  
    bits_write(&bits, 12, 0x0D);            // section length, 表示这个字节后面有用的字节数, 包括CRC32  
      
    bits_write(&bits, 16, 0x0001);          // transport stream id, 用来区别其他的TS流  
      
    bits_write(&bits, 2, 0x03);             // reserved2, 固定为0x03  
    bits_write(&bits, 5, 0x00);             // version number, 范围0-31  
    bits_write(&bits, 1, 1);                // current next indicator, 0 下一个表有效, 1当前传送的PAT表可以使用  
      
    bits_write(&bits, 8, 0x00);             // section number, PAT可能分为多段传输，第一段为00  
    bits_write(&bits, 8, 0x00);             // last section number  
      
    bits_write(&bits, 16, 0x0001);          // program number  
    bits_write(&bits, 3, 0x07);             // reserved3和pmt_pid是一组，共有几个频道由program number指示  
    bits_write(&bits, 13, TS_PID_PMT);      // pmt of pid in ts head  
  
/*
    bits_write(&bits, 8, 0x2a);             // CRC_32 先暂时写死  
    bits_write(&bits, 8, 0xb1);  
    bits_write(&bits, 8, 0x04);  
    bits_write(&bits, 8, 0xb2);  
*/

    bits_align(&bits);  
    return bits.i_data;  
}  

int ts_pointer_field(char *buf)
{
    BITS_BUFFER_S bits;  
      
    if (!buf)  
    {  
        return 0;  
    }  
    bits_initwrite(&bits, 32, (unsigned char *)buf);  

    bits_write(&bits, 8, 0x0);  
  
    bits_align(&bits);  
    return bits.i_data;  
}

/*  
 *@remark : 添加pat头  
 */  
int mk_ts_pat_packet(char *buf, int counter)  
{  
    int nOffset = 0;  
    int nRet = 0;  
      
    if (!buf)  
    {  
        return 0;  
    }  
  
    if (0 >= (nRet = ts_header(buf, 0x00, 1, 0, counter)))  
    {  
        return 0;  
    } 
    nOffset += nRet;  

    if (0 >= (nRet = ts_pointer_field(buf + nOffset)))  
    {  
        return 0;  
    }  
    nOffset += nRet;  

    if (0 >= (nRet = ts_pat_header(buf + nOffset)))  
    {  
        return 0;  
    }

    uint32_t crc = CRC_encode(buf+nOffset, nRet);    
    nOffset += nRet;  
  
    nRet = ts_writecrc(buf + nOffset, crc);
    nOffset += nRet;  

    // 每一个pat都会当成一个ts包来处理，所以每次剩余部分用1来充填完  
    memset(buf + nOffset, 0xFF, TS_PACKET_SIZE - nOffset);  
    return TS_PACKET_SIZE;  
}  

int ts_pmt_header(char *buf)
{  
    BITS_BUFFER_S bits;  
  
    if (!buf)  
    {  
        return 0;  
    }  
  
    bits_initwrite(&bits, 32, (unsigned char *)buf);  
  
    bits_write(&bits, 8, 0x02);             // table id, 固定为0x02  
      
    bits_write(&bits, 1, 1);                // section syntax indicator, 固定为1  
    bits_write(&bits, 1, 0);                // zero, 0  
    bits_write(&bits, 2, 0x03);             // reserved1, 固定为0x03  
    bits_write(&bits, 12, 0x12);            // section length, 表示这个字节后面有用的字节数, 包括CRC32  
      
    bits_write(&bits, 16, 0x0001);          // program number, 表示当前的PMT关联到的频道号码  
      
    bits_write(&bits, 2, 0x03);             // reserved2, 固定为0x03  
    bits_write(&bits, 5, 0x00);             // version number, 范围0-31  
    bits_write(&bits, 1, 1);                // current next indicator, 0 下一个表有效, 1当前传送的PAT表可以使用  
      
    bits_write(&bits, 8, 0x00);             // section number, PAT可能分为多段传输，第一段为00  
    bits_write(&bits, 8, 0x00);             // last section number  
  
    bits_write(&bits, 3, 0x07);             // reserved3, 固定为0x07  
    bits_write(&bits, 13, TS_PCR_PID);    // pcr of pid in ts head, 如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF  
    bits_write(&bits, 4, 0x0F);             // reserved4, 固定为0x0F  
    bits_write(&bits, 12, 0x00);            // program info length, 前两位bit为00  
  
    bits_write(&bits, 8, TS_PMT_STREAMTYPE_H264_VIDEO);     // stream type, 标志是Video还是Audio还是其他数据  
    bits_write(&bits, 3, 0x07);             // reserved, 固定为0x07  
    bits_write(&bits, 13, TS_PID_VIDEO);    // elementary of pid in ts head  
    bits_write(&bits, 4, 0x0F);             // reserved, 固定为0x0F  
    bits_write(&bits, 12, 0x00);            // elementary stream info length, 前两位bit为00  
  
    bits_align(&bits);  
    return bits.i_data;  
}  

/*  
 *@remaark: 添加PMT头 
 */  
int mk_ts_pmt_packet(char *buf, int counter)  
{  
    int nOffset = 0;  
    int nRet = 0;  
      
    if (!buf)  
    {  
        return 0;  
    }  
  
    if (0 >= (nRet = ts_header(buf, TS_PID_PMT, 1, 0, counter)))  
    {  
        return 0;  
    }  
    nOffset += nRet;  
    
    if (0 >= (nRet = ts_pointer_field(buf + nOffset)))  
    {  
        return 0;  
    }  
    nOffset += nRet;  

    if (0 >= (nRet = ts_pmt_header(buf + nOffset)))  
    {  
        return 0;  
    }

    uint32_t crc = CRC_encode(buf+nOffset, nRet);    
    nOffset += nRet;  
  
    nRet = ts_writecrc(buf + nOffset, crc);
    nOffset += nRet;  

    // 每一个pmt都会当成一个ts包来处理，所以每次剩余部分用1来充填完  
    memset(buf + nOffset, 0xFF, TS_PACKET_SIZE - nOffset);  
    return TS_PACKET_SIZE;  
}  

int ts_adaptation_field(char *buf, int adaptionlength, int writepcr, uint64_t timestamp)
{
    BITS_BUFFER_S bits;  
      
    if (!buf)  
    {  
        return 0;  
    }  
    bits_initwrite(&bits, 32, (unsigned char *)buf);  

    bits_write(&bits, 8, adaptionlength);  
    if( writepcr )
    {
        bits_write(&bits, 8, 0x10);  
//        timestamp = 3600;
        printf("timestamp %d %x %x %x %x %x \n", timestamp,
            uint8_t(timestamp >> 25), uint8_t(timestamp >> 17), uint8_t(timestamp >> 9), uint8_t(timestamp >> 1),
            uint8_t(((timestamp & 1) << 7) | 0x7e) );
        //pcr
        bits_write(&bits, 8, timestamp >> 25);  
        bits_write(&bits, 8, timestamp >> 17);  
        bits_write(&bits, 8, timestamp >> 9);  
        bits_write(&bits, 8, timestamp >> 1);  
        bits_write(&bits, 8, ((timestamp & 1) << 7) | 0x7e);  
        bits_write(&bits, 8, 0x00);
    }
    else
    {
        bits_write(&bits, 8, 0x00);  
    }

    bits_align(&bits);  
    return bits.i_data;  
}

/*  
 *@remark: ts头的封装 
 */  
int mk_ts_data_packet(char *buf, int nFramelen, int bStart, int adaptionfield, int writepcr, int counter, uint64_t timestamp)  
{  
    if (!buf)  
    {  
        return 0;  
    }  
  
    int nRet = ts_header(buf, TS_PID_VIDEO, bStart, adaptionfield, counter);
    if( nRet <= 0 )
    {  
        return 0;  
    }
    int nOffset = nRet;  
    
    if( adaptionfield )
    {
        int adaptionlength = 7;
        if( nFramelen < TS_PACKET_SIZE - 5 )
        {
            adaptionlength = TS_PACKET_SIZE - 5 - nFramelen;
        }
//        printf("adaptionlength %d framelen %d \n", adaptionlength, nFramelen);
        nRet = ts_adaptation_field(buf + nOffset, adaptionlength, writepcr,timestamp);
        int tempOffset = nOffset + nRet;          
        memset(buf + tempOffset, 0xFF, TS_PACKET_SIZE - nOffset);  
        nOffset += adaptionlength+1;
    }
    else if( bStart )
    {
        nRet = ts_pointer_field(buf + nOffset);
        nOffset += nRet;          
    }  
    return nOffset;  
}  

int make_pes_header(char *pData, int stream_id, int payload_len, uint64_t pts, uint64_t dts)  
{  
    payload_len += 13;
    if (PES_MAX_SIZE < payload_len)  
    {  
        payload_len = 0;  
    }  
 
    BITS_BUFFER_S   bitsBuffer;  
    bits_initwrite(&bitsBuffer, 32, (unsigned char *)pData);  
    /*system header*/  
    bits_write( &bitsBuffer, 24,0x000001);  /*start code*/  
    bits_write( &bitsBuffer, 8, (stream_id));   /*streamID*/  
    bits_write( &bitsBuffer, 16,payload_len);  /*packet_len*/ //指出pes分组中数据长度和该字节后的长度和  
 
    bits_write( &bitsBuffer, 2, 2 );        /*'10'*/  
    bits_write( &bitsBuffer, 2, 0 );        /*scrambling_control*/  
    bits_write( &bitsBuffer, 1, 0 );        /*priority*/  
    bits_write( &bitsBuffer, 1, 0 );        /*data_alignment_indicator*/  
    bits_write( &bitsBuffer, 1, 0 );        /*copyright*/  
    bits_write( &bitsBuffer, 1, 0 );        /*original_or_copy*/  

    bits_write( &bitsBuffer, 1, 1 );        /*PTS_flag*/  
    bits_write( &bitsBuffer, 1, 1 );        /*DTS_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*ESCR_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*ES_rate_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*DSM_trick_mode_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*additional_copy_info_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*PES_CRC_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*PES_extension_flag*/  

    bits_write( &bitsBuffer, 8, 10);        /*header_data_length*/   
    // 指出包含在 PES 分组标题中的可选字段和任何填充字节所占用的总字节数。该字段之前  
    //的字节指出了有无可选字段。  
    /*PTS,DTS*/   
//    bits_write( &bitsBuffer, 4, 3 );                    /*'0011'*/  
    bits_write( &bitsBuffer, 4, 2 );                    /*'0011'*/  
    bits_write( &bitsBuffer, 3, ((pts)>>30)&0x07 );     /*PTS[32..30]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,((pts)>>15)&0x7FFF);    /*PTS[29..15]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,(pts)&0x7FFF);          /*PTS[14..0]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 4, 1 );                    /*'0001'*/  
    bits_write( &bitsBuffer, 3, ((dts)>>30)&0x07 );     /*DTS[32..30]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,((dts)>>15)&0x7FFF);    /*DTS[29..15]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,(dts)&0x7FFF);          /*DTS[14..0]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_align(&bitsBuffer);  
    return bitsBuffer.i_data;  
}  

///////////////////////////////////////////////////////////////////////////////////////
struct es2ts_t 
{
    unsigned int pmtcounter;
    unsigned int communitycounter;
    std::fstream fsts;
};

void *es2ts_alloc(const char* filename)
{
    es2ts_t *inst = new es2ts_t();
    inst->pmtcounter = 0;
    inst->communitycounter = 0;
    inst->fsts.open(filename, std::ios::binary | std::ios::out);

    return inst;
}

/* 
 *@remark: 整体发送数据的抽象逻辑处理函数接口 
 */  
int es2ts_writeH264(void *handle, const char* framedata, int nFrameLen, int keyframe, uint64_t timestamp)
{  
    es2ts_t *inst = (es2ts_t*)handle;
    if( !inst || !inst->fsts.is_open() )
        return -1;

    int nSendDataOff = 0;  
    char TSFrameHdr[1024] = {0};
    int nRet = 0;

    if( keyframe == 1)  
    {
        nRet = mk_ts_pat_packet(TSFrameHdr +nSendDataOff, inst->pmtcounter);
//        printf("mk_ts_pat_packet %d \n", nRet);
        if( nRet <= 0)      
        {  
            return -1;  
        }
        nSendDataOff += nRet;  

        nRet = mk_ts_pmt_packet(TSFrameHdr + nSendDataOff, inst->pmtcounter);      
//        printf("mk_ts_pmt_packet %d \n", nRet);
        if( nRet <= 0)      
        {
            return -1;  
        }
        inst->pmtcounter++;
        nSendDataOff += nRet;          
        inst->fsts.write(TSFrameHdr, nSendDataOff);
    }
    
    nSendDataOff = 0;
    //get first pes ts header
    nRet = mk_ts_data_packet(TSFrameHdr + nSendDataOff, nFrameLen+19, 1, 1, 1, inst->communitycounter++, timestamp);
    nSendDataOff += nRet;
    int packetoffset = nRet;

    //get pes header
    nRet = make_pes_header(TSFrameHdr + nSendDataOff, 0xE0, nFrameLen, timestamp, timestamp);
    nSendDataOff += nRet;
    packetoffset += nRet;
//    printf("make_pes_header %d %d\n", nRet, packetoffset);
    if( nFrameLen <= (TS_PACKET_SIZE-6) )
    {
        memcpy(TSFrameHdr + nSendDataOff, framedata, nFrameLen);  
        nSendDataOff += nFrameLen;
        inst->fsts.write(TSFrameHdr, nSendDataOff);
        return 0;        
    }
    else
    {
        memcpy(TSFrameHdr + nSendDataOff,  framedata, TS_PACKET_SIZE - packetoffset);  
        nSendDataOff += (TS_PACKET_SIZE - packetoffset);  
        inst->fsts.write(TSFrameHdr, nSendDataOff);
    }

    nFrameLen -= TS_PACKET_SIZE - packetoffset;
    int frameoffset = TS_PACKET_SIZE - packetoffset;
    while(nFrameLen > 0)
    {
        int nWriteSize = 0;
        if(nFrameLen >= TS_PACKET_SIZE - 6)
        {
            //get other ts header
            nRet = mk_ts_data_packet(TSFrameHdr, nFrameLen, 0, 0, 0, inst->communitycounter++, timestamp);
            nSendDataOff = nRet;

            nWriteSize = TS_PACKET_SIZE - nRet;  
            memcpy(TSFrameHdr + nSendDataOff,  framedata+frameoffset, nWriteSize);  
            frameoffset += nWriteSize;
//            printf("nWriteSize %d \n", nWriteSize);

            nSendDataOff += nWriteSize;
            inst->fsts.write(TSFrameHdr, nSendDataOff);
        }
        else
        {
            //get other ts header
            nRet = mk_ts_data_packet(TSFrameHdr, nFrameLen, 0, 1, 0, inst->communitycounter++, timestamp);
            nSendDataOff = nRet;

            nWriteSize = nFrameLen;  
            memcpy(TSFrameHdr + nSendDataOff,  framedata+frameoffset, nWriteSize);  
            frameoffset += nWriteSize;
//            printf("last nWriteSize %d \n", nWriteSize);

            nSendDataOff += nWriteSize;
            inst->fsts.write(TSFrameHdr, nSendDataOff);
        }

        nFrameLen -= nWriteSize;  
    }  

    return 0;  
}

int es2ts_destroy(void *handle)
{
    es2ts_t *inst = (es2ts_t*)handle;

    inst->fsts.close();
    delete inst;

    return 0;
}

/*
/////////////////////////////////////////////////////////////////////////////////
char buffer[2 * 1000 * 1000 ] = {0};
int bufferlen = 0;

int main()
{
    fsts.open("./222.ts", std::ios::binary | std::ios::out);

    char h264startcode[] = {0,0,0,1};
    uint64_t timestamp =0;
    OpenBitstreamFile((char*)"./111.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。  
    NALU_t *n = AllocNALU( 8 * 1000 * 1000 );//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针  

    while(!feof(bits))   
    {
        int ret = GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001  
        if( ret < 0 )
        {
            printf("break the loop\n");
            break;
        }
        printf("nalu length=%d %d %x %x %x %x \n", n->len, n->nal_unit_type, n->buf[0], n->buf[1], n->buf[2], n->buf[3]);

        int keyframe = 0;
        if( n->nal_unit_type == 7 )//|| n->nal_unit_type == 8 )
        {
            keyframe = 1;
        }

        memcpy(buffer, h264startcode, sizeof(h264startcode));
        bufferlen = sizeof(h264startcode);
        memcpy(buffer+bufferlen, n->buf, n->len);
        bufferlen += n->len;

        gb28181_es2tsForH264(buffer, bufferlen, keyframe, timestamp);
        
        timestamp += 3600;
//        if( bufferlen > 100 )
//            break;
    }

   fsts.close();
//  fs264.close();
    fclose(bits);

    return 0;
}
*/