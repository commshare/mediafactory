#ifndef __ES2TS_H__
#define __ES2TS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>

typedef struct  
{  
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)  
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)  
    unsigned max_size;            //! Nal Unit Buffer size  
    int forbidden_bit;            //! should be always FALSE  
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx  
    int nal_unit_type;            //! NALU_TYPE_xxxx      
    char *buf;                    //! contains the first byte followed by the EBSP  
    unsigned short lost_packets;  //! true, if packet loss is detected  
} NALU_t;  

struct ts_header_t{
    char    syn_byte:8;                                       // 包头同步字节，0x47
    char    transport_error_indicator:1;         //传送数据包差错指示器
    char    payload_unit_start_indicator:1;    //有效净荷单元开始指示器
    char    transport_priority:1;                         //传送优先级
    int     PID:13;                                              //包ID
    char    transport_scrambling_control:2;                //传送加扰控制
    char    adaptation_field_control:2;           //调整字段控制
    char    continuity_conunter:4;                    //连续计数器 0-15
};

// TS adaptation flags
typedef struct
{
    char adaptation_field_extension_flag: 1,
         transport_private_data_flag: 1,
         splicing_point_flag: 1,
         OPCR_flag: 1,
         PCR_flag: 1,
         elementary_stream_priority_indicator: 1,
         random_access_indicator: 1,
         discontinuity_indicator: 1;
} __attribute__((packed))
ts_adaptation_flags;

/*关于TS流需要了解下节目映射表(PAT：Program Associate Table)以及节目映射表(PMT：Program Map Table)，
当发送到数据为视频数据关键帧的时候，需要在包头中添加PAT和PMT 
*/

/*  
 *remark: 上面用到的一些宏定义和一些关于字节操作的函数，很多一些开源到视频处理的库都能看到， 
          为了方便也都将贴出来分享，当然也可以参考下vlc里面的源码 
 */  
  
/*@remark: 常量定义 */  
#define TS_PID_PMT      (0x1000)  
#define TS_PID_VIDEO    (0x101)  
#define TS_PID_AUDIO    (0x84)  
#define TS_PCR_PID (0x100)

#define TS_PMT_STREAMTYPE_11172_AUDIO   (0x03)  
#define TS_PMT_STREAMTYPE_13818_AUDIO   (0x04)  
#define TS_PMT_STREAMTYPE_AAC_AUDIO     (0x0F)  
#define TS_PMT_STREAMTYPE_H264_VIDEO    (0x1B)  

#define PES_HDR_LEN 19
enum TSTYPE{
    TS_TYPE_BEGIN = -1,
    TS_TYPE_PAT,
    TS_TYPE_PMT,
    TS_TYPE_VIDEO,
    TS_TYPE_AUDIO,
    TS_TYPE_END
};

#define PES_MAX_SIZE 65535
#define TS_PACKET_SIZE 188
#define TS_LOAD_LEN (188-12)

// CRC32
static uint32_t CRC_encode(const char* data, int len);

/* @remark: 结构体定义 */  
typedef struct  
{  
    int i_size;             // p_data字节数  
    int i_data;             // 当前操作字节的位置  
    unsigned char i_mask;   // 当前操作位的掩码  
    unsigned char *p_data;  // bits buffer  
} BITS_BUFFER_S;  
  
/* remark:接口函数定义 */  
int bits_initwrite(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data);
  
void bits_align(BITS_BUFFER_S *p_buffer);
  
void bits_write(BITS_BUFFER_S *p_buffer, int i_count, unsigned long i_bits);
    
int bits_initread(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data);
  
int bits_read(BITS_BUFFER_S *p_buffer, int i_count, unsigned long *i_bits);

/* *@remark: ts头相关封装 
 *  PSI 包括了PAT、PMT、NIT、CAT 
 *  PSI--Program Specific Information, PAT--program association table, PMT--program map table 
 *  NIT--network information table, CAT--Conditional Access Table 
 *  一个网络中可以有多个TS流(用PAT中的ts_id区分) 
 *  一个TS流中可以有多个频道(用PAT中的pnumber、pmt_pid区分) 
 *  一个频道中可以有多个PES流(用PMT中的mpt_stream区分) 
 */  
int ts_header(char *buf, int pid, int payload_unit_start_indicator, int adaptation_field_control, int counter);

int ts_writecrc(char *buf, uint32_t crc);

int ts_pat_header(char *buf);

int ts_pointer_field(char *buf);

/*  
 *@remark : 添加pat头  
 */  
int mk_ts_pat_packet(char *buf, int counter);

int ts_pmt_header(char *buf);

/*  
 *@remaark: 添加PMT头 
 */  
int mk_ts_pmt_packet(char *buf, int counter);

int ts_adaptation_field(char *buf, int adaptionlength, int writepcr, uint64_t timestamp);

/*  
 *@remark: ts头的封装 
 */  
int mk_ts_data_packet(char *buf, int nFramelen, int bStart, int adaptionfield, int writepcr, int counter, uint64_t timestamp);

int make_pes_header(char *pData, int stream_id, int payload_len, uint64_t pts, uint64_t dts);


//////////////////////////////////////////////////////////////////
void *es2ts_alloc(const char* filename);

/* 
 *@remark: 整体发送数据的抽象逻辑处理函数接口 
 */  
int es2ts_writeH264(void *handle, const char* framedata, int nFrameLen, int keyframe, uint64_t timestamp);

int es2ts_destroy(void *handle);

#endif